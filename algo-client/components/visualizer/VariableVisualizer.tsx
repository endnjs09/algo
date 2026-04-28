"use client";

import React from 'react';
import { motion } from 'framer-motion';

interface VariableVisualizerProps {
  name: string;
  value: any;
}

export function VariableVisualizer({ name, value }: VariableVisualizerProps) {
  return (
    <motion.div 
      layout
      initial={{ opacity: 0, y: 10 }}
      animate={{ opacity: 1, y: 0 }}
      className="flex items-center gap-2 bg-panel px-4 py-2 rounded-lg border border-gray-700 shadow-lg"
    >
      <span className="text-secondary font-mono font-bold">{name}</span>
      <span className="text-gray-500">=</span>
      <span className="text-white font-mono font-bold">{String(value)}</span>
    </motion.div>
  );
}
