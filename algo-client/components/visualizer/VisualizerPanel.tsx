"use client";

import React, { useMemo } from 'react';
import { useVisualizer } from '@/lib/VisualizerContext';
import { computeState } from '@/lib/visualizerReducer';

import { ArrayVisualizer } from './ArrayVisualizer';
import { VariableVisualizer } from './VariableVisualizer';
import { LinearVisualizer } from './LinearVisualizer';
import { GraphVisualizer } from './GraphVisualizer';
import { MapVisualizer, SetVisualizer, GridVisualizer, UFVisualizer, PointerVisualizer } from './MiscVisualizers';
import { LinkedListVisualizer } from './LinkedListVisualizer';
import { BSTVisualizer } from './BSTVisualizer';
import { LogPanel } from './LogPanel';

export function VisualizerPanel() {
  const { traces, currentStep, metadata, isDarkMode } = useVisualizer();
  const family = metadata?.family || 'UNKNOWN';

  const currentState = useMemo(() => {
    return computeState(traces, currentStep);
  }, [traces, currentStep]);

  return (
    <div className={`flex flex-col h-full relative overflow-hidden transition-colors duration-300 ${
      isDarkMode ? 'bg-[#0f172a]' : 'bg-white'
    }`}>
      {/* Algorithm Family Label */}
      {traces.length > 0 && (
        <div className={`px-6 py-3 border-b flex items-center justify-between z-20 transition-all ${
          isDarkMode ? 'border-gray-800 bg-black/40' : 'border-gray-100 bg-gray-50/50'
        }`}>
          <div className="flex items-center gap-3">
            <span className={`text-xs font-mono uppercase tracking-widest ${isDarkMode ? 'text-gray-500' : 'text-gray-400'}`}>Algorithm Family</span>
            <div className={`px-3 py-1 rounded-full border text-sm font-bold tracking-tight transition-all ${
              isDarkMode ? 'bg-primary/10 border-primary/30 text-primary' : 'bg-blue-50 border-blue-200 text-blue-600'
            }`}>
              {family}
            </div>
          </div>
          <div className={`text-xs font-mono ${isDarkMode ? 'text-gray-500' : 'text-gray-400'}`}>
            {currentStep + 1} / {traces.length} Steps
          </div>
        </div>
      )}

      {/* Background Grid Pattern for IDE feel */}
      <div 
        className={`absolute inset-0 pointer-events-none transition-opacity duration-500 ${isDarkMode ? 'opacity-5' : 'opacity-[0.03]'}`}
        style={{
          backgroundImage: `radial-gradient(circle at 2px 2px, ${isDarkMode ? 'white' : 'black'} 1px, transparent 0)`,
          backgroundSize: '32px 32px'
        }}
      />
      
      <div className="flex-1 overflow-auto p-8 flex flex-col gap-12 z-0">
        {traces.length === 0 ? (
          <div className={`flex-1 flex flex-col items-center justify-center gap-4 transition-colors ${isDarkMode ? 'text-gray-500' : 'text-gray-400'}`}>
             <p className="text-lg font-medium">Code. Visualize. Understand.</p>
             <p className="text-sm opacity-60">Enter your code on the left and click "Visualize" to begin.</p>
          </div>
        ) : (
          <>
            {/* Render Arrays */}
            {Object.keys(currentState.arrays).length > 0 && (
              <div className="space-y-6">
                <h3 className="text-gray-400 font-semibold mb-2 flex items-center gap-2">
                  <span className="w-2 h-2 rounded-full bg-primary" /> Arrays
                </h3>
                {Object.entries(currentState.arrays).map(([id, arrState]: [string, any]) => (
                  <ArrayVisualizer key={id} id={id} values={arrState.values} colors={arrState.colors} />
                ))}
              </div>
            )}
            
            {/* Render Grids */}
            {Object.keys(currentState.grids).length > 0 && (
              <div className="space-y-6">
                {Object.entries(currentState.grids).map(([id, gState]: [string, any]) => (
                  <GridVisualizer key={id} id={id} rows={gState.rows} cols={gState.cols} defaultVal={gState.defaultVal} values={gState.values} colors={gState.colors} />
                ))}
              </div>
            )}

            {/* Render Linears (Stack, Queue, Deque, PQ) */}
            {Object.keys(currentState.linears).length > 0 && (
              <div className="space-y-6">
                {Object.entries(currentState.linears).map(([id, lState]: [string, any]) => (
                  <LinearVisualizer key={id} id={id} type={lState.type} values={lState.values} colors={lState.colors} />
                ))}
              </div>
            )}
            
            {/* Render Maps / Sets */}
            {Object.keys(currentState.maps).length > 0 && (
              <div className="space-y-6">
                {Object.entries(currentState.maps).map(([id, mState]: [string, any]) => (
                  <MapVisualizer key={id} id={id} entries={mState.entries} colors={mState.colors} />
                ))}
              </div>
            )}

            {Object.keys(currentState.sets).length > 0 && (
              <div className="space-y-6">
                {Object.entries(currentState.sets).map(([id, sState]: [string, any]) => (
                  <SetVisualizer key={id} id={id} values={sState.values} colors={sState.colors} />
                ))}
              </div>
            )}

            {/* Render UF */}
            {Object.keys(currentState.ufs).length > 0 && (
              <div className="space-y-6">
                {Object.entries(currentState.ufs).map(([id, uState]: [string, any]) => (
                  <UFVisualizer key={id} id={id} parent={uState.parent} />
                ))}
              </div>
            )}

            {/* Render Variables */}
            {Object.keys(currentState.variables).length > 0 && (
              <div className="space-y-4">
                <h3 className="text-gray-400 font-semibold mb-2 flex items-center gap-2">
                  <span className="w-2 h-2 rounded-full bg-secondary" /> Variables
                </h3>
                <div className="flex flex-wrap gap-4">
                  {Object.entries(currentState.variables).map(([name, varState]: [string, any]) => (
                    <VariableVisualizer key={name} name={name} value={varState.value} />
                  ))}
                </div>
              </div>
            )}

            {/* Render Pointers */}
            {Object.keys(currentState.pointers).length > 0 && (
              <div className="space-y-4">
                <h3 className="text-gray-400 font-semibold mb-2 flex items-center gap-2">
                  <span className="w-2 h-2 rounded-full bg-red-500" /> Pointers (Two-Pointers)
                </h3>
                <div className="flex flex-wrap gap-4">
                  {Object.entries(currentState.pointers).map(([name, pointerState]: [string, any]) => (
                    <PointerVisualizer key={name} name={name} pointer={pointerState} />
                  ))}
                </div>
              </div>
            )}
            
            {/* Render Graph / Tree */}
            {Object.keys(currentState.graphs).length > 0 && (
              <div className="space-y-6">
                {Object.entries(currentState.graphs).map(([id, gState]: [string, any]) => (
                  <GraphVisualizer key={id} id={id} nodes={gState.nodes} edges={gState.edges} />
                ))}
              </div>
            )}

            {/* Render Linked Lists (SLL / DLL) */}
            {Object.keys(currentState.linkedLists).length > 0 && (
              <div className="space-y-6">
                {Object.entries(currentState.linkedLists).map(([id, lState]: [string, any]) => (
                  <LinkedListVisualizer key={id} id={id} type={lState.type} nodes={lState.nodes} links={lState.links} />
                ))}
              </div>
            )}

            {/* Render BST */}
            {Object.keys(currentState.bsts).length > 0 && (
              <div className="space-y-6">
                {Object.entries(currentState.bsts).map(([id, bState]: [string, any]) => (
                  <BSTVisualizer key={id} id={id} nodes={bState.nodes} links={bState.links} />
                ))}
              </div>
            )}
            
          </>
        )}
      </div>

      <LogPanel logs={currentState.logs} />
    </div>
  );
}
