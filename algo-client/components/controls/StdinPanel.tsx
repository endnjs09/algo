"use client";

import React, { useState } from 'react';
import { useVisualizer } from '@/lib/VisualizerContext';
import { Play, Loader2, AlertCircle } from 'lucide-react';

export function StdinPanel() {
  const {
    code, stdin, setStdin,
    isLoading, setIsLoading,
    statusMessage, setStatusMessage,
    errorMessage, setErrorMessage,
    setTraces, setMetadata, setIsPlaying, setCurrentStep, isDarkMode
  } = useVisualizer();

  const handleVisualize = async () => {
    if (code.trim() === "") return;

    if (code.split('\n').length > 500) {
      setErrorMessage("Code exceeds 500 lines limit.");
      return;
    }

    setIsLoading(true);
    setErrorMessage(null);
    setTraces([]); // Reset previous traces
    setStatusMessage("Analyzing code on server...");

    try {
      // Connect to FastAPI backend
      const API_URL = process.env.NEXT_PUBLIC_API_URL || 'http://localhost:8000';
      const response = await fetch(`${API_URL}/api/v1/visualize`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Accept': 'application/json'
        },
        body: JSON.stringify({
          language: 'cpp',
          code,
          stdin,
          client_version: '1.0.0'
        })
      });

      if (!response.ok) {
        const errorData = await response.json().catch(() => ({ detail: response.statusText }));
        throw new Error(errorData.detail || `Server error: ${response.status}`);
      }

      const data = await response.json();

      // -- Parse new format { meta: {}, frames: [] } --
      let traceArray = [];
      if (data.frames && Array.isArray(data.frames)) {
        traceArray = data.frames;
        if (data.meta) setMetadata(data.meta);
      } else if (Array.isArray(data)) {
        // Fallback for old simple array format
        traceArray = data;
        setMetadata({ family: 'UNKNOWN' });
      } else if (data.traces && Array.isArray(data.traces)) {
        // Fallback for { traces: [] } format
        traceArray = data.traces;
        setMetadata({ family: 'UNKNOWN' });
      }

      if (traceArray.length === 0) {
        setErrorMessage("No visualization events were generated. Check your code for logic.");
      } else {
        setTraces(traceArray);
        setCurrentStep(0);
        setIsPlaying(true);
        setStatusMessage("");
      }
    } catch (error: any) {
      console.error("Visualization Error:", error);
      setErrorMessage(error.message || "Failed to connect to the server.");

      // Move mock to a purely development fallback if absolutely needed
      if (process.env.NODE_ENV === 'development' && !process.env.NEXT_PUBLIC_API_URL) {
        console.log("Using mock data as development fallback...");
        // (Mock data logic would go here if we wanted to keep it)
      }
    } finally {
      setIsLoading(false);
    }
  };

  return (
    <div className="flex flex-col h-full border rounded-lg overflow-hidden shrink-0 bg-[#1e293b] border-gray-800 shadow-xl">
      <div className="flex items-center justify-between px-4 py-2 border-b border-gray-800 bg-black/20">
        <span className="text-sm font-semibold text-gray-300 tracking-tight">Standard Input</span>
      </div>

      <div className="flex-1 p-2 relative">
        <textarea
          value={stdin}
          onChange={(e) => setStdin(e.target.value)}
          placeholder="Enter input for your program here..."
          className="w-full h-full bg-transparent text-sm resize-none outline-none font-mono p-2 text-gray-300 placeholder-gray-600"
        />
      </div>

      {/* Status Bar */}
      {(statusMessage || errorMessage) && (
        <div className={`px-4 py-2 text-xs flex items-center gap-2 transition-all ${errorMessage ? 'bg-red-500/10 text-red-400' : 'bg-blue-500/10 text-blue-400'
          }`}>
          {errorMessage ? <AlertCircle size={14} /> : <Loader2 size={14} className="animate-spin" />}
          {errorMessage || statusMessage}
        </div>
      )}

      {/* Action Bar */}
      <div className="p-4 border-t border-gray-800 bg-black/20 flex justify-end">
        <button
          onClick={handleVisualize}
          disabled={isLoading || code.trim() === ""}
          className="flex items-center gap-2 px-8 py-2.5 font-bold rounded-xl transition-all active:scale-95 disabled:opacity-50 disabled:cursor-not-allowed bg-blue-600 text-white shadow-[0_0_15px_rgba(37,99,235,0.3)] hover:bg-blue-500"
        >
          {isLoading ? (
            <Loader2 size={18} className="animate-spin" />
          ) : (
            <Play size={18} fill="currentColor" />
          )}
          <span>Visualize</span>
        </button>
      </div>
    </div>
  );
}
