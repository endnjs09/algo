"use client";

import React, { createContext, useContext, useState, useEffect, useRef, ReactNode } from 'react';
import { TraceEvent } from './types';

interface VisualizerContextType {
  // Data State
  code: string;
  setCode: (code: string) => void;
  stdin: string;
  setStdin: (stdin: string) => void;
  traces: TraceEvent[];
  setTraces: (traces: TraceEvent[]) => void;
  metadata: any;
  setMetadata: (meta: any) => void;
  
  // UI State
  isPlaying: boolean;
  setIsPlaying: (val: boolean | ((prev: boolean) => boolean)) => void;
  currentStep: number;
  setCurrentStep: (step: number) => void;
  playbackSpeed: number;
  setPlaybackSpeed: (speed: number) => void;
  
  statusMessage: string;
  setStatusMessage: (msg: string) => void;
  errorMessage: string | null;
  setErrorMessage: (msg: string | null) => void;
  isLoading: boolean;
  setIsLoading: (val: boolean) => void;
  isDarkMode: boolean;
  
  // Actions
  playPause: () => void;
  stepNext: () => void;
  stepPrev: () => void;
  stop: () => void;
}

const VisualizerContext = createContext<VisualizerContextType | undefined>(undefined);

export function VisualizerProvider({ children }: { children: ReactNode }) {
  const [code, setCode] = useState<string>('#include <iostream>\nusing namespace std;\n\nint main() {\n    cout << "Hello PlayGround" << endl;\n    return 0;\n}');
  const [stdin, setStdin] = useState<string>('');
  const [traces, setTraces] = useState<TraceEvent[]>([]);
  const [metadata, setMetadata] = useState<any>({ family: 'UNKNOWN' });
  
  const [isPlaying, setIsPlaying] = useState(false);
  const [currentStep, setCurrentStep] = useState(0);
  const [playbackSpeed, setPlaybackSpeed] = useState(1);
  const [isLoading, setIsLoading] = useState(false);
  const isDarkMode = true; // Fixed to Dark mode
  
  const [statusMessage, setStatusMessage] = useState('');
  const [errorMessage, setErrorMessage] = useState<string | null>(null);

  const timerRef = useRef<NodeJS.Timeout | null>(null);

  // Playback engine
  useEffect(() => {
    if (isPlaying && currentStep < traces.length - 1) {
      const baseInterval = 1000; // 1 second per step at 1x
      const interval = baseInterval / playbackSpeed;
      
      timerRef.current = setTimeout(() => {
        setCurrentStep((prev) => prev + 1);
      }, interval);
    } else if (isPlaying && currentStep >= traces.length - 1) {
      setIsPlaying(false);
    }

    return () => {
      if (timerRef.current) clearTimeout(timerRef.current);
    };
  }, [isPlaying, currentStep, playbackSpeed, traces.length]);

  const playPause = () => setIsPlaying(prev => !prev);
  const stop = () => {
    setIsPlaying(false);
    setCurrentStep(0);
  };
  const stepNext = () => {
    if (currentStep < traces.length - 1) {
      setCurrentStep(prev => prev + 1);
    } else {
      setIsPlaying(false);
    }
  };
  const stepPrev = () => {
    if (currentStep > 0) setCurrentStep(prev => prev - 1);
  };

  return (
    <VisualizerContext.Provider value={{
      code, setCode,
      stdin, setStdin,
      traces, setTraces,
      metadata, setMetadata,
      isPlaying, setIsPlaying,
      currentStep, setCurrentStep,
      playbackSpeed, setPlaybackSpeed,
      isLoading, setIsLoading,
      isDarkMode,
      statusMessage, setStatusMessage,
      errorMessage, setErrorMessage,
      playPause, stepNext, stepPrev, stop
    }}>
      {children}
    </VisualizerContext.Provider>
  );
}

export function useVisualizer() {
  const context = useContext(VisualizerContext);
  if (context === undefined) {
    throw new Error('useVisualizer must be used within a VisualizerProvider');
  }
  return context;
}
