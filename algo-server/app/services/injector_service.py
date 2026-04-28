import re
import logging
from dataclasses import dataclass, field
from enum import Enum, auto
from typing import Optional

log = logging.getLogger(__name__)

# ── Regex building blocks ─────────────────────────────────────────────────────
# One level of nested angle brackets: int, pair<int,int>, tuple<int,int,int> …
_T1 = r'(?:[^<>,\s]|<[^<>]*>)+'
# Three levels of nesting (priority_queue, map, etc.)
_ANGLE3 = r'(?:[^<>]|<(?:[^<>]|<(?:[^<>]|<[^<>]*>)*>)*>)*'
_SHADOW_T = r'Shadow(?:Vector2D|Vector1D|Queue|Stack|PQ|Deque|Map|Set)<' + _ANGLE3 + r'>'

try:
    import clang.cindex as cl
    _CLANG_OK = True
except ImportError:
    _CLANG_OK = False
    log.info("libclang not installed — using regex analyzer")


# ── Domain types ──────────────────────────────────────────────────────────────

class CType(Enum):
    VEC1D = auto()  # vector<T>
    VEC2D = auto()  # vector<vector<T>>
    QUEUE = auto()  # queue<T>
    STACK = auto()  # stack<T>
    DEQUE = auto()  # deque<T>
    PQ = auto()     # priority_queue<T>
    MAP = auto()    # map<T>
    SET = auto()    # set<T>
    UMAP = auto()
    USET = auto()


class Role(Enum):
    ARRAY = "ARRAY" # array
    GRAPH = "GRAPH" # graph
    GRID = "GRID"  # 2d table(dp etc.)
    FLOW = "FLOW"  # network-flow capacity matrix: dual-update grid + graph
    QUEUE = "QUEUE"
    STACK = "STACK"
    DEQUE = "DEQUE"
    PQ = "PQ"
    MAP = "MAP"
    SET = "SET"

# setting algorithms pattern (v1.0)
class AlgoPattern(Enum):
    SORT = auto()
    BINARY_SEARCH = auto()
    TWO_POINTER = auto()
    SLIDING_WINDOW = auto()
    BFS = auto()
    DFS = auto()
    DIJKSTRA = auto()
    DP_1D = auto()
    DP_2D = auto()
    UNION_FIND = auto()
    GRAPH_GENERIC = auto()
    LINKED_LIST_SLL = auto()
    LINKED_LIST_DLL = auto()
    BST = auto()
    UNKNOWN = auto()


@dataclass
class ContainerVar:
    name: str         # "dist", "adj", "visited" ...
    ctype: CType      # VEC1D, VEC2D, QUEUE ...
    role: Role        # ARRAY, GRAPH, GRID ...
    inner: str = "int"  # inner type "int", "pair<int,int>" ....
    line_no: int = -1   # which line was declared
    # semantic role for 1-D vectors: PLAIN / VISITED / PARENT / DIST
    array_role: str = "PLAIN" # PLAIN, VISITED, PARENT, DIST, SEGTREE
    is_global: bool = False   # is global variable
    is_cstyle: bool = False   # int arr[N] / vector<T> adj[N] — not std::vector


@dataclass
class PatternInfo:
    pattern: AlgoPattern = AlgoPattern.UNKNOWN
    ptr_vars: list[str] = field(default_factory=list)
    ptr_target: str = ""    # ShadowVector1D variable name
    ptr_dir: str = ""       # "halve" | "converge" | "expand"


# ── Main service ──────────────────────────────────────────────────────────────

