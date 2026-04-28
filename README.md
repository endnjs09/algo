# Algo — C++ Algorithm Visualizer

A web-based platform that transforms standard C++ algorithm code into interactive, step-by-step visualizations without requiring any instrumentation or custom API calls from the user.

---

## Getting Started

### Prerequisites

- Python 3.11+
- Node.js 18+
- Docker Desktop

### Installation

**1. Clone**
```bash
git clone https://github.com/endnjs09/algo.git
cd algo
```

**2. Backend**
```bash
cd algo-server
python -m venv .venv
.venv\Scripts\activate        # Windows
# source .venv/bin/activate   # macOS / Linux
pip install -r requirements.txt
docker build -t algo-sandbox ./sandbox
```

**3. Frontend**
```bash
cd ../algo-client
npm install
```

### Running

Open two terminals:

**Terminal 1 — Backend**
```bash
cd algo-server
uvicorn app.main:app --reload
```

**Terminal 2 — Frontend**
```bash
cd algo-client
npm run dev
```

Visit `http://localhost:3000`

---

## How It Works

Submitted code passes through a four-stage pipeline before anything reaches the frontend:

**1. Static Analysis**
`InjectorService` scans the source using regex (with optional libclang fallback) to identify STL containers and classify their semantic roles — `GRAPH`, `GRID`, `FLOW`, `DIST`, `VISITED`, `PARENT`, and so on.

**2. Pattern Detection**
The engine classifies the overall algorithm into one of several families: `TRAVERSAL`, `SHORTEST_PATH`, `MST`, `SORT`, `DP`, `SEARCH`. This drives color themes and specialized tracing behavior at runtime.

**3. Code Rewriting**
STL containers (`vector`, `queue`, `priority_queue`, `map`, `set`) are transparently substituted with Shadow equivalents that intercept every mutation and emit trace events to `TraceLogger`. Pointer-like variables (binary search bounds, two-pointer indices) are rewritten into `ShadowInt`, which calls `setPointer` on every assignment.

**4. Execution & Rendering**
The rewritten binary is compiled with `g++` inside an isolated Docker sandbox (network disabled, memory capped at 256 MB, 10-second execution limit). The resulting `trace.json` is returned to the frontend and animated frame-by-frame on an interactive canvas.

---

## Architecture

```
Browser (Next.js + TypeScript + D3.js)
    │  HTTP POST /api/v1/visualize
    ▼
FastAPI
    ├── InjectorService   — AST analysis + code rewriting (Python)
    └── SandboxService    — Docker compile + execute
            │  trace.json
            ▼
        TraceLogger (C++)
        shadow_containers.h
```

---

## Supported Algorithms (v1.0)

| Category | Algorithms |
|---|---|
| Graph Traversal | BFS, DFS |
| Shortest Path | Dijkstra, Bellman-Ford |
| MST | Prim |
| Sorting | Bubble, Selection, Insertion, Shell, Merge, Quick |
| Dynamic Programming | 1D array, 2D table |
| Search | Binary Search, Two-Pointer, Sliding Window |
| Flow | Max-Flow (Ford-Fulkerson / Edmonds-Karp) |
| Data Structures | Stack, Queue, PQ, Deque, BST, SLL, DLL, Segment Tree... | 

etc.

---

## Visualization Features

- **Time-travel playback** — play, pause, step forward/backward through execution frames
- **Synchronized views** — graph, array bars, 2D grid, and variable panels update in lockstep
- **Algorithm-aware color themes** — color palettes are assigned per algorithm family at runtime (`green` for traversal, `blue` for shortest-path, `orange` for MST, etc.)
- **Pointer tracking** — binary search bounds and two-pointer cursors are rendered as floating arrows over the target array

---

## Heuristic Naming Guide

The engine infers semantic roles from variable names. Using standard names improves detection accuracy, but it is not required — the engine falls back to structural pattern matching when names are non-standard.

| Variable name | Inferred role |
|---|---|
| `adj`, `graph`, `g` | Adjacency list (graph edges) |
| `dist`, `d` | Distance array (shortest-path coloring) |
| `visited`, `vis` | Visited state (node coloring on update) |
| `parent`, `par`, `p` | Parent array (tree edge creation on update) |
| `dp` | DP table (grid rendering) |
| `lo`/`hi`, `left`/`right` | Pointer variables (two-pointer / binary search) |
| `cap`, `flow` | Flow capacity matrix |

etc

---

## Sandbox Constraints

Code runs inside a Docker container with the following hard limits:

- Network: disabled (`--network none`)
- Memory: 256 MB
- CPU: 1 vCPU
- Execution timeout: 10 seconds
- Max trace steps: 10,000

---

## Stack

| Layer | Technology |
|---|---|
| Frontend | Next.js, TypeScript, D3.js |
| Backend | Python, FastAPI |
| Tracing | C++, TraceLogger, Shadow Containers |
| Sandbox | Docker, g++ |

---

## Deployment

> Deployment guide coming soon.

- **Frontend** — Vercel
- **Backend** — ??? (FastAPI + Docker sandbox)

---

## Contributing

The two primary extension points are:

- `sandbox/shadow_include/shadow_containers.h` — add support for new container types or array roles
- `app/services/injector_service.py` — extend pattern detection heuristics or add new rewriting passes
