"use client";

import React from 'react';
import { ArrowRight, ArrowLeftRight, Minus } from 'lucide-react';

interface Node {
  id: number;
  value: any;
  color: string | null;
}

interface LinkedListProps {
  id: string;
  type: 'sll' | 'dll';
  nodes: Record<number, Node>;
  links: Record<number, { next?: number; prev?: number }>;
}

export function LinkedListVisualizer({ id, type, nodes, links }: LinkedListProps) {
  // 1. Find the head (id="head" or the first node with no incoming links if possible)
  // For simplicity based on spec, we follow links starting from a probable head.
  // Actually, we can just render all nodes that are part of this 'id' panel.
  
  const nodeIds = Object.keys(nodes).map(Number);
  
  // We'll use a simple approach: find nodes that have links and order them.
  // In a real scenario, we'd start from 'head' ID.
  const renderNodes = () => {
    // Basic heuristic to find head: a node that is not 'next' of anyone else.
    const targets = new Set(Object.values(links).map(l => l.next).filter(n => n !== undefined && n !== -1));
    let headId = nodeIds.find(id => !targets.has(id));
    
    // If multiple components or cycles, just pick first available if headId not found
    if (headId === undefined) headId = nodeIds[0];

    const ordered: number[] = [];
    const visited = new Set<number>();
    let curr: number | undefined = headId;
    
    while (curr !== undefined && curr !== -1 && !visited.has(curr) && nodes[curr]) {
      ordered.push(curr);
      visited.add(curr);
      curr = links[curr]?.next;
    }

    // Add remaining orphan nodes if any
    nodeIds.forEach(id => {
      if (!visited.has(id)) ordered.push(id);
    });

    return ordered;
  };

  const orderedIds = renderNodes();

  return (
    <div className="flex flex-col gap-4 p-4 rounded-xl bg-black/10 border border-white/5 shadow-inner">
      <div className="flex items-center gap-2 mb-2">
        <span className="text-xs font-bold uppercase tracking-widest text-gray-500">
          {type === 'sll' ? 'Singly Linked List' : 'Doubly Linked List'}
        </span>
        <span className="text-xs px-2 py-0.5 rounded bg-blue-500/10 text-blue-400 font-mono">
          {id}
        </span>
      </div>

      <div className="flex items-center gap-2 overflow-x-auto pb-4">
        {orderedIds.map((nodeId, index) => {
          const node = nodes[nodeId];
          const link = links[nodeId];
          const hasNext = link?.next !== undefined && link.next !== -1;
          const isNull = link?.next === -1;

          return (
            <React.Fragment key={nodeId}>
              {/* Node Box */}
              <div className="flex flex-col items-center">
                <div 
                  className={`
                    relative w-16 h-16 rounded-xl border-2 flex flex-col items-center justify-center transition-all duration-500
                    ${node.color ? `bg-${node.color}-500/20 border-${node.color}-500/50 shadow-[0_0_15px_rgba(var(--${node.color}-500-rgb),0.2)]` : 'bg-white/5 border-white/10'}
                  `}
                >
                  <span className="text-xl font-bold text-white">{node.value}</span>
                  <span className="absolute -top-6 text-[10px] font-mono text-gray-500">id:{nodeId}</span>
                </div>
              </div>

              {/* Arrow */}
              {hasNext && (
                <div className="flex items-center text-blue-500 animate-pulse">
                  {type === 'sll' ? <ArrowRight size={24} /> : <ArrowLeftRight size={24} />}
                </div>
              )}

              {/* NULL marker */}
              {isNull && (
                <div className="flex items-center gap-2">
                  <div className="text-blue-500"><ArrowRight size={24} /></div>
                  <div className="w-12 h-8 rounded-lg bg-red-500/10 border border-red-500/30 flex items-center justify-center">
                    <span className="text-[10px] font-black text-red-400 uppercase tracking-tighter">NULL</span>
                  </div>
                </div>
              )}
            </React.Fragment>
          );
        })}
        
        {orderedIds.length === 0 && (
          <div className="text-gray-600 text-sm italic">No nodes created yet</div>
        )}
      </div>
    </div>
  );
}