class InjectorService:

    _ROLE_CPP = {
        Role.GRAPH: "VectorRole::GRAPH",
        Role.GRID: "VectorRole::GRID",
        Role.FLOW: "VectorRole::FLOW",
    }

    _FAMILY_MAP: dict = {
        AlgoPattern.BFS: "TRAVERSAL",
        AlgoPattern.DFS: "TRAVERSAL",
        AlgoPattern.GRAPH_GENERIC: "TRAVERSAL",
        AlgoPattern.DIJKSTRA: "SHORTEST_PATH",
        AlgoPattern.UNION_FIND: "MST",
        AlgoPattern.SORT: "SORT",
        AlgoPattern.DP_1D: "DP",
        AlgoPattern.DP_2D: "DP",
        AlgoPattern.BINARY_SEARCH: "SEARCH",
        AlgoPattern.TWO_POINTER: "SEARCH",
        AlgoPattern.SLIDING_WINDOW: "SEARCH",
        AlgoPattern.LINKED_LIST_SLL: "LINKED_LIST_SLL",
        AlgoPattern.LINKED_LIST_DLL: "LINKED_LIST_DLL",
        AlgoPattern.BST: "BST",
        AlgoPattern.UNKNOWN: "UNKNOWN",
    }

    # ── Analysis ──────────────────────────────────────────────────────────────
    def _inject_exploration_trace(self, code: str) -> str:
        code = re.sub(
            r'for\s*\(\s*(?:const\s+)?auto\s*&?\s*\[\s*(\w+)\s*,\s*(\w+)\s*\]\s*:\s*(\w+)\s*\[\s*(\w+)\s*\]\s*\)\s*\{?',
            r'for (auto [\1, \2] : \3[\4]) { if(::_tl_ptr) { ::_tl_ptr->colorEdge(\4, \1, "yellow"); ::_tl_ptr->colorNode(\1, "yellow"); }',
            code
        )
        code = re.sub(
            r'for\s*\(\s*(?:const\s+)?(?:auto|int)\s*&?\s*(\w+)\s*:\s*(\w+)\s*\[\s*(\w+)\s*\]\s*\)\s*\{?',
            r'for (auto& \1 : \2[\3]) { if(::_tl_ptr) { int _nxt = _tl_int(\1); ::_tl_ptr->colorEdge(\3, _nxt, "yellow"); ::_tl_ptr->colorNode(_nxt, "yellow"); }',
            code
        )
        return code

    # 변수 분석
    # analysis entry point
    def analyze(self, code: str) -> list[ContainerVar]:
        if _CLANG_OK:
            try:
                return self._analyze_clang(code)
            except Exception as e:
                log.warning("libclang failed (%s); using regex fallback", e)
        return self._analyze_regex(code)

    # 정규식으로 변수 감지
    # detect variables with regular expressions
    def _analyze_regex(self, code: str) -> list[ContainerVar]:
        results: list[ContainerVar] = []
        seen: set[str] = set()
        brace_depth = 0

        for ln, line in enumerate(code.splitlines(), 1):
            is_global = (brace_depth == 0)
            # track if the code is global or within a function now
            brace_depth += line.count('{') - line.count('}')
            s = line.strip()
            if s.startswith("//") or s.startswith("*") or s.startswith("#"):
                continue

            # vector<vector<T>> name (GRID/GRAPH/FLOW)
            m = re.search(
                r'(?:std::)?\bvector\s*<\s*(?:std::)?vector\s*<\s*(' + _T1 + r')\s*>\s*>\s+(\w+)',
                line
            )
            if m and m.group(2) not in seen:
                inner, name = m.group(1).strip(), m.group(2)
                seen.add(name)
                results.append(ContainerVar(
                    name, CType.VEC2D,
                    self._classify2d(name, code),  # GRAPH? GRID? FLOW?
                    inner, ln,
                    is_global=is_global
                ))
                continue

            # vector<int> adj[10]; -> GRAPH
            m_adj = re.search(r'(?:std::)?vector\s*<\s*(' + _T1 + r')\s*>\s+(\w+)\s*\[\s*([^\]]+)\s*\]\s*;', line)
            if m_adj and m_adj.group(2) not in seen:
                inner, name = m_adj.group(1), m_adj.group(2)
                seen.add(name)
                results.append(ContainerVar(name, CType.VEC2D, Role.GRAPH, inner, ln,
                                            is_global=is_global, is_cstyle=True))
                continue

            #  vector<T> name (ARRAY)
            m = re.search(
                r'(?:std::)?\bvector\s*<\s*(' + _T1 + r')\s*>\s+(\w+)(?!\s*[,\)])',
                line
            )
            if m and m.group(2) not in seen:
                inner, name = m.group(1).strip(), m.group(2)
                seen.add(name)
                arr_role = self._classify_vec1d(name, code)
                results.append(ContainerVar(name, CType.VEC1D, Role.ARRAY, inner, ln,
                                            array_role=arr_role, is_global=is_global))
                continue

            # priority_queue<T,...> name
            m = re.search(r'(?:std::)?\bpriority_queue\s*<\s*([\w:<>,\s]+?)>\s+(\w+)', line)
            if m and m.group(2).strip() not in seen:
                name = m.group(2).strip()
                inner = m.group(1).strip().split(',')[0].strip()
                seen.add(name)
                results.append(ContainerVar(name, CType.PQ, Role.PQ, inner, ln,
                                            is_global=is_global))
                continue

            # queue<T> / stack<T> / deque<T> name
            for c_name, c_type, c_role in [
                ('queue', CType.QUEUE, Role.QUEUE),
                ('stack', CType.STACK, Role.STACK),
                ('deque', CType.DEQUE, Role.DEQUE)
            ]:
                m = re.search(rf'(?:std::)?\b{c_name}\s*<\s*([\w:<>,\s]+?)>\s+(\w+)', line)
                if m and m.group(2).strip() not in seen:
                    name = m.group(2).strip()
                    seen.add(name)
                    results.append(ContainerVar(name, c_type, c_role, "int", ln,
                                                is_global=is_global))
                    break # inner loop break
            if line.strip() != s: continue # if matched above, skip further checks for this line

            # (unordered_)map / (unordered_)set name
            m_map = re.search(r'(?:std::)?(unordered_map|map)\s*<\s*(\w[\w:]*)\s*,\s*(\w[\w:]*)\s*>\s+(\w+)', line)
            if m_map and m_map.group(4) not in seen:
                name, ctype = m_map.group(4), (CType.UMAP if m_map.group(1).startswith('unordered') else CType.MAP)
                seen.add(name)
                results.append(ContainerVar(name, ctype, Role.MAP, f"{m_map.group(2)},{m_map.group(3)}", ln,
                                            is_global=is_global))
                continue

            m_set = re.search(r'(?:std::)?(unordered_set|set)\s*<\s*(\w[\w:]*)\s*>\s+(\w+)', line)
            if m_set and m_set.group(3) not in seen:
                name, ctype = m_set.group(3), (CType.USET if m_set.group(1).startswith('unordered') else CType.SET)
                seen.add(name)
                results.append(ContainerVar(name, ctype, Role.SET, m_set.group(2), ln,
                                            is_global=is_global))
                continue

            # int mat[10][20]; -> GRID
            m2d = re.search(r'\b(\w+)\s+(\w+)\s*\[\s*([^\]]+)\s*\]\s*\[\s*([^\]]+)\s*\]\s*;', line)
            if m2d and m2d.group(2) not in seen:
                type_name, name = m2d.group(1), m2d.group(2)
                seen.add(name)
                results.append(ContainerVar(name, CType.VEC2D, Role.GRID, type_name, ln,
                                            is_global=is_global, is_cstyle=True))
                continue


            # int arr[10]; -> ARRAY
            m1d = re.search(r'\b(\w+)\s+(\w+)\s*\[\s*([^\]]+)\s*\]\s*;', line)
            if m1d and m1d.group(2) not in seen:
                type_name, name = m1d.group(1), m1d.group(2)
                seen.add(name)
                arr_role = self._classify_vec1d(name, code)
                results.append(ContainerVar(name, CType.VEC1D, Role.ARRAY, type_name, ln,
                                            array_role=arr_role, is_global=is_global, is_cstyle=True))
                continue
        return results

    # 2D 배열 역할 판단
    # Determining the 2D Array Role
    def _classify2d(self, name: str, code: str) -> Role:
        g, r, f = 0, 0, 0   # GRAPH, GRID, FLOW Score
        esc = re.escape(name)

        # ── FLOW indicators ───────────────────────────────────────────────────
        # Bidirectional update: cap[u][v] -= x AND cap[v][u] += x
        if (re.search(rf'\b{esc}\s*\[\s*\w+\s*\]\s*\[\s*\w+\s*\]\s*-=', code) and
                re.search(rf'\b{esc}\s*\[\s*\w+\s*\]\s*\[\s*\w+\s*\]\s*\+=', code)):
            f += 8
        # Used inside a BFS/DFS-based augmenting-path search
        if re.search(rf'\b{esc}\s*\[\s*\w+\s*\]\s*\[\s*\w+\s*\]\s*[><=]', code):
            f += 2
        FLOW_HINTS = {'cap', 'capacity', 'flow', 'res', 'residual'}
        nl = name.lower()
        if nl in FLOW_HINTS or any(h in nl for h in FLOW_HINTS):
            f += 5

        # ── GRAPH indicators ──────────────────────────────────────────────────
        if re.search(
            rf'\b{esc}\s*\[\s*\w+\s*\]\s*\.\s*(?:push_back|emplace_back)\s*\(',
            code
        ):
            g += 7

        # ── GRID indicators ───────────────────────────────────────────────────
        if re.search(rf'\b{esc}\s*\[\s*\w+\s*\]\s*\[\s*\w+\s*\]\s*[+\-*/]?=', code):
            r += 7
        hits = len(re.findall(rf'\b{esc}\s*\[\s*\w+\s*\]\s*\[\s*\w+\s*\]', code))
        if hits > 2:
            r += 3

        GRAPH_HINTS = {'adj', 'graph', 'edge', 'edges', 'g', 'tree',
                       'children', 'neighbor', 'next'}
        GRID_HINTS  = {'dp', 'grid', 'mat', 'matrix', 'dist', 'cost',
                       'memo', 'table', 'board', 'vis', 'visited', 'check', 'cache'}
        if nl in GRAPH_HINTS or any(h in nl for h in GRAPH_HINTS):
            g += 5
        if nl in GRID_HINTS or any(h in nl for h in GRID_HINTS):
            r += 5

        # FLOW wins only when it has a clear signal
        if f >= 8 and f >= g and f >= r:
            return Role.FLOW
        return Role.GRAPH if g >= r else Role.GRID

    # 1D 배열 역할 판단
    # Determining the 1D Array Role
    def _classify_vec1d(self, name: str, code: str) -> str:
        esc = re.escape(name)
        scores: dict[str, int] = {'PLAIN': 0, 'VISITED': 0, 'PARENT': 0,
                                   'DIST': 0, 'SEGTREE': 0}

        # ── VISITED ───────────────────────────────────────────────────────────
        # Assigned literal 1 / true / false / 0
        if re.search(rf'\b{esc}\s*\[\s*\w+\s*\]\s*=\s*(?:true|1|false|0)\s*;', code):
            scores['VISITED'] += 4
        # Used in negation / zero-check condition
        if re.search(rf'!\s*{esc}\s*\[|{esc}\s*\[\s*\w+\s*\]\s*==\s*0', code):
            scores['VISITED'] += 3
        # Initialized to all-false/0
        if re.search(rf'{esc}\s*\(\s*\w+\s*,\s*(?:false|0)\s*\)', code):
            scores['VISITED'] += 2

        # ── PARENT ────────────────────────────────────────────────────────────
        # parent[v] = u  (RHS is a node-range variable, not a literal)
        if re.search(rf'\b{esc}\s*\[\s*\w+\s*\]\s*=\s*(?!-1\b|true\b|false\b)\w+\s*;', code):
            scores['PARENT'] += 3
        # Used in path-reconstruction: v = name[v] back-tracking loop
        if re.search(rf'\w+\s*=\s*{esc}\s*\[\s*\w+\s*\]', code):
            scores['PARENT'] += 3
        # Initialized to -1 (sentinel for "no parent")
        if re.search(rf'{esc}\s*\(\s*\w+\s*,\s*-1\s*\)', code):
            scores['PARENT'] += 3

        # ── DIST ──────────────────────────────────────────────────────────────
        # Dijkstra relaxation: name[v] > name[u] + w pattern
        if re.search(rf'\b{esc}\s*\[\s*\w+\s*\]\s*>\s*{esc}\s*\[\s*\w+\s*\]\s*\+', code):
            scores['DIST'] += 7
        # Initialized to INF / large value
        if re.search(rf'{esc}\s*\(\s*\w+\s*,\s*(?:INT_MAX|1e9|0x3f3f3f3f|\d{{7,}})\s*\)', code):
            scores['DIST'] += 4

        # ── SEGTREE ───────────────────────────────────────────────────────────
        # tree[i] children accessed with 2*i / 2*i+1 (1-indexed segment tree) pattern
        if re.search(
            rf'\b{esc}\s*\[\s*2\s*\*\s*\w+\s*\]'
            rf'|\b{esc}\s*\[\s*\w+\s*\*\s*2\s*\]',
            code
        ):
            scores['SEGTREE'] += 5
        if re.search(rf'\b{esc}\s*\[\s*2\s*\*\s*\w+\s*\+\s*1\s*\]', code):
            scores['SEGTREE'] += 5
        # Parent access with i>>1 or i/2
        if re.search(
            rf'\b{esc}\s*\[\s*\w+\s*/\s*2\s*\]'
            rf'|\b{esc}\s*\[\s*\w+\s*>>\s*1\s*\]',
            code
        ):
            scores['SEGTREE'] += 3
        # Common segment-tree size patterns: 4*n, 2*(n+1)
        if re.search(rf'\b{esc}\s*\(\s*["\w]+\s*,\s*4\s*\*', code):
            scores['SEGTREE'] += 3

        # ── Name hints (tiebreaker, lower weight) ────────────────────────────
        nl = name.lower()
        if any(h == nl or nl.startswith(h) for h in
               {'visited', 'vis', 'check', 'seen', 'used', 'inq', 'inqueue'}):
            scores['VISITED'] += 2
        if any(h == nl or nl.startswith(h) for h in
               {'parent', 'prev', 'pred', 'par', 'fa', 'from', 'pre'}):
            scores['PARENT'] += 3
        if any(h == nl or nl.startswith(h) for h in
               {'dist', 'distance', 'cost', 'weight', 'd'}):
            scores['DIST'] += 2
        if any(h in nl for h in {'tree', 'seg', 'node', 'nodes', 'segtree'}):
            scores['SEGTREE'] += 3

        best = max(scores, key=lambda k: scores[k])
        return best if scores[best] > 0 else 'PLAIN'

    # ── libclang path ─────────────────────────────────────────────────────────
    # libclang으로 변수 감지
    def _analyze_clang(self, code: str) -> list[ContainerVar]:
        import tempfile, os
        with tempfile.NamedTemporaryFile(suffix='.cpp', mode='w',
                                         delete=False, encoding='utf-8') as f:
            f.write(code)
            tmp = f.name
        try:
            idx = cl.Index.create()
            tu  = idx.parse(tmp, args=['-std=c++17', '-x', 'c++'])
            results: list[ContainerVar] = []
            seen: set[str] = set()
            self._clang_walk(tu.cursor, code, results, seen)
            return results
        finally:
            os.unlink(tmp)

    def _clang_walk(self, cursor, code, results, seen):
        if cursor.kind == cl.CursorKind.VAR_DECL:
            ts, name = cursor.type.spelling, cursor.spelling
            if name and name not in seen:
                cv = self._clang_classify(name, ts, code, cursor.location.line)
                if cv:
                    seen.add(name)
                    results.append(cv)
        for child in cursor.get_children():
            self._clang_walk(child, code, results, seen)

    def _clang_classify(self, name, ts, code, line) -> Optional[ContainerVar]:
        if 'vector<vector<' in ts:
            m = re.search(r'vector<vector<(\w+)>>', ts)
            inner = m.group(1) if m else 'int'
            return ContainerVar(name, CType.VEC2D,
                                self._classify2d(name, code), inner, line)
        if re.search(r'\bvector<', ts):
            m = re.search(r'vector<(\w+)>', ts)
            return ContainerVar(name, CType.VEC1D, Role.ARRAY,
                                m.group(1) if m else 'int', line)
        if 'priority_queue<' in ts:
            return ContainerVar(name, CType.PQ, Role.PQ, 'int', line)
        if 'queue<' in ts:
            return ContainerVar(name, CType.QUEUE, Role.QUEUE, 'int', line)
        if 'stack<' in ts:
            return ContainerVar(name, CType.STACK, Role.STACK, 'int', line)
        if 'deque<' in ts:
            return ContainerVar(name, CType.DEQUE, Role.DEQUE, 'int', line)
        if 'unordered_map<' in ts or 'map<' in ts:
            ctype = CType.UMAP if 'unordered' in ts else CType.MAP
            return ContainerVar(name, ctype, Role.MAP, 'int,int', line)
        if 'unordered_set<' in ts or 'set<' in ts:
            ctype = CType.USET if 'unordered' in ts else CType.SET
            return ContainerVar(name, ctype, Role.SET, 'int', line)
        return None

    # ── Algorithm pattern detection ───────────────────────────────────────────
    # 알고리즘 종류 판단 (점수제: SORT / BFS / DFS / DIJKSTRA / DP / BINARY_SEARCH...)
    def detect_pattern(self, code: str) -> PatternInfo:
        """
        Structural pattern analysis — variable names are extracted from the
        detected structure, not assumed in advance.
        """
        scores: dict[AlgoPattern, int] = {p: 0 for p in AlgoPattern
                                           if p != AlgoPattern.UNKNOWN}

        # ── SORT ─────────────────────────────────────────────────────────────
        # Double nested loop
        if re.search(r'for\s*\([^{]*\)\s*(?:\{[^{}]*)?for\s*\(', code, re.DOTALL):
            scores[AlgoPattern.SORT] += 2
        # swap call (std::swap or .swap)
        if re.search(r'\bswap\s*\(|\bstd::swap\s*\(|\.swap\s*\(', code):
            scores[AlgoPattern.SORT] += 3
        # arr[j] compared to arr[j+1] or arr[j-1] (adjacent comparison)
        if re.search(
            r'\w+\s*\[\s*\w+\s*\]\s*[<>]\s*\w+\s*\[\s*\w+\s*(?:[+-]\s*1)?\s*\]',
            code
        ):
            scores[AlgoPattern.SORT] += 3

        # ── BINARY SEARCH ────────────────────────────────────────────────────
        bs_lo = bs_hi = ""
        bs_while = re.search(r'while\s*\(\s*(\w+)\s*<=\s*(\w+)\s*\)', code)
        if bs_while:
            bs_lo, bs_hi = bs_while.group(1), bs_while.group(2)
            elo, ehi = re.escape(bs_lo), re.escape(bs_hi)
            # mid = (lo + hi) / 2  OR  lo + (hi - lo) / 2
            mid_re = (rf'\w+\s*=\s*\(\s*{elo}\s*\+\s*{ehi}\s*\)\s*/\s*2'
                      rf'|\w+\s*=\s*{elo}\s*\+\s*\(\s*{ehi}\s*-\s*{elo}\s*\)\s*/\s*2')
            if re.search(mid_re, code):
                scores[AlgoPattern.BINARY_SEARCH] += 8
            if re.search(rf'{elo}\s*=\s*\w+\s*\+\s*1', code):
                scores[AlgoPattern.BINARY_SEARCH] += 3
            if re.search(rf'{ehi}\s*=\s*\w+\s*-\s*1', code):
                scores[AlgoPattern.BINARY_SEARCH] += 3

        # ── TWO POINTER ──────────────────────────────────────────────────────
        tp_lo = tp_hi = ""
        tp_while = re.search(r'while\s*\(\s*(\w+)\s*<\s*(\w+)\s*\)', code)
        if tp_while:
            tp_lo, tp_hi = tp_while.group(1), tp_while.group(2)
            elo, ehi = re.escape(tp_lo), re.escape(tp_hi)
            inc = bool(re.search(rf'\b{elo}\s*\+\+|\+\+\s*{elo}|{elo}\s*\+=', code))
            dec = bool(re.search(rf'\b{ehi}\s*--|-\-\s*{ehi}|{ehi}\s*-=', code))
            if inc:
                scores[AlgoPattern.TWO_POINTER] += 3
            if dec:
                scores[AlgoPattern.TWO_POINTER] += 3
            # Both pointers index the same array
            arr_lo = re.search(rf'(\w+)\s*\[\s*{elo}\s*\]', code)
            arr_hi = re.search(rf'(\w+)\s*\[\s*{ehi}\s*\]', code)
            if arr_lo and arr_hi and arr_lo.group(1) == arr_hi.group(1):
                scores[AlgoPattern.TWO_POINTER] += 4

        # ── SLIDING WINDOW ──────────────────────────────────────────────────
        # outer for (right expands) + inner while/if (left shrinks via left++)
        sw_for = re.search(r'for\s*\(\s*int\s+(\w+)\s*=\s*0\s*;', code)
        sw_left = sw_right = ""
        if sw_for:
            sw_right = sw_for.group(1)
            scores[AlgoPattern.SLIDING_WINDOW] += 2
            sw_left_m = re.search(r'\bint\s+(\w+)\s*=\s*0\s*;', code)
            if sw_left_m and sw_left_m.group(1) != sw_right:
                sw_left = sw_left_m.group(1)
                ell = re.escape(sw_left)
                if re.search(rf'\b{ell}\s*\+\+|\+\+\s*{ell}|{ell}\s*\+=', code):
                    scores[AlgoPattern.SLIDING_WINDOW] += 5
            if sw_left:
                arr_r = re.search(rf'(\w+)\s*\[\s*{re.escape(sw_right)}\s*\]', code)
                arr_l = re.search(rf'(\w+)\s*\[\s*{re.escape(sw_left)}\s*\]', code)
                if arr_r and arr_l and arr_r.group(1) == arr_l.group(1):
                    scores[AlgoPattern.SLIDING_WINDOW] += 3

        # ── BFS ──────────────────────────────────────────────────────────────
        if re.search(r'(?:std::)?queue\s*<|ShadowQueue\s*<', code):
            scores[AlgoPattern.BFS] += 3
        if re.search(r'while\s*\(\s*!\s*\w+\.empty\s*\(\s*\)\s*\)', code):
            scores[AlgoPattern.BFS] += 4
        # range-for over adjacency list row
        if re.search(r'for\s*\([^:]+:\s*\w+\s*\[\s*\w+\s*\]\s*\)', code):
            scores[AlgoPattern.BFS] += 3

        # ── DFS ──────────────────────────────────────────────────────────────
        if re.search(r'\bvoid\s+dfs\s*\(|\bbool\s+dfs\s*\(|\bint\s+dfs\s*\(', code):
            scores[AlgoPattern.DFS] += 6
        # Self-recursive non-main function
        for fn in re.findall(r'\b(?:void|int|bool)\s+(\w+)\s*\([^)]*\)\s*\{', code):
            if fn == 'main':
                continue
            body_after_decl = re.sub(
                rf'\b(?:void|int|bool)\s+{re.escape(fn)}\s*\([^)]*\)\s*\{{', '', code, count=1
            )
            if re.search(rf'\b{re.escape(fn)}\s*\(', body_after_decl):
                scores[AlgoPattern.DFS] += 2

        # ── DIJKSTRA ─────────────────────────────────────────────────────────
        if re.search(r'priority_queue\s*<|ShadowPQ\s*<', code):
            scores[AlgoPattern.DIJKSTRA] += 3
        # Relaxation: dist[v] > dist[u] + w
        if re.search(r'\w+\s*\[\s*\w+\s*\]\s*>\s*\w+\s*\[\s*\w+\s*\]\s*\+', code):
            scores[AlgoPattern.DIJKSTRA] += 5
        if re.search(r'\bINF\b|0x3f3f3f3f|1e9|INT_MAX', code):
            scores[AlgoPattern.DIJKSTRA] += 2

        # ── DP_2D ────────────────────────────────────────────────────────────
        # dp[i-1][j] or dp[i][j-1] read
        if re.search(r'\w+\s*\[\s*\w+\s*-\s*1\s*\]\s*\[', code):
            scores[AlgoPattern.DP_2D] += 6
        if re.search(r'\w+\s*\[\s*\w+\s*\]\s*\[\s*\w+\s*-\s*1\s*\]', code):
            scores[AlgoPattern.DP_2D] += 4
        # double nested for
        if re.search(r'for\s*\([^)]*\).*\n.*for\s*\([^)]*\)', code):
            scores[AlgoPattern.DP_2D] += 1

        # ── DP_1D ────────────────────────────────────────────────────────────
        # dp[i] = max/min(dp[i-1], ...)
        if re.search(
            r'\w+\s*\[\s*\w+\s*\]\s*=\s*(?:std::)?(?:max|min)\s*\(\s*\w+\s*\[\s*\w+',
            code
        ):
            scores[AlgoPattern.DP_1D] += 5
        if re.search(r'\w+\s*\[\s*\w+\s*\]\s*\+=\s*\w+\s*\[\s*\w+\s*-\s*1\s*\]', code):
            scores[AlgoPattern.DP_1D] += 5

        # ── UNION_FIND ───────────────────────────────────────────────────────
        if re.search(r'\bparent\s*\[|\brank\s*\[', code):
            scores[AlgoPattern.UNION_FIND] += 3
        if re.search(r'parent\s*\[\s*parent\s*\[', code):
            scores[AlgoPattern.UNION_FIND] += 5

        # ── GRAPH_GENERIC ────────────────────────────────────────────────────
        if re.search(r'\badj\s*\[|\bgraph\s*\[', code):
            scores[AlgoPattern.GRAPH_GENERIC] += 4

        # ── LINKED_LIST_SLL / DLL / BST ──────────────────────────────────────
        struct_info = self._detect_node_struct(code)
        if struct_info:
            _, ll_type, *_ = struct_info
            if ll_type == 'SLL':
                scores[AlgoPattern.LINKED_LIST_SLL] += 10
            elif ll_type == 'DLL':
                scores[AlgoPattern.LINKED_LIST_DLL] += 10
            elif ll_type == 'BST':
                scores[AlgoPattern.BST] += 10

        best = max(scores, key=lambda p: scores[p])
        if scores[best] == 0:
            return PatternInfo()

        if best == AlgoPattern.BINARY_SEARCH and bs_lo and bs_hi:
            info = self._extract_bs_info(code, bs_lo, bs_hi)
            info.pattern = AlgoPattern.BINARY_SEARCH
            return info

        if best == AlgoPattern.TWO_POINTER and tp_lo and tp_hi:
            info = self._extract_tp_info(code, tp_lo, tp_hi)
            info.pattern = AlgoPattern.TWO_POINTER
            return info

        if best == AlgoPattern.SLIDING_WINDOW and sw_left and sw_right:
            target = self._find_indexed_array(code, [sw_right, sw_left])
            return PatternInfo(
                pattern = AlgoPattern.SLIDING_WINDOW,
                ptr_vars = [sw_left, sw_right],
                ptr_target = target,
                ptr_dir = "expand",
            )

        return PatternInfo(pattern=best)

    def _extract_bs_info(self, code: str, lo_var: str, hi_var: str) -> PatternInfo:
        elo, ehi = re.escape(lo_var), re.escape(hi_var)
        mid_var = ""
        m = re.search(
            rf'(\w+)\s*=\s*(?:\(\s*{elo}\s*\+\s*{ehi}\s*\)\s*/\s*2'
            rf'|{elo}\s*\+\s*\(\s*{ehi}\s*-\s*{elo}\s*\)\s*/\s*2)',
            code
        )
        if m:
            mid_var = m.group(1)

        target = self._find_indexed_array(code, [mid_var, lo_var, hi_var])
        ptr_vars = [v for v in [lo_var, hi_var, mid_var] if v]
        return PatternInfo(ptr_vars=ptr_vars, ptr_target=target, ptr_dir="halve")

    def _extract_tp_info(self, code: str, lo_var: str, hi_var: str) -> PatternInfo:
        target = self._find_indexed_array(code, [lo_var, hi_var])
        return PatternInfo(ptr_vars=[lo_var, hi_var], ptr_target=target, ptr_dir="converge")

    # ── 연결리스트 / BST 구조체 감지 ─────────────────────────────────────────

    def _detect_node_struct(self, code: str) -> tuple | None:
        """
        struct Node { T val; Node* next; };  →  ('Node', 'SLL', 'int')
        struct Node { T val; Node* prev; Node* next; };  →  ('Node', 'DLL', 'int')
        struct Node { T val; Node* left; Node* right; };  →  ('Node', 'BST', 'int')

        중첩 중괄호(생성자 본문 등)를 brace-counting으로 처리한다.
        """
        m = re.search(r'\bstruct\s+(\w+)\s*\{', code)
        if not m:
            return None

        struct_name = m.group(1)
        brace_start = m.end() - 1  # '{'의 위치

        # 중첩 중괄호를 고려해 닫는 '}'를 찾는다
        depth, end = 0, brace_start
        for i, ch in enumerate(code[brace_start:]):
            if ch == '{':
                depth += 1
            elif ch == '}':
                depth -= 1
                if depth == 0:
                    end = brace_start + i
                    break

        # 닫는 '}'이 없거나 ';'로 끝나지 않으면 struct 정의가 아님
        after = code[end + 1:end + 5].lstrip()
        if not after.startswith(';'):
            return None

        body = code[brace_start + 1:end]

        # 자기참조 포인터 필드 찾기: "StructName* fieldname;"
        ptr_fields = set(re.findall(
            rf'\b{re.escape(struct_name)}\s*\*\s*(\w+)\s*[;=]',
            body
        ))
        if not ptr_fields:
            return None

        # 값 필드 타입 추출 (첫 번째 비포인터 기본 타입 필드)
        val_type = 'int'
        m_val = re.search(
            r'\b(int|long\s+long|long|double|float|char)\s+\w+\s*[;=]',
            body
        )
        if m_val:
            val_type = m_val.group(1).replace(' ', '_')  # "long long" → "long_long" (임시)
            if val_type == 'long_long':
                val_type = 'long long'

        # SLL / DLL / BST 분류
        if 'left' in ptr_fields and 'right' in ptr_fields:
            ll_type = 'BST'
        elif 'prev' in ptr_fields:
            ll_type = 'DLL'
        else:
            ll_type = 'SLL'

        head_var = self._find_ll_head_var(code, struct_name)
        return (struct_name, ll_type, val_type, head_var)

    def _find_ll_head_var(self, code: str, struct_name: str) -> str:
        """head/root 포인터 변수 이름을 찾아 패널 ID로 사용한다."""
        preferred = {'head', 'root', 'list', 'tree', 'dummy', 'sentinel'}
        # "Node* head = nullptr;" 또는 "Node* head;" 패턴
        matches = re.findall(
            rf'\b{re.escape(struct_name)}\s*\*\s*(\w+)\s*[;=]',
            code
        )
        for name in matches:
            if name in preferred:
                return name
        return matches[0] if matches else 'list'

    def _replace_struct_decl(self, code: str, struct_name: str,
                              ll_type: str, val_type: str) -> str:
        """struct 정의를 using alias로 교체한다 (중첩 중괄호 안전 처리)."""
        shadow_map = {
            'SLL': f'ShadowSLLNode<{val_type}>',
            'DLL': f'ShadowDLLNode<{val_type}>',
            'BST': f'ShadowBSTNode<{val_type}>',
        }
        shadow_type = shadow_map[ll_type]

        m = re.search(rf'\bstruct\s+{re.escape(struct_name)}\s*\{{', code)
        if not m:
            return code

        brace_start = m.end() - 1
        depth, end = 0, brace_start
        for i, ch in enumerate(code[brace_start:]):
            if ch == '{':
                depth += 1
            elif ch == '}':
                depth -= 1
                if depth == 0:
                    end = brace_start + i
                    break

        # ';' 위치 찾기
        semi = end + 1
        while semi < len(code) and code[semi] in ' \t\n\r':
            semi += 1
        if semi >= len(code) or code[semi] != ';':
            return code

        replacement = f'using {struct_name} = {shadow_type};'
        return code[:m.start()] + replacement + code[semi + 1:]

    def _replace_ll_delete(self, code: str) -> str:
        """delete expr; → _ll_delete(expr);  (expr은 변수 또는 ->체인)"""
        return re.sub(r'\bdelete\s+([\w][\w\->]*)\s*;', r'_ll_delete(\1);', code)

    def _find_indexed_array(self, code: str, vars_: list[str]) -> str:
        """Return the first Shadow-array variable that is indexed by any of vars_."""
        for v in vars_:
            if not v:
                continue
            m = re.search(rf'(\w+)\s*\[\s*{re.escape(v)}\s*\]', code)
            if m:
                return m.group(1)
        return ""

    # ── Rewriting pipeline ────────────────────────────────────────────────────

    def inject(self, code: str) -> str:
        containers = self.analyze(code)
        pattern_info = self.detect_pattern(code)
        struct_info = self._detect_node_struct(code)  # (name, ll_type, val_type) or None
        globals_ = [c for c in containers
                        if c.is_global and c.ctype in (CType.VEC1D, CType.VEC2D)]

        # int arr[N] → ShadowVector1D (Case 6)
        code = self._replace_cstyle_arrays(code, containers)

        # vector<T> → ShadowVector1D (Case 1, 2)
        # queue → ShadowQueue (Case 3)
        code = self._replace_types(code)

        # std::swap → .swap() (Case 4)
        code = self._replace_swaps(code)

        # Insert variable name + ArrayRole/VectorRole into generator (Cases 1-3, 6)
        code = self._inject_constructors(code, containers)

        # Insert colorEdge/colorNode in Graph Navigation for Statement
        code = self._inject_exploration_trace(code)

        # int lo, hi → ShadowInt (Case 5)
        code = self._inject_pointer_tracking(code, pattern_info)

        # struct Node {...}; → using Node = ShadowXXXNode<T>;  +  delete → _ll_delete
        if struct_info:
            struct_name, ll_type, val_type, _ = struct_info
            code = self._replace_struct_decl(code, struct_name, ll_type, val_type)
            code = self._replace_ll_delete(code)

        # main() initialize + save() (Case 7)
        code = self._inject_main_boilerplate(code, globals_, pattern_info, struct_info)

        # added #include "shadow_containers.h"
        code = self._prepend_header(code)
        return code

    # added shadow_containers.h
    def _prepend_header(self, code: str) -> str:
        inc_line = '#include "shadow_containers.h"\n'
        lines = code.splitlines(keepends=True)
        last_inc = -1
        for i, l in enumerate(lines):
            if l.strip().startswith('#include'):
                last_inc = i
        ins = last_inc + 1 if last_inc >= 0 else 0
        lines.insert(ins, inc_line)
        return ''.join(lines)

    def _replace_types(self, code: str) -> str:
        # vector<vector<T>>  must precede vector<T>
        code = re.sub(
            r'(?:std::)?\bvector\s*<\s*(?:std::)?vector\s*<\s*(' + _T1 + r')\s*>\s*>',
            lambda m: f'ShadowVector2D<{m.group(1).strip()}>',
            code
        )
        code = re.sub(
            r'(?:std::)?\bvector\s*<\s*(' + _T1 + r')\s*>',
            lambda m: f'ShadowVector1D<{m.group(1).strip()}>',
            code
        )
        # priority_queue must precede queue
        code = re.sub(
            r'(?:std::)?\bpriority_queue\s*<(' + _ANGLE3 + r')>',
            lambda m: f'ShadowPQ<{m.group(1).strip()}>',
            code
        )
        code = re.sub(
            r'(?:std::)?\bqueue\s*<(' + _ANGLE3 + r')>',
            lambda m: f'ShadowQueue<{m.group(1).strip()}>',
            code
        )
        code = re.sub(
            r'(?:std::)?\bstack\s*<\s*([\w:]+)\s*>',
            lambda m: f'ShadowStack<{m.group(1).strip()}>',
            code
        )
        code = re.sub(
            r'(?:std::)?\bdeque\s*<\s*([\w:]+)\s*>',
            lambda m: f'ShadowDeque<{m.group(1).strip()}>',
            code
        )
        # unordered_map must precede map
        code = re.sub(
            r'(?:std::)?\bunordered_map\s*<(' + _ANGLE3 + r')>',
            lambda m: f'ShadowMap<{m.group(1).strip()}>',
            code
        )
        code = re.sub(
            r'(?:std::)?\bmap\s*<(' + _ANGLE3 + r')>',
            lambda m: f'ShadowMap<{m.group(1).strip()}>',
            code
        )
        # unordered_set must precede set
        code = re.sub(
            r'(?:std::)?\bunordered_set\s*<(' + _ANGLE3 + r')>',
            lambda m: f'ShadowSet<{m.group(1).strip()}>',
            code
        )
        code = re.sub(
            r'(?:std::)?\bset\s*<\s*(' + _T1 + r')\s*>',
            lambda m: f'ShadowSet<{m.group(1).strip()}>',
            code
        )
        return code

    def _replace_swaps(self, code: str) -> str:
        # std::swap(arr[i], arr[j])  →  arr.swap(i, j)
        pattern = (r'\b(?:std::)?swap\s*\(\s*(\w+)\s*\[\s*([^\]]+)\s*\]\s*,'
                   r'\s*\1\s*\[\s*([^\]]+)\s*\]\s*\)')
        return re.sub(pattern, r'\1.swap(\2, \3)', code)

    # ── Constructor injection ─────────────────────────────────────────────────

    def _inject_constructors(self, code: str, containers: list[ContainerVar]) -> str:
        var_map = {c.name: c for c in containers}
        lines = code.splitlines()
        return '\n'.join(self._transform_line(l, var_map) for l in lines)

    def _transform_line(self, line: str, var_map: dict) -> str:
        for name, c in var_map.items():
            if c.ctype == CType.VEC2D:
                line = self._inject_vec2d(line, name, c.inner,
                                          self._ROLE_CPP.get(c.role, 'VectorRole::GRAPH'))
            elif c.ctype == CType.VEC1D:
                line = self._inject_vec1d(line, name, c.inner, c.array_role)
            elif c.ctype in (CType.QUEUE, CType.STACK, CType.DEQUE, CType.PQ,
                              CType.MAP, CType.SET, CType.UMAP, CType.USET):
                line = self._inject_simple(line, name)
        return line

    def _inject_vec2d(self, line: str, name: str, inner: str, role_cpp: str) -> str:
        if f'("{name}"' in line:   # already processed by _replace_cstyle_arrays
            return line
        stype = f'ShadowVector2D<{inner}>'
        esc = re.escape(name)
        esct = re.escape(stype)

        # ShadowVector2D<T> name(rows, ShadowVector1D<T>(inner_args...))
        m = re.search(
            rf'\b{esct}\s+{esc}\s*\(\s*([^,]+?)\s*,\s*ShadowVector1D\s*<'
            + _T1 + r'>\s*\(([^)]*)\)\s*\)',
            line
        )
        if m:
            return line.replace(
                m.group(0),
                f'{stype} {name}("{name}", {role_cpp}, {m.group(1).strip()}, {m.group(2).strip()})'
            )

        # ShadowVector2D<T> name(n, m) or name(n, m, default)
        m = re.search(
            rf'\b{esct}\s+{esc}\s*\(\s*([^,)]+)\s*,\s*([^,)]+)(?:\s*,\s*([^)]+))?\s*\)',
            line
        )
        if m:
            rows = m.group(1).strip()
            cols = m.group(2).strip()
            defval = m.group(3).strip() if m.group(3) else ""
            args = f'"{name}", {role_cpp}, {rows}, {cols}' + (f', {defval}' if defval else '')
            return line.replace(m.group(0), f'{stype} {name}({args})')

        # ShadowVector2D<T> name(n)
        m = re.search(rf'\b{esct}\s+{esc}\s*\(\s*([^)]+)\s*\)', line)
        if m:
            return line.replace(
                m.group(0),
                f'{stype} {name}("{name}", {role_cpp}, {m.group(1).strip()})'
            )

        # ShadowVector2D<T> name;
        m = re.search(rf'\b{esct}\s+{esc}\s*;', line)
        if m:
            return line.replace(m.group(0), f'{stype} {name}("{name}", {role_cpp});')

        return line

    def _inject_vec1d(self, line: str, name: str, inner: str,
                       array_role: str = "PLAIN") -> str:
        if f'("{name}"' in line:   # already processed by _replace_cstyle_arrays
            return line
        stype = f'ShadowVector1D<{inner}>'
        esc = re.escape(name)
        esct = re.escape(stype)
        role_arg = f', ArrayRole::{array_role}' if array_role != 'PLAIN' else ''

        # initializer-list:  ShadowVector1D<T> name = {…}
        # Role not injected here — init-list constructor doesn't need a role param.
        m = re.search(rf'\b{esct}\s+{esc}\s*=\s*(\{{[^}}]*\}})', line)
        if m:
            return line.replace(m.group(0), f'{stype} {name}("{name}", {m.group(1)})')

        # ShadowVector1D<T> name(n, val) or name(n)  — append role
        m = re.search(rf'\b{esct}\s+{esc}\s*\(([^)]+)\)', line)
        if m:
            args = m.group(1).strip()
            return line.replace(m.group(0),
                                f'{stype} {name}("{name}", {args}{role_arg})')

        # ShadowVector1D<T> name;
        m = re.search(rf'\b{esct}\s+{esc}\s*;', line)
        if m:
            # Only add role to default constructor when non-PLAIN
            role_suffix = f'("{name}"{role_arg})' if role_arg else f'("{name}")'
            return line.replace(m.group(0), f'{stype} {name}{role_suffix};')

        return line

    def _inject_simple(self, line: str, name: str) -> str:
        esc = re.escape(name)
        m = re.search(rf'\b({_SHADOW_T})\s+{esc}\s*;', line)
        if m:
            return line.replace(m.group(0), f'{m.group(1)} {name}("{name}");')
        return line

    # ── Pointer tracking injection ────────────────────────────────────────────

    def _inject_pointer_tracking(self, code: str, info: PatternInfo) -> str:
        if info.pattern == AlgoPattern.BINARY_SEARCH:
            return self._inject_bs_pointers(code, info)
        if info.pattern == AlgoPattern.TWO_POINTER:
            return self._inject_tp_pointers(code, info)
        if info.pattern == AlgoPattern.SLIDING_WINDOW:
            return self._inject_sw_pointers(code, info)
        return code

    def _inject_bs_pointers(self, code: str, info: PatternInfo) -> str:
        if len(info.ptr_vars) < 2 or not info.ptr_target:
            return code

        lo_var, hi_var = info.ptr_vars[0], info.ptr_vars[1]
        mid_var = info.ptr_vars[2] if len(info.ptr_vars) > 2 else ""
        tgt = info.ptr_target
        lines = code.splitlines()
        out = []
        in_scope = False

        for idx, line in enumerate(lines):
            # while문 진입 확인
            if re.search(rf'while\s*\(\s*{re.escape(lo_var)}\s*<=\s*{re.escape(hi_var)}\s*\)', line):
                in_scope = True

            if not in_scope and not line.lstrip().startswith('for'):
                # 1. 다중 선언 처리 (tp와 동일한 로직 적용)
                m_multi = re.search(
                    rf'\bint\s+{re.escape(lo_var)}\s*=\s*([^,;]+)\s*,\s*{re.escape(hi_var)}\s*=\s*([^;]+);',
                    line
                )
                if m_multi:
                    lo_init, hi_init = m_multi.group(1).strip(), m_multi.group(2).strip()
                    # 🌟 세미콜론으로 분리하여 안전하게 선언
                    line = line.replace(
                        m_multi.group(0),
                        f'ShadowInt {lo_var}("{lo_var}", "{tgt}", {lo_init}); '
                        f'ShadowInt {hi_var}("{hi_var}", "{tgt}", {hi_init});'
                    )
                    out.append(line)
                    continue

                # 2. 단일 선언 처리 (방어 로직 추가)
                for var in [lo_var, hi_var]:
                    m2 = re.search(rf'\bint\s+{re.escape(var)}\s*=\s*([^;]+?)\s*;', line)
                    if m2:
                        if ',' in m2.group(1): continue # 쉼표 있으면 다중 선언이므로 패스
                        init_expr = m2.group(1).strip()
                        line = line.replace(m2.group(0), f'ShadowInt {var}("{var}", "{tgt}", {init_expr});')

            # while 내부의 mid 변수 처리
            if in_scope and mid_var:
                m3 = re.search(rf'\bint\s+{re.escape(mid_var)}\s*=\s*([^;]+?)\s*;', line)
                if m3:
                    expr = m3.group(1).strip()
                    if re.search(rf'\b{re.escape(lo_var)}\b|\b{re.escape(hi_var)}\b', expr):
                        line = line.replace(m3.group(0), f'ShadowInt {mid_var}("{mid_var}", "{tgt}", {expr});')

            out.append(line)
        return '\n'.join(out)

    def _inject_tp_pointers(self, code: str, info: PatternInfo) -> str:
        if len(info.ptr_vars) < 2 or not info.ptr_target:
            return code

        lo_var, hi_var = info.ptr_vars[0], info.ptr_vars[1]
        tgt = info.ptr_target
        lines = code.splitlines()
        out = []

        for idx, line in enumerate(lines):
            m_multi = re.search(
                rf'\bint\s+{re.escape(lo_var)}\s*=\s*([^,;]+)\s*,\s*{re.escape(hi_var)}\s*=\s*([^;]+);',
                line
            )
            if m_multi:
                lo_init, hi_init = m_multi.group(1).strip(), m_multi.group(2).strip()
                new_line = line.replace(
                    m_multi.group(0),
                    f'ShadowInt {lo_var}("{lo_var}", "{tgt}", {lo_init}); '
                    f'ShadowInt {hi_var}("{hi_var}", "{tgt}", {hi_init});'
                )
                out.append(new_line)
                continue

            for var in [lo_var, hi_var]:
                m = re.search(rf'\bint\s+{re.escape(var)}\s*=\s*([^;]+?)\s*;', line)
                if m:
                    if ',' in m.group(1): continue
                    init_expr = m.group(1).strip()
                    line = line.replace(m.group(0), f'ShadowInt {var}("{var}", "{tgt}", {init_expr});')

            out.append(line)
        return '\n'.join(out)

    def _inject_sw_pointers(self, code: str, info: PatternInfo) -> str:
        """
        Sliding window:  int left = 0;  for (int right = 0; ...) { ... }
        Replace 'int left' with ShadowInt (standalone declaration).
        Replace 'int right' INSIDE the for-loop header with ShadowInt.
        Both emit setPointer on every increment; destructor cleans up.
        """
        if len(info.ptr_vars) < 2 or not info.ptr_target:
            return code

        left_var, right_var = info.ptr_vars[0], info.ptr_vars[1]
        tgt = info.ptr_target
        eleft = re.escape(left_var)
        eright = re.escape(right_var)

        lines = code.splitlines()
        out = []

        for line in lines:
            # Standalone: int left = 0;  (not inside a for-header)
            if not re.search(r'\bfor\s*\(', line):
                m = re.search(rf'\bint\s+{eleft}\s*=\s*0\s*;', line)
                if m:
                    line = line.replace(
                        m.group(0),
                        f'ShadowInt {left_var}("{left_var}", "{tgt}", 0);'
                    )

            # For-loop header: for (int right = 0; right < n; right++)
            m = re.search(rf'(for\s*\(\s*)int\s+{eright}\s*=\s*(\d+)', line)
            if m:
                line = line.replace(
                    m.group(0),
                    f'{m.group(1)}ShadowInt {right_var}("{right_var}", "{tgt}", {m.group(2)})'
                )

            out.append(line)

        return '\n'.join(out)

    # ── Main boilerplate injection ────────────────────────────────────────────

    # ── C-style array → Shadow type conversion ───────────────────────────────

    def _replace_cstyle_arrays(self, code: str, containers: list[ContainerVar]) -> str:
        cstyle = {c.name: c for c in containers if c.is_global and c.is_cstyle}
        if not cstyle:
            return code
        lines = code.splitlines()
        out = []
        brace_depth = 0
        for line in lines:
            is_global_line = (brace_depth == 0)
            brace_depth += line.count('{') - line.count('}')
            s = line.strip()
            if is_global_line and s and not s.startswith(('//', '*', '#')):
                line = self._rewrite_cstyle_decl(line, cstyle)
            out.append(line)
        return '\n'.join(out)

    def _rewrite_cstyle_decl(self, line: str, cstyle: dict) -> str:
        # vector<T> adj[N]; → ShadowVector2D<T> adj("adj", VectorRole::GRAPH, N);
        m = re.search(r'(?:std::)?vector\s*<\s*(' + _T1 + r')\s*>\s+(\w+)\s*\[\s*([^\]]+)\s*\]\s*;', line)
        if m:
            inner, name, size = m.group(1).strip(), m.group(2), m.group(3).strip()
            if name in cstyle:
                c = cstyle[name]
                role_cpp = self._ROLE_CPP.get(c.role, 'VectorRole::GRAPH')
                return line.replace(m.group(0),
                    f'ShadowVector2D<{inner}> {name}("{name}", {role_cpp}, {size});')

        # int mat[N][M]; → ShadowVector2D<int> mat("mat", VectorRole::GRID, N, M);
        m = re.search(
            r'\b(\w+)\s+(\w+)\s*\[\s*([^\]]+)\s*\]\s*\[\s*([^\]]+)\s*\]\s*;',
            line
        )
        if m:
            type_name, name, n_sz, m_sz = (m.group(1), m.group(2),
                                            m.group(3).strip(), m.group(4).strip())
            if name in cstyle:
                c = cstyle[name]
                role_cpp = self._ROLE_CPP.get(c.role, 'VectorRole::GRID')
                return line.replace(m.group(0),
                    f'ShadowVector2D<{type_name}> {name}("{name}", {role_cpp}, {n_sz}, {m_sz});')

        # int arr[N]; → ShadowVector1D<int> arr("arr", N);
        m = re.search(r'\b(\w+)\s+(\w+)\s*\[\s*([^\]]+)\s*\]\s*;', line)
        if m:
            type_name, name, size = m.group(1), m.group(2), m.group(3).strip()
            if name in cstyle:
                c = cstyle[name]
                role_arg = (f', ArrayRole::{c.array_role}'
                            if c.array_role != 'PLAIN' else '')
                return line.replace(m.group(0),
                    f'ShadowVector1D<{type_name}> {name}("{name}", {size}{role_arg});')

        return line

    def _emit_tl_header(self, out: list, indent: str,
                         global_containers: list[ContainerVar],
                         pattern_info: 'PatternInfo | None' = None,
                         struct_info: 'tuple | None' = None) -> None:
        family = self._FAMILY_MAP.get(
            pattern_info.pattern if pattern_info else AlgoPattern.UNKNOWN,
            "UNKNOWN"
        )
        out.append(f'{indent}    TraceLogger _tl;')
        out.append(f'{indent}    ::_tl_ptr = &_tl;')
        out.append(f'{indent}    ::_viz_family = VizFamily::{family};')
        out.append(f'{indent}    ::_init_viz_colors();')
        out.append(f'{indent}    _tl.setAlgoFamily("{family}");')
        for c in global_containers:
            if c.ctype == CType.VEC1D or (c.ctype == CType.VEC2D and c.role != Role.GRAPH):
                out.append(f'{indent}    {c.name}.try_init();')
        if struct_info:
            struct_name, ll_type, _, head_var = struct_info
            create_fn = {'SLL': 'createSLL', 'DLL': 'createDLL', 'BST': 'createBST'}[ll_type]
            out.append(f'{indent}    ::_ll_node_id_counter = 0;')
            out.append(f'{indent}    ::_ll_id = "{head_var}";')
            out.append(f'{indent}    _tl.{create_fn}("{head_var}");')

    # init TraceLogger and save()
    def _inject_main_boilerplate(self, code: str,
                                  global_containers: list[ContainerVar] | None = None,
                                  pattern_info: 'PatternInfo | None' = None,
                                  struct_info: 'tuple | None' = None) -> str:
        """
        State-machine walk:
          • After main()'s opening '{': inject TraceLogger + _tl_ptr setup
          • Before EVERY 'return …;' inside main: inject _tl.save(…)
          • Before main's closing '}' if no return was seen: inject save
        """
        lines = code.splitlines()
        out: list[str] = []
        state = 'pre_main'
        depth = 0
        boilerplate_done = False
        any_return_seen = False

        for line in lines:
            stripped = line.strip()
            indent = line[: len(line) - len(line.lstrip())]

            if state == 'pre_main':
                out.append(line)
                if re.search(r'\bint\s+main\s*\(', line):
                    opens = line.count('{')
                    closes = line.count('}')
                    depth = opens - closes
                    if depth > 0:
                        state = 'main_body'
                        self._emit_tl_header(out, indent, global_containers or [], pattern_info, struct_info)
                        boilerplate_done = True
                    else:
                        state = 'main_sig'
                continue

            if state == 'main_sig':
                out.append(line)
                opens = line.count('{')
                closes = line.count('}')
                depth += opens - closes
                if opens > 0 and depth > 0:
                    state = 'main_body'
                    self._emit_tl_header(out, indent, global_containers or [], pattern_info, struct_info)
                    boilerplate_done = True
                continue

            if state == 'main_body':
                # Inject save before every return inside main
                if re.match(r'\s*return\s+', line):
                    out.append(f'{indent}_tl.save("trace.json");')
                    any_return_seen = True

                out.append(line)

                opens = line.count('{')
                closes = line.count('}')
                depth += opens - closes

                if depth <= 0:
                    if not any_return_seen:
                        out.insert(-1, f'    _tl.save("trace.json");')
                    state = 'post_main'
                continue

            out.append(line)

        return '\n'.join(out)


