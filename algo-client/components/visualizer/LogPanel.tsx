"use client";

import React, { useEffect, useRef } from 'react';
import { Terminal } from 'lucide-react';

interface LogPanelProps {
  logs: string[];
}

export function LogPanel({ logs }: LogPanelProps) {
  const scrollRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (scrollRef.current) {
      scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
    }
  }, [logs]);

  if (logs.length === 0) return null;

  return (
    <div className="absolute bottom-4 right-4 w-80 max-h-48 bg-black/80 backdrop-blur-md border border-gray-800 rounded-lg shadow-2xl flex flex-col z-20">
      <div className="flex items-center gap-2 px-3 py-2 bg-gray-900 border-b border-gray-800 rounded-t-lg">
        <Terminal size={14} className="text-gray-400" />
        <span className="text-xs font-semibold text-gray-400">Stdout / Logs</span>
      </div>
      <div ref={scrollRef} className="flex-1 overflow-y-auto p-3 flex flex-col gap-1 font-mono text-xs">
        {logs.map((log, i) => (
          <div key={i} className="text-gray-300">
             <span className="text-gray-600 mr-2">❯</span>
             {log}
          </div>
        ))}
      </div>
    </div>
  );
}
