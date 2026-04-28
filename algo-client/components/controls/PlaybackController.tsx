"use client";

import React, { useEffect, useRef } from 'react';
import { useVisualizer } from '@/lib/VisualizerContext';
import { Play, Pause, Square, SkipForward } from 'lucide-react';

export function PlaybackController() {
  const { 
    isPlaying, traces, currentStep, setCurrentStep, playbackSpeed, setPlaybackSpeed,
    playPause, stepNext, stepPrev, stop, isDarkMode
  } = useVisualizer();

  const totalSteps = traces.length;
  const isTypingRef = useRef(false);

  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      // Don't trigger if user is typing in Monaco editor or any input
      const activeElement = document.activeElement;
      const isTyping = 
        activeElement?.tagName === 'INPUT' || 
        activeElement?.tagName === 'TEXTAREA' ||
        activeElement?.classList.contains('monaco-editor') ||
        (activeElement as HTMLElement)?.isContentEditable;

      if (isTyping) return;

      if (e.code === 'Space') {
        e.preventDefault();
        playPause();
      } else if (e.code === 'ArrowRight') {
        stepNext();
      } else if (e.code === 'ArrowLeft') {
        stepPrev();
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [playPause, stepNext, stepPrev]);

  const isDisabled = totalSteps === 0;

  return (
    <div className="px-8 py-4 border-t flex items-center justify-between backdrop-blur-md bg-[#0f172a]/80 border-gray-800 text-white shadow-[0_-4px_20px_rgba(0,0,0,0.2)]">
      {/* Left: Playback Controls */}
      <div className="flex items-center gap-6">
        <div className="flex items-center gap-2">
          <button 
            onClick={stop}
            disabled={isDisabled}
            className="p-2 rounded-lg transition-colors disabled:opacity-20 text-gray-400 hover:bg-white/10 hover:text-white"
            title="Stop"
          >
            <Square size={20} fill="currentColor" />
          </button>
          
          <button 
            onClick={playPause}
            disabled={isDisabled}
            className="p-3 rounded-full shadow-lg transition-all active:scale-90 disabled:opacity-20 bg-blue-500 text-white hover:bg-blue-400"
            title={isPlaying ? "Pause (Space)" : "Play (Space)"}
          >
            {isPlaying ? <Pause size={24} fill="currentColor" /> : <Play size={24} fill="currentColor" />}
          </button>

          <button 
            onClick={stepNext}
            disabled={isDisabled || currentStep >= totalSteps - 1}
            className="p-2 rounded-lg transition-colors disabled:opacity-20 text-gray-400 hover:bg-white/10 hover:text-white"
            title="Next (Right Arrow)"
          >
            <SkipForward size={20} fill="currentColor" />
          </button>
        </div>

        {/* Speed Selector */}
        <div className="flex items-center gap-3 ml-4">
          <span className="text-xs font-bold uppercase tracking-wider text-gray-500">Speed</span>
          <select 
            value={playbackSpeed}
            onChange={(e) => setPlaybackSpeed(Number(e.target.value))}
            className="text-sm font-bold rounded-lg px-3 py-1.5 outline-none transition-all bg-[#1e293b] border border-white/10 text-white cursor-pointer hover:bg-[#2d3a4f]"
          >
            {[0.5, 1, 2, 4, 8].map(s => (
              <option key={s} value={s} className="bg-[#1e293b] text-white">{s}x</option>
            ))}
          </select>
        </div>
      </div>

      {/* Middle: Progress Slider */}
      <div className="flex-1 max-w-2xl mx-12 flex items-center gap-4">
        <span className="text-xs font-mono w-16 text-right text-gray-500">
          Step {isDisabled ? 0 : currentStep + 1}
        </span>
        <div className="flex-1 relative flex items-center">
          <input 
            type="range"
            min={0}
            max={Math.max(0, totalSteps - 1)}
            value={currentStep}
            onChange={(e) => setCurrentStep(Number(e.target.value))}
            disabled={isDisabled}
            className="w-full h-1.5 rounded-full appearance-none cursor-pointer accent-blue-500 disabled:cursor-not-allowed bg-gray-800"
          />
        </div>
        <span className="text-xs font-mono w-16 text-gray-500">
          Total {totalSteps}
        </span>
      </div>

      {/* Right: Info/Status */}
      <div className="flex items-center gap-4">
        <div className={`px-4 py-1.5 rounded-full border text-[10px] font-black uppercase tracking-tighter transition-all ${
          isPlaying 
            ? 'bg-green-500/10 border-green-500/30 text-green-400'
            : 'bg-gray-800 border-gray-700 text-gray-500'
        }`}>
          {isPlaying ? 'Live Executing' : 'Paused'}
        </div>
      </div>
    </div>
  );
}
