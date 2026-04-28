#pragma once
#ifndef SHADOW_CONTAINERS_H
#define SHADOW_CONTAINERS_H

#include <vector>
#include <queue>
#include <stack>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include "TraceLogger.h"

// Global TraceLogger pointer set by injected main() boilerplate.
inline TraceLogger* _tl_ptr = nullptr;

// Role for vector<vector<T>> containers.
enum class VectorRole {
    GRAPH,  // adjacency list — push_back emits createEdge
    GRID,   // 2-D DP / board — cell write emits setGridValue + DP row coloring
    FLOW,   // network-flow capacity matrix — cell write updates BOTH grid and edge weight
};

// Semantic role for ShadowVector1D — drives automatic graph-level tracing.
enum class ArrayRole {
    PLAIN,    // generic array — delta color only
    VISITED,  // visited[node] = 1/true → colorNode(node, "gray")
    PARENT,   // parent[v] = u          → colorEdge(u, v, "yellow")
    UF_PARENT,  // new edge create only
    DIST,     // dist[v] updated        → delta color (green = relaxation)
    SEGTREE,  // tree[i] = val          → updateNodeValue(i) + colorNode(i, "yellow")
};

// ── Algorithm family — set once in main() boilerplate, drives VizColors ───────
enum class VizFamily {
    UNKNOWN,
    TRAVERSAL,      // BFS, DFS, generic graph traversal
    SHORTEST_PATH,  // Dijkstra, Bellman-Ford
    MST,            // Kruskal, Prim
    SORT,           // comparison / exchange sorts
    DP,             // 1-D / 2-D DP
    SEARCH,         // binary search, two-pointer, sliding window
    LINKED_LIST_SLL, // 단순 연결리스트
    LINKED_LIST_DLL, // 이중 연결리스트
    BST,             // 이진 탐색 트리
};
inline VizFamily _viz_family = VizFamily::UNKNOWN;

struct VizColors {
    const char* node_done = "green";   // node fully processed / visited
    const char* node_active = "yellow";  // node currently being expanded
    const char* edge_tree = "green";   // tree / shortest-path / MST edge
};
inline VizColors _viz_colors;

inline void _init_viz_colors() {
    _viz_colors = VizColors{};  // reset to defaults
    switch (_viz_family) {
        case VizFamily::SHORTEST_PATH:
            _viz_colors.node_done = "blue";
            _viz_colors.edge_tree = "blue";
            break;
        case VizFamily::MST:
            _viz_colors.edge_tree = "orange";
            break;
        case VizFamily::LINKED_LIST_SLL:
        case VizFamily::LINKED_LIST_DLL:
        case VizFamily::BST:
            _viz_colors.node_done   = "green";
            _viz_colors.node_active = "yellow";
            break;
        default: break;
    }
}

// ── Utility: extract a representative int from any value type ─────────────────
template<typename T>
inline int _tl_int(const T& v) {
    if constexpr (std::is_arithmetic_v<T>) return static_cast<int>(v);
    else return 0;
}
template<typename A, typename B>
inline int _tl_int(const std::pair<A,B>& p) {
    return static_cast<int>(p.second);  // {weight, node} Dijkstra style
}
// 어떤 타입이든 int로 변환 (TraceLogger가 int만 받음)
template<typename A, typename B, typename C>
inline int _tl_int(const std::pair<A, std::pair<B, C>>& p) {
    return static_cast<int>(p.second.first);
}
template<typename A, typename B>
inline int _tl_edge_weight(const std::pair<A,B>& p) { return static_cast<int>(p.first); }
template<typename T>
inline int _tl_edge_weight(const T&) { return 0; }


// ═══════════════════════════════════════════════════════════════════════════════
// ShadowInt  —  int wrapper that calls setPointer on every assignment.
//               Used by the injector to track binary-search / two-pointer vars.
// ═══════════════════════════════════════════════════════════════════════════════
class ShadowInt {
    int _val = 0;
    std::string _name;
    std::string _target_id;

    void _emit() const {
        if (::_tl_ptr && !_name.empty() && !_target_id.empty())
            ::_tl_ptr->setPointer(_name, _target_id, _val);
    }

public:
    ShadowInt() = default;

    ShadowInt(const std::string& name, const std::string& target_id, int init = 0)
        : _val(init), _name(name), _target_id(target_id) { _emit(); }

    // Cursor disappears when the variable goes out of scope (loop/function ends)
    ~ShadowInt() {
        if (::_tl_ptr && !_name.empty() && !_target_id.empty())
            ::_tl_ptr->removePointer(_name);
    }

    friend std::istream& operator>>(std::istream& is, ShadowInt& si) {
        int v;
        is >> v;
        si = v; // setPointer trigger
        return is;
    }

    ShadowInt& operator=(int v) {
        _val = v;
        _emit();
        return *this;
    }
    ShadowInt& operator=(const ShadowInt& o) {
        _val = o._val;
        _emit();
        return *this;
    }

    ShadowInt& operator+=(int v) { return *this = _val + v; }
    ShadowInt& operator-=(int v) { return *this = _val - v; }
    ShadowInt& operator*=(int v) { return *this = _val * v; }
    ShadowInt& operator/=(int v) { return *this = _val / v; }
    ShadowInt& operator%=(int v) { return *this = _val % v; }

    ShadowInt& operator++() { return *this = _val + 1; }
    ShadowInt& operator--() { return *this = _val - 1; }
    int operator++(int) {
        int t = _val;
        *this = _val + 1;
        return t;
    }
    int operator--(int) {
        int t = _val;
        *this = _val - 1;
        return t;
    }

    operator int() const { return _val; }

    // Arithmetic (return plain int so they compose naturally)
    int operator+(int v)  const { return _val + v; }
    int operator-(int v)  const { return _val - v; }
    int operator*(int v)  const { return _val * v; }
    int operator/(int v)  const { return _val / v; }
    int operator%(int v)  const { return _val % v; }

    friend int operator+(int a, const ShadowInt& b) { return a + b._val; }
    friend int operator-(int a, const ShadowInt& b) { return a - b._val; }
    friend int operator*(int a, const ShadowInt& b) { return a * b._val; }