# ==============================================
# 1. vector<int> → ShadowVector1D<int>
# ==============================================
# vector<int> arr = {5, 3, 1, 4, 2};  →   ShadowVector1D<int> arr("arr", {5, 3, 1, 4, 2});
# vector<int> dist(n, INF);           →   ShadowVector1D<int> dist("dist", n, INF, ArrayRole::DIST);
# vector<int> visited(n, false);      →   ShadowVector1D<int> visited("visited", n, false, ArrayRole::VISITED);


# ==============================================
# 2. vector<vector<int>> → ShadowVector2D<int>
# ==============================================
# vector<vector<int>> adj(n);   →   ShadowVector2D<int> adj("adj", VectorRole::GRAPH, n);
# adj[u].push_back(v);          →   adj[u].push_back(v);  // push_back automatically calls createEdge

# <<DP TABLE>>
# vector<vector<int>> dp(n+1, vector<int>(W+1, 0));  →   ShadowVector2D<int> dp("dp", VectorRole::GRID, n+1, W+1, 0);

# <<FLOW>>
# vector<vector<int>> cap(n, vector<int>(n, 0));
# cap[u][v] -= f;
# cap[v][u] += f;
# → ShadowVector2D<int> cap("cap", VectorRole::FLOW, n, n, 0);


