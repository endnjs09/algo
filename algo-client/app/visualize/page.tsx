"use client";

import React from 'react';
import { Header } from '@/components/layout/Header';
import { CodeEditor } from '@/components/editor/CodeEditor';
import { StdinPanel } from '@/components/controls/StdinPanel';
import { PlaybackController } from '@/components/controls/PlaybackController';
import { VisualizerPanel } from '@/components/visualizer/VisualizerPanel';
import { VisualizerProvider, useVisualizer } from '@/lib/VisualizerContext';

export default function VisualizePage() {
  return <VisualizeContent />;
}

function VisualizeContent() {
  const { isDarkMode } = useVisualizer();
  const [leftWidth, setLeftWidth] = React.useState(45); // width in %
  const isResizing = React.useRef(false);

  const startResizing = React.useCallback((e: React.MouseEvent) => {
    isResizing.current = true;
    document.body.style.cursor = 'col-resize';
    document.body.style.userSelect = 'none';
  }, []);

  const stopResizing = React.useCallback(() => {
    isResizing.current = false;
    document.body.style.cursor = 'default';
    document.body.style.userSelect = 'auto';
  }, []);

  const resize = React.useCallback((e: MouseEvent) => {
    if (!isResizing.current) return;
    
    const newWidth = (e.clientX / window.innerWidth) * 100;
    if (newWidth > 20 && newWidth < 80) {
      setLeftWidth(newWidth);
    }
  }, []);

  React.useEffect(() => {
    window.addEventListener('mousemove', resize);
    window.addEventListener('mouseup', stopResizing);
    return () => {
      window.removeEventListener('mousemove', resize);
      window.removeEventListener('mouseup', stopResizing);
    };
  }, [resize, stopResizing]);

  return (
    <div className="flex flex-col h-screen w-full overflow-hidden bg-[#0f172a] text-white">
      <Header />
      
      {/* Main Workspace */}
      <div className="flex-1 flex min-h-0 px-4 py-4 gap-0 relative pt-20">
        
        {/* Left Column: Editor & Stdin */}
        <div 
          style={{ width: `${leftWidth}%` }}
          className="flex flex-col gap-4 min-w-[300px] h-full pr-2"
        >
           {/* Editor takes up 70% of the left column */}
           <div className="flex-[7] min-h-0">
              <CodeEditor />
           </div>
           
           {/* Stdin takes up 30% of the left column */}
           <div className="flex-[3] min-h-0">
              <StdinPanel />
           </div>
        </div>

        {/* Resizer Bar */}
        <div 
          onMouseDown={startResizing}
          className="w-1.5 h-full cursor-col-resize flex items-center justify-center group relative z-50 mx-1"
        >
          <div className="w-[2px] h-full bg-gray-800 group-hover:bg-blue-500 transition-colors" />
          <div className="absolute top-1/2 -translate-y-1/2 w-4 h-8 flex flex-col items-center justify-center gap-1 opacity-0 group-hover:opacity-100 transition-opacity">
            <div className="w-1 h-1 rounded-full bg-blue-400" />
            <div className="w-1 h-1 rounded-full bg-blue-400" />
            <div className="w-1 h-1 rounded-full bg-blue-400" />
          </div>
        </div>

        {/* Right Column: Visualizer Canvas */}
        <div 
          style={{ width: `${100 - leftWidth}%` }}
          className="flex flex-col min-h-0 shadow-2xl rounded-lg overflow-hidden border border-gray-800 bg-[#1e293b] relative ml-2"
        >
          <VisualizerPanel />
        </div>

      </div>

      {/* Bottom Bar: Playback Controls */}
      <PlaybackController />
    </div>
  );
}
