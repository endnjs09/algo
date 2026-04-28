"use client";

import React from 'react';
import { motion, AnimatePresence } from 'framer-motion';

// --- Map Visualizer ---
export function MapVisualizer({ id, entries, colors }: { id: string, entries: Record<string, any>, colors: Record<string, string> }) {
  const keys = Object.keys(entries);
  return (
    <div className="flex flex-col gap-2">
      <div className="text-sm text-gray-500 font-mono font-bold uppercase tracking-wider">Map: {id}</div>
      <div className="flex flex-wrap gap-2 p-4 bg-panel/20 rounded-xl border border-gray-800">
        <AnimatePresence>
          {keys.map((key) => {
            const hasColor = colors[key] !== undefined;
            const bg = hasColor ? (colors[key] === 'red' ? 'bg-red-500/20 border-red-500' : 'bg-primary/20 border-primary') : 'bg-black/30 border-gray-700';
            return (
              <motion.div key={key} layout initial={{ opacity: 0, scale: 0.5 }} animate={{ opacity: 1, scale: 1 }} exit={{ opacity: 0, scale: 0.5 }}
                className={`flex items-center text-sm font-mono border rounded overflow-hidden ${bg}`}>
                <span className="px-2 py-1 bg-black/40 text-gray-300 font-bold border-r border-inherit">{key}</span>
                <span className="px-2 py-1 text-white">{String(entries[key])}</span>
              </motion.div>
            )
          })}
          {keys.length === 0 && <span className="text-gray-600 italic">Empty Map</span>}
        </AnimatePresence>
      </div>
    </div>
  );
}

// --- Set Visualizer ---
export function SetVisualizer({ id, values, colors }: { id: string, values: any[], colors: Record<string, string> }) {
  return (
    <div className="flex flex-col gap-2">
      <div className="text-sm text-gray-500 font-mono font-bold uppercase tracking-wider">Set: {id}</div>
      <div className="flex flex-wrap gap-2 p-4 bg-panel/20 rounded-xl border border-gray-800">
        <AnimatePresence>
          {values.map((val) => {
            const strVal = String(val);
            const hasColor = colors[strVal] !== undefined;
            const bg = hasColor ? (colors[strVal] === 'red' ? 'bg-red-500/30 border-red-500 text-red-200' : 'bg-primary/30 border-primary text-primary') : 'bg-black/40 border-gray-700 text-gray-300';
            return (
              <motion.div key={strVal} layout initial={{ opacity: 0, scale: 0.5 }} animate={{ opacity: 1, scale: 1 }} exit={{ opacity: 0, scale: 0.5 }}
                className={`px-3 py-1.5 rounded-full border font-mono text-sm ${bg}`}>
                {strVal}
              </motion.div>
            )
          })}
          {values.length === 0 && <span className="text-gray-600 italic">Empty Set</span>}
        </AnimatePresence>
      </div>
    </div>
  );
}

// --- Grid / Matrix Visualizer ---
export function GridVisualizer({ id, rows, cols, defaultVal, values, colors }: { id: string, rows: number, cols: number, defaultVal: number, values: Record<string, any>, colors: Record<string, string> }) {
  const gridRows = Array.from({ length: rows || 0 });
  const gridCols = Array.from({ length: cols || 0 });

  return (
    <div className="flex flex-col gap-2">
      <div className="text-sm text-gray-500 font-mono font-bold uppercase tracking-wider">Grid: {id}</div>
      <div className="inline-flex flex-col gap-1 p-2 bg-panel/30 border border-gray-800 rounded-lg">
        {gridRows.map((_, r) => (
          <div key={r} className="flex gap-1">
            {gridCols.map((_, c) => {
              const key = `${r},${c}`;
              const val = values[key] ?? defaultVal ?? 0;
              const hasColor = colors[key] !== undefined;
              const bg = hasColor ? (colors[key] === 'red' ? 'bg-red-500/80 text-white' : 'bg-primary/80 text-black') : 'bg-editor text-gray-400';
              return (
                <motion.div key={key} layout className={`w-10 h-10 flex items-center justify-center font-mono text-xs rounded border border-gray-700 ${bg}`}>
                  {val}
                </motion.div>
              )
            })}
          </div>
        ))}
      </div>
    </div>
  )
}

// --- UF Visualizer ---
export function UFVisualizer({ id, parent }: { id: string, parent: Record<number, number> }) {
  const keys = Object.keys(parent).map(Number).sort((a, b) => a - b);
  return (
    <div className="flex flex-col gap-2 bg-panel/20 p-4 border border-gray-800 rounded-xl">
      <div className="text-sm text-gray-500 font-mono font-bold uppercase tracking-wider">Union-Find: {id}</div>
      <div className="flex items-center gap-4 flex-wrap">
        {keys.map(k => (
          <div key={k} className="flex flex-col items-center">
            <div className="w-8 h-8 rounded-full bg-editor text-gray-300 flex items-center justify-center text-xs border border-gray-600 mb-1">{k}</div>
            <svg width="12" height="12"><path d="M6,0 L6,12" stroke="#4b5563" /><path d="M3,9 L6,12 L9,9" fill="#4b5563" /></svg>
            <div className="w-8 h-8 rounded-full bg-primary/20 text-primary border border-primary flex items-center justify-center text-xs mt-1">{parent[k] ?? k}</div>
          </div>
        ))}
      </div>
    </div>
  )
}

// --- Pointer Visualizer ---
export function PointerVisualizer({ name, pointer }: { name: string, pointer: any }) {
  return (
    <motion.div layout initial={{ opacity: 0, y: 10 }} animate={{ opacity: 1, y: 0 }}
      className={`px-3 py-1.5 flex items-center gap-2 rounded-full border text-sm font-mono
       ${pointer.color === 'red' ? 'bg-red-500/20 border-red-500 text-red-200'
          : pointer.color === 'green' ? 'bg-green-500/20 border-green-500 text-green-200'
            : 'bg-primary/20 border-primary text-primary'}`}>
      <span className="font-bold">{name}</span>
      <span className="text-gray-500 text-xs">→</span>
      <span>{pointer.targetId}[{pointer.targetIndex}]</span>
    </motion.div>
  );
}

