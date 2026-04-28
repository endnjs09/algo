"use client";

import React from 'react';
import { motion, AnimatePresence } from 'framer-motion';

interface LinearVisualizerProps {
  id: string;
  type: 'stack' | 'queue' | 'deque' | 'pq';
  values: any[];
  colors: Record<number, string>;
}

export function LinearVisualizer({ id, type, values, colors }: LinearVisualizerProps) {
  // Config layout depending on type. Stack is usually vertical or right-pushing. Queue flows right to left or left to right.
  // For simplicity, we make them horizontal lists.
  
  return (
    <div className="flex flex-col gap-2 bg-panel/20 p-4 rounded-xl border border-gray-800">
      <div className="flex justify-between items-center text-gray-500 font-mono text-sm uppercase font-bold tracking-wider mb-2">
        <span>{type}: {id}</span>
        {type === 'stack' ? <span className="text-xs">Top →</span> 
        : type === 'queue' ? <span className="text-xs">← Front | Back ←</span>
        : type === 'deque' ? <span className="text-xs">↔ Front | Back ↔</span>
        : <span className="text-xs">Highest Priority →</span>}
      </div>
      
      <div className="flex gap-2 min-h-[60px] items-center p-2 rounded bg-black/20 overflow-x-auto border border-gray-800/80 shadow-inner">
        <AnimatePresence>
          {values.length === 0 && (
            <motion.div initial={{opacity:0}} animate={{opacity:1}} exit={{opacity:0}} className="text-gray-600 font-mono text-sm italic mx-auto">
              Empty
            </motion.div>
          )}
          {values.map((val, idx) => {
            const hasColor = colors[idx] !== undefined;
            const colorClass = hasColor 
                ? (colors[idx] === 'red' ? 'bg-red-500/20 border-red-500 text-red-200' 
                   : 'bg-primary/20 border-primary text-primary')
                : 'bg-panel border-gray-700 text-gray-300';
            
            return (
              <motion.div
                key={`${val}-${idx}`}
                layout
                initial={{ opacity: 0, scale: 0.5, x: 20 }}
                animate={{ opacity: 1, scale: 1, x: 0 }}
                exit={{ opacity: 0, scale: 0.5, y: -20 }}
                transition={{ type: 'spring', stiffness: 300, damping: 25 }}
                className={`flex-shrink-0 w-14 h-14 flex items-center justify-center font-mono font-bold text-lg border-2 rounded-lg transition-colors ${colorClass}`}
              >
                {val}
              </motion.div>
            );
          })}
        </AnimatePresence>
      </div>
    </div>
  );
}