    bool operator< (int v)  const { return _val <  v; }
    bool operator<=(int v)  const { return _val <= v; }
    bool operator> (int v)  const { return _val >  v; }
    bool operator>=(int v)  const { return _val >= v; }
    bool operator==(int v)  const { return _val == v; }
    bool operator!=(int v)  const { return _val != v; }

    bool operator< (const ShadowInt& o) const { return _val <  o._val; }
    bool operator<=(const ShadowInt& o) const { return _val <= o._val; }
    bool operator> (const ShadowInt& o) const { return _val >  o._val; }
    bool operator>=(const ShadowInt& o) const { return _val >= o._val; }
    bool operator==(const ShadowInt& o) const { return _val == o._val; }
    bool operator!=(const ShadowInt& o) const { return _val != o._val; }

    friend int max(const ShadowInt& a, const ShadowInt& b) { return (a._val > b._val) ? a._val : b._val; }
    friend int max(const ShadowInt& a, int b) { return (a._val > b) ? a._val : b; }
    friend int max(int a, const ShadowInt& b) { return (a > b._val) ? a : b._val; }

    friend int min(const ShadowInt& a, const ShadowInt& b) { return (a._val < b._val) ? a._val : b._val; }
    friend int min(const ShadowInt& a, int b) { return (a._val < b) ? a._val : b; }
    friend int min(int a, const ShadowInt& b) { return (a < b._val) ? a : b._val; }
};

namespace std {
    inline int max(const ShadowInt& a, int b) { return (int(a) > b) ? int(a) : b; }
    inline int max(int a, const ShadowInt& b) { return (a > int(b)) ? a : int(b); }
    inline int max(const ShadowInt& a, const ShadowInt& b) { return (int(a) > int(b)) ? int(a) : int(b); }

    inline int min(const ShadowInt& a, int b) { return (int(a) < b) ? int(a) : b; }
    inline int min(int a, const ShadowInt& b) { return (a < int(b)) ? a : int(b); }
    inline int min(const ShadowInt& a, const ShadowInt& b) { return (int(a) < int(b)) ? int(a) : int(b); }
}


// ═══════════════════════════════════════════════════════════════════════════════
// ShadowVector1D<T>  —  wraps std::vector<T>, traces every mutation
// ═══════════════════════════════════════════════════════════════════════════════
template<typename T>
class ShadowVector1D {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    std::string _id;
    std::vector<T> _data;
    ArrayRole _role = ArrayRole::PLAIN;
    bool _initialized = false;

    // ── Constructors ──────────────────────────────────────────────────────────
    ShadowVector1D() = default;
    explicit ShadowVector1D(const std::string& id) : _id(id) {}
    ShadowVector1D(const std::string& id, ArrayRole role) : _id(id), _role(role) {}

    explicit ShadowVector1D(std::size_t n) : _data(n) { _trace_init(); }
    ShadowVector1D(std::size_t n, const T& val) : _data(n, val) { _trace_init(); }

    ShadowVector1D(const std::string& id, std::size_t n)
        : _id(id), _data(n) { _trace_init(); }

    ShadowVector1D(const std::string& id, std::size_t n, const T& val)
        : _id(id), _data(n, val) { _trace_init(); }

    // Role-aware constructors — injected by InjectorService for VISITED/PARENT/DIST
    ShadowVector1D(const std::string& id, std::size_t n, ArrayRole role)
        : _id(id), _data(n), _role(role) { _trace_init(); }

    ShadowVector1D(const std::string& id, std::size_t n, const T& val, ArrayRole role)
        : _id(id), _data(n, val), _role(role) { _trace_init(); }

    ShadowVector1D(const std::string& id, std::initializer_list<T> init)
        : _id(id), _data(init) { _trace_init(); }

    ShadowVector1D(std::initializer_list<T> init) : _data(init) {}

    ShadowVector1D(const std::string& id, std::vector<T> data)
        : _id(id), _data(std::move(data)) { _trace_init(); }

    // Initialize 1D Array (1D 배열 초기화)
    // When it is called is when the array is first created
    void _trace_init() {
        if (_initialized || !::_tl_ptr || _id.empty() || _data.empty()) return;
        _initialized = true;
        std::vector<int> vals;
        vals.reserve(_data.size());
        for (const auto& v : _data) vals.push_back(_tl_int(v));
        ::_tl_ptr->setArray(_id, vals);
        if (_role == ArrayRole::SEGTREE) _trace_segtree_init();
        if (_role == ArrayRole::PARENT)  _trace_parent_init();
    }

    // Build the implicit binary-tree graph for segment-tree visualization.
    // Assumes 1-indexed storage: tree[1] = root, children of i = 2i, 2i+1.
    // 세그먼트 트리 초기화
    void _trace_segtree_init() {
        if (!::_tl_ptr || _id.empty()) return;
        int n = static_cast<int>(_data.size());
        for (int i = 1; i < n; i++)
            ::_tl_ptr->createNode(i, std::to_string(_tl_int(_data[i])));
        for (int i = 1; 2 * i < n; i++) {
            ::_tl_ptr->createEdge(i, 2 * i,     0, false);
            if (2 * i + 1 < n)
                ::_tl_ptr->createEdge(i, 2 * i + 1, 0, false);
        }
    }
    // 부모 배열 초기화
    void _trace_parent_init() {
        if (!::_tl_ptr || _id.empty()) return;
        int n = static_cast<int>(_data.size());
        for (int i = 0; i < n; i++) {
            ::_tl_ptr->createNode(i, std::to_string(i));
            ::_tl_ptr->colorNode(i, "red");
        }
    }

    void try_init() {
        _trace_init();
    }

    // ── CellProxy — intercepts assignment to arr[i] ───────────────────────────
    // arr[i] = val 가로채기
    struct CellProxy {
        T& _ref;
        ShadowVector1D& _parent;
        int _idx;

        friend std::istream& operator>>(std::istream& is, CellProxy cp) {
            T val;
            is >> val;
            cp = val;
            return is;
        }

