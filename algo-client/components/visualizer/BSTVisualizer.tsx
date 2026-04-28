"use client";

import React from 'react';

interface Node {
  id: number;
  value: any;
  color: string | null;
}

interface BSTProps {
  id: string;
  nodes: Record<number, Node>;
  links: Record<number, { left?: number; right?: number }>;
}

export function BSTVisualizer({ id, nodes, links }: BSTProps) {
  const nodeIds = Object.keys(nodes).map(Number);
  
  // Find root: first node created or node with no incoming links
  // Based on spec, let's assume the first created node is root if not specified.
  const findRoot = () => {
    const children = new Set(Object.values(links).flatMap(l => [l.left, l.right]).filter(n => n !== undefined && n !== -1));
    return nodeIds.find(id => !children.has(id)) || nodeIds[0];
  };

  const rootId = findRoot();

  // Recursive render to calculate positions
  const renderTree = (nodeId: number | undefined | -1, x: number, y: number, level: number) => {
    if (nodeId === undefined || nodeId === -1 || !nodes[nodeId]) return null;

    const node = nodes[nodeId];
    const leftId = links[nodeId]?.left;
    const rightId = links[nodeId]?.right;

    // Horizontal spacing decreases with depth
    const spread = 240 / Math.pow(1.5, level);

    return (
      <div className="absolute transition-all duration-700 ease-in-out" style={{ left: `${x}px`, top: `${y}px` }}>
        {/* Node */}
        <div className="relative group">
          <div 
            className={`
              w-12 h-12 rounded-full border-2 flex items-center justify-center font-bold text-white transition-all duration-500
              ${node.color ? `bg-${node.color}-500/20 border-${node.color}-500/50 shadow-[0_0_15px_rgba(var(--${node.color}-500-rgb),0.3)]` : 'bg-white/5 border-white/10 shadow-lg backdrop-blur-sm'}
            `}
          >
            {node.value}
          </div>
          <span className="absolute -top-6 left-1/2 -translate-x-1/2 text-[9px] font-mono text-gray-500 opacity-0 group-hover:opacity-100 transition-opacity">id:{nodeId}</span>
          
          {/* Edge to Left Child */}
          {leftId !== undefined && leftId !== -1 && nodes[leftId] && (
            <div 
              className="absolute bg-gray-700/50 origin-top-left transition-all duration-700"
              style={{
                top: '24px',
                left: '24px',
                height: '2px',
                width: `${Math.sqrt(Math.pow(spread, 2) + Math.pow(80, 2))}px`,
                transform: `rotate(${Math.atan2(80, -spread) * 180 / Math.PI}deg)`
              }}
            />
          )}

          {/* Edge to Right Child */}
          {rightId !== undefined && rightId !== -1 && nodes[rightId] && (
            <div 
              className="absolute bg-gray-700/50 origin-top-left transition-all duration-700"
              style={{
                top: '24px',
                left: '24px',
                height: '2px',
                width: `${Math.sqrt(Math.pow(spread, 2) + Math.pow(80, 2))}px`,
                transform: `rotate(${Math.atan2(80, spread) * 180 / Math.PI}deg)`
              }}
            />
          )}
        </div>

        {/* Children */}
        {renderTree(leftId, -spread, 80, level + 1)}
        {renderTree(rightId, spread, 80, level + 1)}
      </div>
    );
  };

  return (
    <div className="flex flex-col gap-4 p-6 rounded-2xl bg-black/10 border border-white/5 min-h-[400px] relative overflow-hidden">
      <div className="flex items-center gap-2 mb-2 relative z-10">
        <span className="text-xs font-bold uppercase tracking-widest text-gray-500">Binary Search Tree</span>
        <span className="text-xs px-2 py-0.5 rounded bg-purple-500/10 text-purple-400 font-mono">
          {id}
        </span>
      </div>

      <div className="flex-1 flex justify-center pt-12 relative scale-90 md:scale-100">
        <div className="relative">
           {rootId !== undefined ? renderTree(rootId, 0, 0, 0) : (
             <div className="text-gray-600 text-sm italic">No root created yet</div>
           )}
        </div>
      </div>
    </div>
  );
}