# ==============================================
# 3. queue / stack / priority_queue
# ==============================================
# queue<int> q;    →     ShadowQueue<int> q("q");
# q.push(0);       →     q.push(0);
# q.pop();         →     q.pop();

# priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
#       →  ShadowPQ<pair<int,int>, ...> pq("pq");
# pq.push({0, src});     →     pq.push({0, src});
# pq.pop();              →     pq.pop();


# ==============================================
# 4. std::swap → .swap()
# ==============================================
# std::swap(arr[i], arr[j]);    →    arr.swap(i, j);  (swapArray)


# ==============================================
# 5. std::swap → .swap()
# ==============================================
# int lo = 0, hi = n - 1;     →     ShadowInt lo("lo", "arr", 0);
# int mid = (lo + hi) / 2;    →     ShadowInt hi("hi", "arr", n - 1);
#                             →     ShadowInt mid("mid", "arr", (lo + hi) / 2);

# int left = 0, right = n - 1;    →     ShadowInt left("left", "arr", 0);
#                                 →     ShadowInt right("right", "arr", n - 1);


# ==============================================
# 6. CStyle Array
# ==============================================
# int dist[101];                     →      ShadowVector1D<int> dist("dist", 101, ArrayRole::DIST);
# int parent[101];                   →      ShadowVector1D<int> parent("parent", 101, ArrayRole::PARENT);
# vector<pair<int,int>> adj[101];    →      ShadowVector2D<pair<int,int>> adj("adj", VectorRole::GRAPH, 101);
# int dp[101][101];                  →      ShadowVector2D<int> dp("dp", VectorRole::GRID, 101, 101);