        CellProxy& operator=(const T& val) {
            T old = _ref;
            _ref = val;
            if (!::_tl_ptr || _parent._id.empty()) return *this;

            ::_tl_ptr->updateArrayValue(_parent._id, _idx, _tl_int(val));

            switch (_parent._role) {

            case ArrayRole::VISITED:
                // visited[node] = 1/true → colorNode; =0/false → reset
                if constexpr (std::is_arithmetic_v<T>) {
                    if (_tl_int(val) != 0)
                        ::_tl_ptr->colorNode(_idx, ::_viz_colors.node_done);
                    else
                        ::_tl_ptr->colorNode(_idx, "red");
                }
                break;

            case ArrayRole::PARENT:
                // parent[v] = u → highlight tree edge u→v in yellow
                if constexpr (std::is_arithmetic_v<T>) {
                    int v = _idx;
                    int u = _tl_int(val);
                    int prev_u = _tl_int(old);
                    // Remove old tree-edge highlight
                    if (prev_u >= 0 && prev_u != u)
                        ::_tl_ptr->colorEdge(prev_u, v, "red");
                    // Highlight new tree edge
                    if (u >= 0 && u != v) {
                        ::_tl_ptr->colorEdge(u, v, ::_viz_colors.edge_tree);
                        ::_tl_ptr->colorNode(v, ::_viz_colors.node_done);
                    }

                    ::_tl_ptr->colorArray(_parent._id, {_idx}, "blue");
                }
                break;

            case ArrayRole::SEGTREE:
                // Dual-update: array panel + tree-node panel
                ::_tl_ptr->colorArray(_parent._id, {_idx}, "yellow");
                ::_tl_ptr->colorNode(_idx, "yellow");
                ::_tl_ptr->updateNodeValue(_idx, std::to_string(_tl_int(val)));
                break;

            case ArrayRole::DIST:
            case ArrayRole::PLAIN:
            default:
                // Delta coloring: decrease → green (relaxation), increase → orange
                if constexpr (std::is_arithmetic_v<T>) {
                    if (val < old) ::_tl_ptr->colorArray(_parent._id, {_idx}, "green");
                    else if (val > old) ::_tl_ptr->colorArray(_parent._id, {_idx}, "orange");
                }
                break;
            }
            return *this;
        }
        CellProxy& operator=(const CellProxy& o) { return *this = static_cast<T>(o); }

        operator T()  const { return _ref; }
        // Explicit T& conversion — only use when caller genuinely needs a reference.
        // Bypasses tracing; required for std::sort internals.
        T& get_ref() { return _ref; }

        CellProxy& operator+=(const T& v) { return *this = _ref + v; }
        CellProxy& operator-=(const T& v) { return *this = _ref - v; }
        CellProxy& operator*=(const T& v) { return *this = _ref * v; }
        CellProxy& operator/=(const T& v) { return *this = _ref / v; }
        CellProxy& operator|=(const T& v) { return *this = _ref | v; }
        CellProxy& operator&=(const T& v) { return *this = _ref & v; }
        CellProxy& operator++() { return *this += 1; }
        CellProxy& operator--() { return *this -= 1; }

        // Arithmetic pass-throughs
        T operator+(const T& v) const { return _ref + v; }
        T operator-(const T& v) const { return _ref - v; }
        T operator*(const T& v) const { return _ref * v; }
        T operator++(int) {
            T tmp = _ref;
            *this += 1;
            return tmp;
        }
        T operator--(int) {
            T tmp = _ref;
            *this -= 1;
            return tmp;
        }

        // Comparison pass-throughs
        bool operator< (const T& v) const { return _ref <  v; }
        bool operator> (const T& v) const { return _ref >  v; }
        bool operator<=(const T& v) const { return _ref <= v; }
        bool operator>=(const T& v) const { return _ref >= v; }
        bool operator==(const T& v) const { return _ref == v; }
        bool operator!=(const T& v) const { return _ref != v; }

        bool operator< (const CellProxy& o) const { return _ref <  o._ref; }
        bool operator> (const CellProxy& o) const { return _ref >  o._ref; }
        bool operator<=(const CellProxy& o) const { return _ref <= o._ref; }
        bool operator>=(const CellProxy& o) const { return _ref >= o._ref; }
        bool operator==(const CellProxy& o) const { return _ref == o._ref; }
        bool operator!=(const CellProxy& o) const { return _ref != o._ref; }

        // ADL-visible max/min so that unqualified max(arr[i], x) compiles.
        friend T max(const CellProxy& a, const CellProxy& b) { return std::max<T>(a._ref, b._ref); }
        friend T max(const CellProxy& a, const T& b) { return std::max<T>(a._ref, b); }
        friend T max(const T& a, const CellProxy& b) { return std::max<T>(a, b._ref); }
        friend T min(const CellProxy& a, const CellProxy& b) { return std::min<T>(a._ref, b._ref); }
        friend T min(const CellProxy& a, const T& b) { return std::min<T>(a._ref, b); }
        friend T min(const T& a, const CellProxy& b) { return std::min<T>(a, b._ref); }
    };

    // ── operator[] ────────────────────────────────────────────────────────────
    CellProxy operator[](std::size_t i) { return {_data[i], *this, static_cast<int>(i)}; }
    const T& operator[](std::size_t i) const { return _data[i]; }

    // ShadowInt index overloads
    CellProxy operator[](const ShadowInt& i) { return (*this)[static_cast<std::size_t>(static_cast<int>(i))]; }
    const T& operator[](const ShadowInt& i) const { return _data[static_cast<std::size_t>(static_cast<int>(i))]; }

    // ── Mutation interface ────────────────────────────────────────────────────
    std::size_t size() const { return _data.size();  }
    bool empty() const { return _data.empty(); }

    void swap(std::size_t i, std::size_t j) {
        if (i >= _data.size() || j >= _data.size() || i == j) return;
        std::swap(_data[i], _data[j]);
        if (::_tl_ptr && !_id.empty()) {
            ::_tl_ptr->resetColorArray(_id);
            ::_tl_ptr->colorArray(_id, {static_cast<int>(i), static_cast<int>(j)}, "cyan");
            ::_tl_ptr->swapArray(_id, static_cast<int>(i), static_cast<int>(j));
        }
    }

