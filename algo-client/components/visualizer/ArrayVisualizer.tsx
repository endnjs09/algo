"use client";

import React, { useMemo } from 'react';
import { motion, AnimatePresence } from 'framer-motion';

interface ArrayVisualizerProps {
  id: string;
  values: any[];
  colors: Record<number, string>;
}

export function ArrayVisualizer({ id, values, colors }: ArrayVisualizerProps) {
  // Determine maximum value to scale the bars appropriately. Provide a fallback if empty or zero.
  const numericValues = values.map(v => Number(v)).filter(n => !isNaN(n));
  const maxVal = numericValues.length > 0 ? Math.max(...numericValues, 10) : 10;
  const maxBarHeight = 160; // Max pixels for the tallest bar

  return (
    <div className="flex flex-col gap-4">
      <div className="text-sm text-gray-500 font-mono font-bold uppercase tracking-wider">{id}</div>
      <div className="flex items-end gap-3 h-[200px] bg-panel/30 p-4 rounded-xl border border-gray-800/50 relative">
        <AnimatePresence>
          {values.map((val, idx) => {
            const numVal = Number(val);
            const height = isNaN(numVal) ? 40 : Math.max(((numVal / maxVal) * maxBarHeight), 20); // Min height of 20px

            const hasColor = colors[idx] !== undefined;
            // Map common color strings to Tailwind colors heuristically for now.
            const colorClass = hasColor
              ? (colors[idx] === 'red' ? 'bg-red-500/80 shadow-[0_0_15px_rgba(239,68,68,0.5)]'
                : colors[idx] === 'green' ? 'bg-green-500/80 shadow-[0_0_15px_rgba(34,197,94,0.5)]'
                  : 'bg-primary/80 shadow-[0_0_15px_rgba(0,212,255,0.5)]')
              : 'bg-gray-600 shadow-md';

            return (
              <motion.div
                key={`${val}-${idx}`} // Uniqueness for framer motion swaps
                layout
                initial={{ opacity: 0, height: 0, y: 10 }}
                animate={{ opacity: 1, height, y: 0 }}
                exit={{ opacity: 0, scale: 0.5 }}
                transition={{ type: 'spring', stiffness: 200, damping: 20 }}
                className={`relative w-12 rounded-t-md flex flex-col justify-start items-center transition-colors duration-300 ${colorClass}`}
              >
                {/* Value floating above the bar */}
                <div className="absolute -top-7 text-white font-mono font-bold text-sm bg-black/50 px-2 py-0.5 rounded backdrop-blur-sm border border-gray-800">
                  {val}
                </div>

                {/* Index pill anchored to the bottom */}
                <div className="absolute -bottom-8 text-gray-400 font-mono text-xs cursor-default">
                  {idx}
                </div>
              </motion.div>
            );
          })}
        </AnimatePresence>

        {/* Baseline styling */}
        <div className="absolute bottom-[30px] left-0 right-0 h-px bg-gray-700/50 rounded-full" />
      </div>
    </div>
  );
}
