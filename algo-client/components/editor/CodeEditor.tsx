"use client";

import React, { useRef, useState, useCallback, useEffect } from 'react';
import Editor, { OnMount } from '@monaco-editor/react';
import { useVisualizer } from '@/lib/VisualizerContext';
import { Trash2, FileDown, Upload, FileCode } from 'lucide-react';

const SAMPLES: Record<string, string> = {
  "hello_world": "#include <iostream>\nusing namespace std;\n\nint main() {\n    cout << \"Hello AlGo!\" << endl;\n    return 0;\n}",
  "bubble_sort": "#include <iostream>\n#include <vector>\nusing namespace std;\n\nint main() {\n    vector<int> arr = {5, 3, 1, 4, 2};\n    int n = arr.size();\n    for (int i = 0; i < n - 1; i++) {\n        for (int j = 0; j < n - i - 1; j++) {\n            if (arr[j] > arr[j + 1]) {\n                swap(arr[j], arr[j + 1]);\n            }\n        }\n    }\n    return 0;\n}",
  "dijkstra": `#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

typedef pair<int, int> pii;
const int INF = 1e9;

void dijkstra(int start, int n, const vector<vector<pii>>& adj, vector<int>& dist, vector<int>& parent) {
    fill(dist.begin(), dist.end(), INF);
    fill(parent.begin(), parent.end(), -1);

    priority_queue<pii, vector<pii>, greater<pii>> pq;

    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        int d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (d > dist[u]) continue;

        for (auto& edge : adj[u]) {
            int weight = edge.first;
            int v = edge.second;

            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = u; 
                pq.push({dist[v], v});
            }
        }
    }
}

int main() {
    int n = 5;
    vector<vector<pii>> adj(n + 1);
    vector<int> dist(n + 1);
    vector<int> parent(n + 1);

    adj[1].push_back({10, 2});
    adj[1].push_back({3, 3});
    adj[2].push_back({4, 4}); 
    adj[3].push_back({2, 2}); 
    adj[3].push_back({8, 4});
    adj[3].push_back({2, 5});
    adj[4].push_back({5, 5});

    dijkstra(1, n, adj, dist, parent);

    cout << "--- Result ---" << endl;
    for (int i = 1; i <= n; i++) {
        if (dist[i] == INF) cout << i << ": INF" << endl;
        else cout << i << ": " << dist[i] << endl;
    }

    return 0;
}`
};

export function CodeEditor() {
  const { code, setCode, isDarkMode, currentStep, traces } = useVisualizer();
  const editorRef = useRef<any>(null); // Use any for avoid type dependency that crashes SSR
  const monacoRef = useRef<any>(null); // Store monaco instance
  const [decorations, setDecorations] = useState<any>(null);
  
  const lineCount = code.split('\n').length;
  const isOverLimit = lineCount > 500;

  const handleEditorMount: OnMount = (editor, monacoInstance) => {
    editorRef.current = editor;
    monacoRef.current = monacoInstance;
    setDecorations(editor.createDecorationsCollection());
  };

  // Handle Drag and Drop
  const handleDrop = useCallback((e: React.DragEvent<HTMLDivElement>) => {
    e.preventDefault();
    if (e.dataTransfer.files && e.dataTransfer.files.length > 0) {
      const file = e.dataTransfer.files[0];
      if (file.name.endsWith('.cpp') || file.name.endsWith('.txt')) {
        const reader = new FileReader();
        reader.onload = (event) => {
          if (event.target?.result) {
            setCode(event.target.result.toString());
          }
        };
        reader.readAsText(file);
      } else {
        alert('Please drop a .cpp or .txt file');
      }
    }
  }, [setCode]);

  const handleDragOver = (e: React.DragEvent<HTMLDivElement>) => {
    e.preventDefault();
  };

  const handleReset = () => {
    setCode("");
  };

  const handleSampleChange = (e: React.ChangeEvent<HTMLSelectElement>) => {
    const val = e.target.value;
    if (SAMPLES[val]) {
      setCode(SAMPLES[val]);
    }
  };

  // Highlight current line if supported in traces
  useEffect(() => {
    const editor = editorRef.current;
    const monaco = monacoRef.current;
    if (!editor || !monaco || !decorations) return;
    
    if (traces.length > 0 && traces[currentStep]) {
      const trace = traces[currentStep] as any;
      if (trace.line) {
        decorations.set([{
          range: new monaco.Range(trace.line, 1, trace.line, 1),
          options: {
            isWholeLine: true,
            className: 'bg-primary/20 border-l-4 border-primary',
          }
        }]);
      } else {
        decorations.clear();
      }
    } else {
      decorations.clear();
    }
  }, [currentStep, traces, decorations]);

  return (
    <div 
      className="flex flex-col h-full border border-gray-800 rounded-lg overflow-hidden bg-[#1e1e1e] shadow-xl"
      onDrop={handleDrop}
      onDragOver={handleDragOver}
    >
      {/* Editor Header */}
      <div className="flex items-center justify-between px-4 py-2 border-b border-gray-800 bg-black/20">
        <div className="flex items-center gap-3">
          <div className="flex items-center gap-2">
            <FileCode size={16} className="text-blue-400" />
            <span className="text-xs font-semibold text-gray-300">main.cpp</span>
          </div>
          <select 
            onChange={handleSampleChange}
            className="text-xs rounded px-2 py-1 outline-none focus:border-primary bg-[#2d2d2d] text-gray-300 border border-gray-700"
            defaultValue=""
          >
            <option value="" disabled>Load Example...</option>
            <option value="bubble_sort">Bubble Sort</option>
            <option value="dijkstra">Dijkstra</option>
            <option value="hello_world">Hello World</option>
          </select>
        </div>
        
        <div className="flex items-center gap-4">
          <span className={`text-xs ${isOverLimit ? 'text-red-400 font-bold' : 'text-gray-500'}`}>
            {lineCount} / 500 lines
          </span>
          <button 
            onClick={handleReset}
            className="text-gray-400 hover:text-red-400 transition-colors"
            title="Reset code"
          >
            <Trash2 size={16} />
          </button>
        </div>
      </div>

      {/* Monaco Editor */}
      <div className="flex-1 min-h-0 w-full">
        <Editor
          height="100%"
          language="cpp"
          theme="vs-dark"
          value={code}
          onChange={(val) => {
            if (val !== undefined) setCode(val);
          }}
          onMount={handleEditorMount}
          options={{
            minimap: { enabled: false },
            fontSize: 14,
            fontFamily: 'var(--font-fira-code)',
            fontLigatures: true,
            lineHeight: 24,
            scrollBeyondLastLine: false,
            smoothScrolling: true,
            cursorBlinking: "smooth",
            padding: { top: 16 }
          }}
        />
      </div>
      
      {/* Drag overlay hint */}
      <div className="pointer-events-none absolute inset-0 hidden border-2 border-blue-500 border-dashed bg-black/50 flex flex-col items-center justify-center backdrop-blur-sm z-10 ui-drag-active:flex">
        <Upload size={48} className="text-blue-500 mb-4" />
        <p className="text-lg font-bold text-white">Drop .cpp file here</p>
      </div>
    </div>
  );
}