    void push_back(const T& val) {
        _data.push_back(val);
        if (::_tl_ptr && !_id.empty()) ::_tl_ptr->pushBack(_id, _tl_int(val));
    }
    void pop_back() {
        _data.pop_back();
        if (::_tl_ptr && !_id.empty()) ::_tl_ptr->popBack(_id);
    }

    void resize(std::size_t n) {
        _data.resize(n);
        _trace_init();
    }
    void resize(std::size_t n, const T& v) {
        _data.resize(n, v);
        _trace_init();
    }
    void clear() {
        _data.clear();
        _initialized = false;
    }
    void assign(std::size_t n, const T& v) {
        _data.assign(n, v);
        _trace_init();
    }

    T& front() { return _data.front(); }
    const T& front() const { return _data.front(); }
    T& back() { return _data.back();  }
    const T& back() const { return _data.back();  }

    auto begin() { return _data.begin();  }
    auto end() { return _data.end();    }
    auto begin() const { return _data.begin();  }
    auto end() const { return _data.end();    }
    auto rbegin() { return _data.rbegin(); }
    auto rend() { return _data.rend();   }
    auto rbegin() const { return _data.rbegin(); }
    auto rend() const { return _data.rend();   }

    T* data() { return _data.data(); }
    const T* data() const { return _data.data(); }

    operator std::vector<T>&() { return _data; }
    operator const std::vector<T>&() const { return _data; }
};


// ═══════════════════════════════════════════════════════════════════════════════
// ShadowVector2D<T>  —  wraps vector<vector<T>>, traces graph edges / grid cells
// ═══════════════════════════════════════════════════════════════════════════════
template<typename T>
class ShadowVector2D {
public:
    std::string _id;
    VectorRole _role;
    std::vector<std::vector<T>> _data;
    std::unordered_set<int> _nodes;
    int _num_cols = -1;         // tracked for DP row coloring
    int _last_write_row = -1;   // DP: last row written to
    std::map<std::pair<int, int>, int> _init_cap;
    bool _initialized = false;

    // ── Constructors ──────────────────────────────────────────────────────────
    ShadowVector2D() : _id(""), _role(VectorRole::GRAPH) {}

    ShadowVector2D(const std::string& id, VectorRole role)
        : _id(id), _role(role) {}

    ShadowVector2D(const std::string& id, VectorRole role, int n)
        : _id(id), _role(role), _data(static_cast<std::size_t>(n))
    { _trace_init(n, -1, T{}); }

    ShadowVector2D(const std::string& id, VectorRole role, int n, int m, T defval = T{})
        : _id(id), _role(role),
          _data(static_cast<std::size_t>(n),
                std::vector<T>(static_cast<std::size_t>(m), defval))
    { _trace_init(n, m, defval); }

    void _trace_init(int n, int m, T defval) {
        if (_initialized || !::_tl_ptr || _id.empty()) return;
        _initialized = true;
        if (_role == VectorRole::GRAPH) {
            for (int i = 1; i < n; i++) {
                ::_tl_ptr->createNode(i, std::to_string(i));
                ::_tl_ptr->colorNode(i, "red");
                _nodes.insert(i);
            }
        } else {
            // GRID and FLOW both create a grid panel
            if (m > 0) {
                _num_cols = m;
                ::_tl_ptr->createGrid(_id, n, m, _tl_int(defval));
            }
        }
    }
    // global variable
    void try_init() {
        int n = (int)_data.size();
        if (n == 0) return;
        if (_role == VectorRole::GRAPH) {
            _trace_init(n, -1, T{});
            return;
        }

        int m = (int)_data[0].size();
        if (m == 0) return;
        _trace_init(n, m, _data[0][0]);
    }

    // ── RowProxy ──────────────────────────────────────────────────────────────
    struct RowProxy {
        std::vector<T>& _row;
        ShadowVector2D& _par;
        int _ri;

        // ── CellProxy (GRID) ──────────────────────────────────────────────────
        // mat[i][j] = val 가로채기
        struct CellProxy {
            T& _ref;
            ShadowVector2D& _par;
            int _r, _c;

            CellProxy& operator=(const T& val) {
                T old = _ref;
                _ref = val;
                if (!::_tl_ptr || _par._id.empty()) return *this;

                if (_par._role == VectorRole::GRID) {
                    // ── DP row-transition coloring ────────────────────────────
                    // When we start writing a new row:
                    //   • color the just-completed row green  (= the "i-1" reference row)
                    //   • the current row gets blue per-cell  (= "i" row being filled)
                    if (_par._last_write_row != _r) {
                        if (_par._last_write_row >= 0 && _par._num_cols > 0)
                            ::_tl_ptr->rangeColorGrid(
                                _par._id,
                                _par._last_write_row, 0,
                                _par._last_write_row, _par._num_cols - 1,
                                "green");
                        _par._last_write_row = _r;
                    }
                    // Per-cell: highlight the cell being written in blue
                    if constexpr (std::is_arithmetic_v<T>) {
                        if (val != old)
                            ::_tl_ptr->colorGrid(_par._id, _r, _c, "blue");
                    }
                    ::_tl_ptr->setGridValue(_par._id, _r, _c, _tl_int(val));

                } else if (_par._role == VectorRole::FLOW) {
                    int ival = _tl_int(val); // 현재 잔여 용량
                    auto edge_key = std::make_pair(_r, _c);

                    if (_par._init_cap.find(edge_key) == _par._init_cap.end()) {
                        _par._init_cap[edge_key] = ival;
                    }
                    int cap = _par._init_cap[edge_key];

                    if (cap > 0) {
                        int flow = cap - ival;
                        ::_tl_ptr->updateEdgeText(_r, _c, std::to_string(flow) + " / " + std::to_string(cap));
                        ::_tl_ptr->updateEdgeStyle(_r, _c, "solid");
                    }
                    else {
                        if (ival > 0) {
                            ::_tl_ptr->updateEdgeText(_r, _c, "(" + std::to_string(-ival) + ")");
                        } else {
                            ::_tl_ptr->updateEdgeText(_r, _c, "");
                        }
                        ::_tl_ptr->updateEdgeStyle(_r, _c, "dotted");
                    }

                    ::_tl_ptr->setGridValue(_par._id, _r, _c, ival);

                    if (val != old) {
                        ::_tl_ptr->colorGrid(_par._id, _r, _c, "blue");
                    }
                }
                return *this;
            }
            CellProxy& operator=(const CellProxy& o) { return *this = static_cast<T>(o); }

            operator T() const { return _ref; }
            operator T&() { return _ref; }

            CellProxy& operator+=(const T& v) { return *this = _ref + v; }
            CellProxy& operator-=(const T& v) { return *this = _ref - v; }
            CellProxy& operator*=(const T& v) { return *this = _ref * v; }
            CellProxy& operator/=(const T& v) { return *this = _ref / v; }
            T operator+(const T& v) const { return _ref + v; }
            T operator-(const T& v) const { return _ref - v; }
            T operator*(const T& v) const { return _ref * v; }

            friend bool operator< (const CellProxy& a, const T& b) { return a._ref <  b; }
            friend bool operator< (const T& a, const CellProxy& b) { return a <  b._ref; }
            friend bool operator> (const CellProxy& a, const T& b) { return a._ref >  b; }
            friend bool operator> (const T& a, const CellProxy& b) { return a >  b._ref; }
            friend bool operator<=(const CellProxy& a, const T& b) { return a._ref <= b; }
            friend bool operator<=(const T& a, const CellProxy& b) { return a <= b._ref; }
            friend bool operator>=(const CellProxy& a, const T& b) { return a._ref >= b; }
            friend bool operator>=(const T& a, const CellProxy& b) { return a >= b._ref; }
            friend bool operator==(const CellProxy& a, const T& b) { return a._ref == b; }
            friend bool operator==(const T& a, const CellProxy& b) { return a == b._ref; }
            friend bool operator!=(const CellProxy& a, const T& b) { return a._ref != b; }
            friend bool operator!=(const T& a, const CellProxy& b) { return a != b._ref; }

            // CellProxy vs CellProxy
            friend bool operator<(const CellProxy& a, const CellProxy& b) { return a._ref < b._ref; }
            friend bool operator>(const CellProxy& a, const CellProxy& b) { return a._ref > b._ref; }

            // ADL max/min
            friend T max(const CellProxy& a, const T& b) { return std::max<T>(a._ref, b); }
            friend T max(const T& a, const CellProxy& b) { return std::max<T>(a, b._ref); }
            friend T max(const CellProxy& a, const CellProxy& b) { return std::max<T>(a._ref, b._ref); }
            friend T min(const CellProxy& a, const T& b) { return std::min<T>(a._ref, b); }
            friend T min(const T& a, const CellProxy& b) { return std::min<T>(a, b._ref); }
            friend T min(const CellProxy& a, const CellProxy& b) { return std::min<T>(a._ref, b._ref); }
        };

        CellProxy operator[](int col) {
            return {_row[static_cast<std::size_t>(col)], _par, _ri, col};
        }
        const T& operator[](int col) const {
            return _row[static_cast<std::size_t>(col)];
        }

        // GRAPH: intercept push_back to emit createEdge
        void push_back(const T& val) {
            _row.push_back(val);
            if (::_tl_ptr && !_par._id.empty() && _par._role == VectorRole::GRAPH) {
                int dest   = _tl_int(val);
                int weight = _tl_edge_weight(val);
                if (_par._nodes.find(dest) == _par._nodes.end()) {
                    ::_tl_ptr->createNode(dest, std::to_string(dest));
                    ::_tl_ptr->colorNode(dest, "red");
                    _par._nodes.insert(dest);
                }
                ::_tl_ptr->createEdge(_ri, dest, weight, false);
                ::_tl_ptr->colorEdge(_ri, dest, "red");
            }
        }
        void emplace_back(const T& val) { push_back(val); }

        std::size_t size() const { return _row.size();  }
        bool empty() const { return _row.empty(); }
        T& front() { return _row.front(); }
        const T& front() const { return _row.front(); }
        T& back() { return _row.back();  }
        const T& back()  const { return _row.back();  }
        auto begin() { return _row.begin(); }
        auto end() { return _row.end();  }
        auto begin() const { return _row.begin(); }
        auto end() const { return _row.end();  }
        void clear() { _row.clear(); }
        void resize(std::size_t n) { _row.resize(n); }
        void resize(std::size_t n, const T& v) { _row.resize(n, v); }
        void pop_back() { _row.pop_back(); }

        operator std::vector<T>&() { return _row; }
        operator const std::vector<T>&() const { return _row; }
    };

    // ── operator[] ─────────────────────────────────────────────────────────
    RowProxy operator[](std::size_t i) {
        if (::_tl_ptr && !_id.empty() && _role == VectorRole::GRAPH) {
            int ii = static_cast<int>(i);
            if (_nodes.find(ii) == _nodes.end()) {
                ::_tl_ptr->createNode(ii, std::to_string(ii));
                ::_tl_ptr->colorNode(ii, "red");
                _nodes.insert(ii);
            }
        }
        return {_data[i], *this, static_cast<int>(i)};
    }
    const std::vector<T>& operator[](std::size_t i) const { return _data[i]; }

    // ShadowInt index
    RowProxy operator[](const ShadowInt& i) { return (*this)[static_cast<std::size_t>(static_cast<int>(i))]; }
    const std::vector<T>& operator[](const ShadowInt& i) const { return _data[static_cast<std::size_t>(static_cast<int>(i))]; }

    // ── STL-compatible interface ────────────────────────────────────────────
    std::size_t size() const { return _data.size();  }
    bool empty() const { return _data.empty(); }

    void resize(std::size_t n) {
        std::size_t old = _data.size();
        _data.resize(n);
        if (!::_tl_ptr || _id.empty()) return;
        if (_role == VectorRole::GRAPH) {
            for (std::size_t i = old; i < n; i++) {
                int ii = static_cast<int>(i);
                if (_nodes.find(ii) == _nodes.end()) {
                    ::_tl_ptr->createNode(ii, std::to_string(ii));
                    _nodes.insert(ii);
                }
            }
        }
    }

    void push_back(const std::vector<T>& row) { _data.push_back(row); }
    void push_back(std::vector<T>&& row) { _data.push_back(std::move(row)); }
    void pop_back() { _data.pop_back(); }
    void clear() { _data.clear(); }
    void assign(std::size_t n, const std::vector<T>& row) {
        _data.assign(n, row);
        if (_role != VectorRole::GRAPH && !row.empty())
            _trace_init((int)n, (int)row.size(), row[0]);
    }

    auto begin() { return _data.begin(); }
    auto end() { return _data.end();   }
    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end();   }
};


// ═══════════════════════════════════════════════════════════════════════════════
// ShadowQueue<T>
// ═══════════════════════════════════════════════════════════════════════════════
template<typename T>
class ShadowQueue {
public:
    std::string _id;
    std::queue<T> _data;
    bool _created = false;

    ShadowQueue() = default;
    explicit ShadowQueue(const std::string& id) : _id(id) {}

    void _lazy_create() {
        if (!_created && ::_tl_ptr && !_id.empty()) {
            ::_tl_ptr->createQueue(_id);
            _created = true;
        }
    }

    void push(const T& val) {
        _lazy_create();
        _data.push(val);
        if (::_tl_ptr && !_id.empty()) {
            ::_tl_ptr->pushQueue(_id, _tl_int(val));
            ::_tl_ptr->colorNode(_tl_int(val), "yellow");
        }
    }
    void pop() {
        _lazy_create();
        T val = _data.front();
        _data.pop();
        if (::_tl_ptr && !_id.empty()) {
            ::_tl_ptr->popQueue(_id);
            ::_tl_ptr->colorNode(_tl_int(val), ::_viz_colors.node_done);
        }
    }
    T& front() { return _data.front(); }
    const T& front() const { return _data.front(); }
    T& back() { return _data.back();  }
    const T& back() const { return _data.back();  }
    bool empty() const { return _data.empty(); }
    std::size_t size() const { return _data.size();  }
};


// ═══════════════════════════════════════════════════════════════════════════════
// ShadowStack<T>
// ═══════════════════════════════════════════════════════════════════════════════
template<typename T>
class ShadowStack {
public:
    std::string _id;
    std::stack<T> _data;
    bool _created = false;

    ShadowStack() = default;
    explicit ShadowStack(const std::string& id) : _id(id) {}

    void _lazy_create() {
        if (!_created && ::_tl_ptr && !_id.empty()) {
            ::_tl_ptr->createStack(_id);
            _created = true;
        }
    }

    void push(const T& val) {
        _lazy_create();
        _data.push(val);
        if (::_tl_ptr && !_id.empty()) ::_tl_ptr->pushStack(_id, _tl_int(val));
    }
    void pop() {
        _lazy_create();
        _data.pop();
        if (::_tl_ptr && !_id.empty()) ::_tl_ptr->popStack(_id);
    }
    T& top() { return _data.top();   }
    const T& top() const { return _data.top();   }
    bool empty() const { return _data.empty(); }
    std::size_t size() const { return _data.size();  }
};


// ═══════════════════════════════════════════════════════════════════════════════
// ShadowDeque<T>
// ═══════════════════════════════════════════════════════════════════════════════
template<typename T>
class ShadowDeque {
public:
    std::string _id;
    std::deque<T> _data;
    bool _created = false;

    ShadowDeque() = default;
    explicit ShadowDeque(const std::string& id) : _id(id) {}

    void _lazy_create() {
        if (!_created && ::_tl_ptr && !_id.empty()) {
            ::_tl_ptr->createDeque(_id);
            _created = true;
        }
    }

    void push_back(const T& val) {
        _lazy_create();
        _data.push_back(val);
        if (::_tl_ptr && !_id.empty()) ::_tl_ptr->pushBackDeque(_id, _tl_int(val));
    }
    void push_front(const T& val) {
        _lazy_create();
        _data.push_front(val);
        if (::_tl_ptr && !_id.empty()) ::_tl_ptr->pushFrontDeque(_id, _tl_int(val));
    }
    void pop_back() {
        _lazy_create();
        _data.pop_back();
        if (::_tl_ptr && !_id.empty()) ::_tl_ptr->popBackDeque(_id);
    }
    void pop_front() {
        _lazy_create();
        _data.pop_front();
        if (::_tl_ptr && !_id.empty()) ::_tl_ptr->popFrontDeque(_id);
    }
    T& front() { return _data.front(); }
    const T& front() const { return _data.front(); }
    T& back() { return _data.back();  }
    const T& back()  const { return _data.back();  }
    bool empty() const { return _data.empty(); }
    std::size_t size() const { return _data.size();  }

    T& operator[](std::size_t i) { return _data[i]; }
    const T& operator[](std::size_t i) const { return _data[i]; }

    auto begin() { return _data.begin(); }
    auto end() { return _data.end();   }
    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end();   }
};


// ═══════════════════════════════════════════════════════════════════════════════
// ShadowPQ<T, Container, Compare>
// ═══════════════════════════════════════════════════════════════════════════════
template<
    typename T,
    typename Container = std::vector<T>,
    typename Compare = std::less<T>
>
class ShadowPQ {
public:
    using PQ = std::priority_queue<T, std::vector<T>, Compare>;

    std::string _id;
    PQ _data;
    bool _created = false;

    ShadowPQ() = default;
    explicit ShadowPQ(const std::string& id) : _id(id) {}

    void _lazy_create() {
        if (!_created && ::_tl_ptr && !_id.empty()) {
            ::_tl_ptr->createPQ(_id);
            _created = true;
        }
    }

    void push(const T& val) {
        _lazy_create();
        _data.push(val);
        if (::_tl_ptr && !_id.empty()) {
            ::_tl_ptr->pushPQ(_id, _tl_int(val));
            ::_tl_ptr->colorNode(_tl_int(val), "yellow");
        }
    }
    void pop() {
        _lazy_create();
        T val = _data.top();
        _data.pop();
        if (::_tl_ptr && !_id.empty()) {
            ::_tl_ptr->popPQ(_id);
            ::_tl_ptr->colorNode(_tl_int(val), ::_viz_colors.node_done);
        }
    }
    const T& top() const { return _data.top();   }
    bool empty() const { return _data.empty(); }
    std::size_t size() const { return _data.size();  }
};


// ═══════════════════════════════════════════════════════════════════════════════
// ShadowMap<K, V>  —  wraps std::map / std::unordered_map
//
// operator[] returns a ValueProxy that fires setMapValue on assignment.
// Supports int keys and std::string keys; V is converted to int via _tl_int.
// ═══════════════════════════════════════════════════════════════════════════════
template<typename K, typename V = int>
class ShadowMap {
public:
    std::string  _id;
    std::map<K, V> _data;
    bool _created = false;

    ShadowMap() = default;
    explicit ShadowMap(const std::string& id) : _id(id) {}

    void _lazy_create() {
        if (!_created && ::_tl_ptr && !_id.empty()) {
            ::_tl_ptr->createMap(_id);
            _created = true;
        }
    }

    void _trace_set(const K& key, const V& val) {
        if (!::_tl_ptr || _id.empty()) return;
        if constexpr (std::is_integral_v<K>)
            ::_tl_ptr->setMapValue(_id, static_cast<int>(key), _tl_int(val));
        else if constexpr (std::is_same_v<K, std::string>)
            ::_tl_ptr->setMapValueStr(_id, key, _tl_int(val));
    }

    void _trace_erase(const K& key) {
        if (!::_tl_ptr || _id.empty()) return;
        if constexpr (std::is_integral_v<K>)
            ::_tl_ptr->eraseMap(_id, static_cast<int>(key));
        else if constexpr (std::is_same_v<K, std::string>)
            ::_tl_ptr->eraseMapStr(_id, key);
    }

    // ── ValueProxy ────────────────────────────────────────────────────────────
    struct ValueProxy {
        ShadowMap& _par;
        const K _key;
        V& _ref;

        ValueProxy& operator=(const V& val) {
            _ref = val;
            _par._trace_set(_key, val);
            return *this;
        }
        ValueProxy& operator=(const ValueProxy& o) { return *this = static_cast<V>(o); }

        operator V() const { return _ref; }
        operator V&() { return _ref; }

        ValueProxy& operator+=(const V& v) { return *this = _ref + v; }
        ValueProxy& operator-=(const V& v) { return *this = _ref - v; }
        ValueProxy& operator++(   ) { return *this = _ref + 1; }

        bool operator< (const V& v) const { return _ref <  v; }
        bool operator> (const V& v) const { return _ref >  v; }
        bool operator==(const V& v) const { return _ref == v; }
        bool operator!=(const V& v) const { return _ref != v; }
    };

    ValueProxy operator[](const K& key) {
        _lazy_create();
        return {*this, key, _data[key]};
    }

    // count / find / contains — read-only, no tracing needed
    std::size_t count(const K& key) const { return _data.count(key); }
    bool empty() const { return _data.empty();  }
    std::size_t size() const { return _data.size();  }
    bool contains(const K& k) const { return _data.count(k) > 0; }

    auto find(const K& key) { return _data.find(key); }
    auto find(const K& key) const { return _data.find(key); }

    void erase(const K& key) {
        _lazy_create();
        if (_data.count(key)) {
            _data.erase(key);
            _trace_erase(key);
        }
    }

    auto begin() { return _data.begin(); }
    auto end() { return _data.end();   }
    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end();   }
};


// ═══════════════════════════════════════════════════════════════════════════════
// ShadowSet<T>  —  wraps std::set / std::unordered_set
// ═══════════════════════════════════════════════════════════════════════════════
template<typename T>
class ShadowSet {
public:
    std::string _id;
    std::set<T> _data;
    bool _created = false;

    ShadowSet() = default;
    explicit ShadowSet(const std::string& id) : _id(id) {}

    void _lazy_create() {
        if (!_created && ::_tl_ptr && !_id.empty()) {
            ::_tl_ptr->createSet(_id);
            _created = true;
        }
    }

    // Returns {iterator, bool} like std::set::insert
    std::pair<typename std::set<T>::iterator, bool> insert(const T& val) {
        _lazy_create();
        auto res = _data.insert(val);
        if (res.second && ::_tl_ptr && !_id.empty())
            ::_tl_ptr->insertSet(_id, _tl_int(val));
        return res;
    }

    void erase(const T& val) {
        _lazy_create();
        if (_data.count(val)) {
            _data.erase(val);
            if (::_tl_ptr && !_id.empty()) ::_tl_ptr->eraseSet(_id, _tl_int(val));
        }
    }

    std::size_t count(const T& val)  const { return _data.count(val); }
    bool empty() const { return _data.empty();    }
    std::size_t size() const { return _data.size();     }
    bool contains(const T& v) const { return _data.count(v) > 0; }

    auto find(const T& val) { return _data.find(val); }
    auto find(const T& val) const { return _data.find(val); }

    auto begin() { return _data.begin(); }
    auto end() { return _data.end();   }
    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end();   }
};

// ═══════════════════════════════════════════════════════════════════════════════
// 연결리스트 / BST  Shadow 타입
// ═══════════════════════════════════════════════════════════════════════════════

// 노드 객체 고유 ID 카운터 (main 보일러플레이트에서 0으로 리셋)
inline int _ll_node_id_counter = 0;
// 현재 연결리스트/BST 패널 ID (main 보일러플레이트에서 head/root 변수명으로 설정)
inline std::string _ll_id = "list";

// ── 포인터 필드 종류 ─────────────────────────────────────────────────────────
enum class LLFieldType {
    SLL_NEXT,
    DLL_NEXT, DLL_PREV,
    BST_LEFT, BST_RIGHT,
};

// ── 전방 선언 ─────────────────────────────────────────────────────────────────
template<typename T> struct ShadowSLLNode;
template<typename T> struct ShadowDLLNode;
template<typename T> struct ShadowBSTNode;

// ═══════════════════════════════════════════════════════════════════════════════
// ShadowPtr<NodeT>
//   — 연결리스트/BST의 포인터 필드(next/prev/left/right)를 감싸는 프록시.
//   — operator= 가로채기로 TraceLogger에 링크 변경 이벤트를 자동 발생시킨다.
//   — 노드 생성자 body에서 bind()를 호출해 owner와 필드 종류를 등록한다.
// ═══════════════════════════════════════════════════════════════════════════════
template<typename NodeT>
class ShadowPtr {
    NodeT*      _raw   = nullptr;
    NodeT*      _owner = nullptr;
    std::string _lid;          // 패널 ID
    LLFieldType _ft   = LLFieldType::SLL_NEXT;

    void _emit(int to_id) const {
        if (!::_tl_ptr || !_owner) return;
        int from = _owner->_id;
        switch (_ft) {
            case LLFieldType::SLL_NEXT:  ::_tl_ptr->sllSetNext (_lid, from, to_id); break;
            case LLFieldType::DLL_NEXT:  ::_tl_ptr->dllSetNext (_lid, from, to_id); break;
            case LLFieldType::DLL_PREV:  ::_tl_ptr->dllSetPrev (_lid, from, to_id); break;
            case LLFieldType::BST_LEFT:  ::_tl_ptr->bstSetLeft (_lid, from, to_id); break;
            case LLFieldType::BST_RIGHT: ::_tl_ptr->bstSetRight(_lid, from, to_id); break;
        }
    }

public:
    ShadowPtr() = default;

    void bind(NodeT* owner, const std::string& lid, LLFieldType ft) {
        _owner = owner; _lid = lid; _ft = ft;
    }

    // ptr->next = other;  or  ptr->next = nullptr;
    ShadowPtr& operator=(NodeT* p) {
        _raw = p;
        _emit(p ? p->_id : -1);
        return *this;
    }
    ShadowPtr& operator=(std::nullptr_t) {
        _raw = nullptr;
        _emit(-1);
        return *this;
    }

    NodeT* operator->() const { return _raw; }
    NodeT& operator*()  const { return *_raw; }
    operator NodeT*()   const { return _raw; }
    explicit operator bool() const { return _raw != nullptr; }

    bool operator==(NodeT* p)      const { return _raw == p; }
    bool operator!=(NodeT* p)      const { return _raw != p; }
    bool operator==(std::nullptr_t) const { return _raw == nullptr; }
    bool operator!=(std::nullptr_t) const { return _raw != nullptr; }

    friend bool operator==(NodeT* p, const ShadowPtr& sp) { return sp._raw == p; }
    friend bool operator!=(NodeT* p, const ShadowPtr& sp) { return sp._raw != p; }
};

// ═══════════════════════════════════════════════════════════════════════════════
// ShadowSLLNode<T>  —  단순 연결리스트 노드
//   struct Node { T val; Node* next; };  →  using Node = ShadowSLLNode<T>;
// ═══════════════════════════════════════════════════════════════════════════════
template<typename T>
struct ShadowSLLNode {
    T   val;
    int _id;
    std::string _list_id;
    ShadowPtr<ShadowSLLNode<T>> next;

    explicit ShadowSLLNode(T v)
        : val(v), _id(++::_ll_node_id_counter), _list_id(::_ll_id) {
        next.bind(this, _list_id, LLFieldType::SLL_NEXT);
        if (::_tl_ptr) ::_tl_ptr->sllCreateNode(_list_id, _id, ::_tl_int(v));
    }

    void _trace_delete() const {
        if (::_tl_ptr) ::_tl_ptr->sllDeleteNode(_list_id, _id);
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
// ShadowDLLNode<T>  —  이중 연결리스트 노드
//   struct Node { T val; Node* prev; Node* next; };  →  using Node = ShadowDLLNode<T>;
// ═══════════════════════════════════════════════════════════════════════════════
template<typename T>
struct ShadowDLLNode {
    T   val;
    int _id;
    std::string _list_id;
    ShadowPtr<ShadowDLLNode<T>> next;
    ShadowPtr<ShadowDLLNode<T>> prev;

    explicit ShadowDLLNode(T v)
        : val(v), _id(++::_ll_node_id_counter), _list_id(::_ll_id) {
        next.bind(this, _list_id, LLFieldType::DLL_NEXT);
        prev.bind(this, _list_id, LLFieldType::DLL_PREV);
        if (::_tl_ptr) ::_tl_ptr->dllCreateNode(_list_id, _id, ::_tl_int(v));
    }

    void _trace_delete() const {
        if (::_tl_ptr) ::_tl_ptr->dllDeleteNode(_list_id, _id);
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
// ShadowBSTNode<T>  —  이진 탐색 트리 노드
//   struct Node { T val; Node* left; Node* right; };  →  using Node = ShadowBSTNode<T>;
// ═══════════════════════════════════════════════════════════════════════════════
template<typename T>
struct ShadowBSTNode {
    T   val;
    int _id;
    std::string _list_id;
    ShadowPtr<ShadowBSTNode<T>> left;
    ShadowPtr<ShadowBSTNode<T>> right;

    explicit ShadowBSTNode(T v)
        : val(v), _id(++::_ll_node_id_counter), _list_id(::_ll_id) {
        left.bind (this, _list_id, LLFieldType::BST_LEFT);
        right.bind(this, _list_id, LLFieldType::BST_RIGHT);
        if (::_tl_ptr) ::_tl_ptr->bstCreateNode(_list_id, _id, ::_tl_int(v));
    }

    void _trace_delete() const {
        if (::_tl_ptr) ::_tl_ptr->bstDeleteNode(_list_id, _id);
    }
};

// ── delete 헬퍼: delete ptr → _ll_delete(ptr) ─────────────────────────────────
// 각 노드 타입의 _trace_delete()를 호출한 뒤 실제로 메모리를 해제한다.
template<typename NodeT>
inline void _ll_delete(NodeT* p) {
    if (!p) return;
    p->_trace_delete();
    delete p;
}

#endif // SHADOW_CONTAINERS_H



// arr[i] = val 실행
//     ↓
// operator[](i) → CellProxy 반환
//     ↓
// CellProxy::operator=(val) 호출
//     ↓
// _ref = val              // 실제 값 변경
// updateArrayValue(...)   // TraceLogger 호출
// switch(role) {
//     VISITED  → colorNode
//     PARENT   → colorEdge
//     SEGTREE  → updateNodeValue + colorNode
//     PLAIN    → colorArray (delta)
// }