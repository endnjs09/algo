"use client";

import React, { useEffect, useRef } from 'react';
import * as d3 from 'd3';

interface GraphVisualizerProps {
  id: string;
  nodes: Record<string | number, any>;
  edges: Record<string, any>;
}

export function GraphVisualizer({ id, nodes, edges }: GraphVisualizerProps) {
  const containerRef = useRef<HTMLDivElement>(null);
  const svgRef = useRef<SVGSVGElement>(null);

  // Persistent simulation state
  const simRef = useRef<d3.Simulation<any, any> | null>(null);
  const simNodesRef = useRef<Map<string, any>>(new Map());
  const prevCountRef = useRef({ nodes: 0, edges: 0 });

  // ── Refs read by the tick handler every tick — no stale closure ──
  const linkSelRef = useRef<d3.Selection<SVGPathElement, any, any, any> | null>(null);
  const edgeLabelSelRef = useRef<d3.Selection<SVGTextElement, any, any, any> | null>(null);
  const nodeSelRef = useRef<d3.Selection<SVGGElement, any, any, any> | null>(null);
  const simNodesCurRef = useRef<any[]>([]);

  const W = 800, H = 550, PAD = 40;

  // ── Effect 1: create simulation + register tick handler ONCE per graph id ──
  useEffect(() => {
    if (!svgRef.current) return;

    const sim = d3.forceSimulation<any>()
      .force("charge", d3.forceManyBody().strength(-400).distanceMax(300))
      .force("center", d3.forceCenter(W / 2, H / 2).strength(0.05))
      .force("collide", d3.forceCollide(40).iterations(2))
      .force("link", d3.forceLink<any, any>().distance(130).strength(0.8))
      .on("tick", () => {
        const link = linkSelRef.current;
        const edgeLabel = edgeLabelSelRef.current;
        const node = nodeSelRef.current;
        const simNodes = simNodesCurRef.current;
        if (!link || !edgeLabel || !node) return;

        // Clamp all nodes inside viewport
        simNodes.forEach((d: any) => {
          d.x = Math.max(PAD, Math.min(W - PAD, d.x ?? W / 2));
          d.y = Math.max(PAD, Math.min(H - PAD, d.y ?? H / 2));
        });

        // Compute SVG path string — straight for single edges, curved for bidirectional pairs
        const edgePath = (d: any) => {
          const sx = d.source?.x ?? 0, sy = d.source?.y ?? 0;
          let tx = d.target?.x ?? 0, ty = d.target?.y ?? 0;

          const R = 22; // Node radius (20) + small buffer

          if (!d.curveDir) {
            const dx = tx - sx, dy = ty - sy;
            const len = Math.sqrt(dx * dx + dy * dy) || 1;
            tx -= (dx / len) * R;
            ty -= (dy / len) * R;
            return `M${sx},${sy}L${tx},${ty}`;
          }

          const mx = (sx + tx) / 2, my = (sy + ty) / 2;
          const dx = tx - sx, dy = ty - sy;
          const len = Math.sqrt(dx * dx + dy * dy) || 1;
          const off = 35 * d.curveDir;
          const cx = mx + (-dy / len) * off, cy = my + (dx / len) * off;

          // Shorten bezier: move target end along the tangent (from control point to target)
          const tdx = tx - cx, tdy = ty - cy;
          const tlen = Math.sqrt(tdx * tdx + tdy * tdy) || 1;
          tx -= (tdx / tlen) * R;
          ty -= (tdy / tlen) * R;

          return `M${sx},${sy}Q${cx},${cy},${tx},${ty}`;
        };
        link.attr("d", (d: any) => edgePath(d));

        // Update edge labels — midpoint on curve with slight perpendicular offset
        const setLabelPos = (sel: any) => {
          sel.attr("transform", (d: any) => {
            const sx = d.source?.x, sy = d.source?.y;
            const tx = d.target?.x, ty = d.target?.y;
            if (sx == null || sy == null || tx == null || ty == null) return "translate(-1000,-1000)";

            const mx = (sx + tx) / 2, my = (sy + ty) / 2;
            const dx = tx - sx, dy = ty - sy;
            const len = Math.sqrt(dx * dx + dy * dy) || 1;

            // Curve control point (same logic as edgePath)
            const off = 35 * (d.curveDir ?? 0);
            const cx = mx + (-dy / len) * off, cy = my + (dx / len) * off;

            // Midpoint of quadratic bezier at t=0.5
            const lx = 0.25 * sx + 0.5 * cx + 0.25 * tx;
            const ly = 0.25 * sy + 0.5 * cy + 0.25 * ty;

            // Extra offset for label to be "outside" or "above" the curve
            // If it's a curve, push label further in the curve direction
            const lOff = d.curveDir ? 15 * d.curveDir : -10;
            const lox = (-dy / len) * lOff;
            const loy = (dx / len) * lOff;

            return `translate(${lx + lox},${ly + loy})`;
          });
        };
        setLabelPos(edgeLabel);

        // Update node positions
        node.attr("transform", (d: any) => `translate(${d.x},${d.y})`);
      });

    simRef.current = sim;

    return () => {
      sim.stop();
      simRef.current = null;
      simNodesRef.current.clear();
      prevCountRef.current = { nodes: 0, edges: 0 };
      // Clear selection refs so stale tick reads are skipped
      linkSelRef.current = null;
      edgeLabelSelRef.current = null;
      nodeSelRef.current = null;
      simNodesCurRef.current = [];
    };
  }, [id]); // recreate only when the graph changes

  // ── Effect 2: update D3 DOM and selection refs on every data change ──
  useEffect(() => {
    if (!svgRef.current || !simRef.current) return;
    const svg = d3.select(svgRef.current);
    const simulation = simRef.current;

    // -- Node filtering (Strict Node 1+ policy) --
    const currentEdges = Object.values(edges) as any[];

    const nodeIdsInEdges = new Set<string>();
    currentEdges.forEach((e) => {
      if (String(e.u) !== '0') nodeIdsInEdges.add(String(e.u));
      if (String(e.v) !== '0') nodeIdsInEdges.add(String(e.v));
    });

    const allNodes = Object.values(nodes) as any[];
    const currentNodes = allNodes.filter((n) =>
      String(n.id) !== '0' && (currentEdges.length === 0 || nodeIdsInEdges.has(String(n.id)))
    );

    // Reuse existing sim-node objects (preserves x, y, vx, vy from simulation)
    const newSimNodes = currentNodes.map((n) => {
      const existing = simNodesRef.current.get(String(n.id));
      if (existing) {
        existing.label = n.valueText ?? n.id;
        existing.color = n.color;
        return existing;
      }
      return {
        id: String(n.id),
        label: n.valueText ?? n.id,
        color: n.color,
        x: W / 2 + (Math.random() - 0.5) * 50,
        y: H / 2 + (Math.random() - 0.5) * 50,
        vx: 0, vy: 0,
      };
    });

    simNodesRef.current.clear();
    newSimNodes.forEach(n => simNodesRef.current.set(n.id, n));
    simNodesCurRef.current = newSimNodes;

    // Determine dashed style:
    //  1. Explicit backend style field ("dotted" → dashed, "solid" → solid)
    //  2. Fallback: heuristic — bidirectional pair where u > v is treated as reverse edge
    const edgeKeySet = new Set(currentEdges.map((e) => `${e.u}-${e.v}`));

    const newSimLinks = currentEdges.map((e) => {
      const hasPair = edgeKeySet.has(`${e.v}-${e.u}`);
      const isReverse = hasPair && String(e.u) > String(e.v);
      const isDashed = e.style === 'dotted' || (e.style == null && isReverse);
      // curveDir: always 1 for pairs. Since u->v and v->u have reversed vectors, 
      // they will naturally curve in opposite directions visually.
      const curveDir = hasPair ? 1 : 0;
      const rawLabel = e.text && e.text !== '' ? e.text
        : (e.weight != null && e.weight !== 0 ? String(e.weight) : null);
      return {
        source: newSimNodes.find(n => n.id === String(e.u)),
        target: newSimNodes.find(n => n.id === String(e.v)),
        label: rawLabel,                   // always show label
        isDashed: isDashed,
        curveDir: curveDir,
        hasPair: hasPair,
        directed: e.directed,
        color: e.color,
        key: `${e.u}-${e.v}`,
      };
    }).filter(l => l.source && l.target);

    // -- Simulation --
    simulation.nodes(newSimNodes);
    (simulation.force("link") as d3.ForceLink<any, any>).links(newSimLinks);

    const topologyChanged =
      currentNodes.length !== prevCountRef.current.nodes ||
      currentEdges.length !== prevCountRef.current.edges;
    if (topologyChanged) {
      simulation.alpha(0.3).restart();
      prevCountRef.current = { nodes: currentNodes.length, edges: currentEdges.length };
    }

    // -- Helpers --
    const c = (v: string | null | undefined, fb: string) =>
      v === 'red' ? '#ef4444' :
        v === 'green' ? '#22c55e' :
          v === 'blue' ? '#3b82f6' :
            v === 'yellow' ? '#eab308' :
              v === 'cyan' ? '#22d3ee' : fb;

    // -- Arrow markers (one per color) --
    const defs = svg.selectAll('defs').data([0]).join('defs');
    ([['default', '#6b7280'], ['red', '#ef4444'], ['green', '#22c55e'], ['blue', '#3b82f6'], ['yellow', '#eab308']] as const)
      .forEach(([name, fill]) => {
        defs.selectAll(`marker#arr-${name}`).data([0]).join('marker')
          .attr('id', `arr-${name}`)
          .attr('viewBox', '0 -5 10 10').attr('refX', 10).attr('refY', 0)
          .attr('markerWidth', 6).attr('markerHeight', 6).attr('orient', 'auto')
          .selectAll('path').data([0]).join('path')
          .attr('d', 'M0,-5L10,0L0,5').attr('fill', fill);
      });

    // -- Glow Filter for Cyan Nodes (Source/Sink) --
    defs.selectAll('filter#glow').data([0]).join('filter')
      .attr('id', 'glow').attr('x', '-50%').attr('y', '-50%').attr('width', '200%').attr('height', '200%')
      .html(`
        <feGaussianBlur stdDeviation="4" result="blur" />
        <feComposite in="SourceGraphic" in2="blur" operator="over" />
      `);

    const arrowId = (color: string | null | undefined) =>
      color === 'red' ? 'url(#arr-red)' :
        color === 'green' ? 'url(#arr-green)' :
          color === 'blue' ? 'url(#arr-blue)' :
            color === 'yellow' ? 'url(#arr-yellow)' :
              color === 'cyan' ? 'url(#arr-cyan)' : 'url(#arr-default)';

    // Add cyan arrow marker
    defs.selectAll('marker#arr-cyan').data([0]).join('marker')
      .attr('id', 'arr-cyan').attr('viewBox', '0 -5 10 10').attr('refX', 10).attr('refY', 0)
      .attr('markerWidth', 6).attr('markerHeight', 6).attr('orient', 'auto')
      .selectAll('path').data([0]).join('path').attr('d', 'M0,-5L10,0L0,5').attr('fill', '#22d3ee');

    // -- Render groups (created once, reused) --
    const edgesG = svg.selectAll('g.edges').data([0]).join('g').attr('class', 'edges');
    const elabelsG = svg.selectAll('g.elabels').data([0]).join('g').attr('class', 'elabels');
    const nodesG = svg.selectAll('g.vnodes').data([0]).join('g').attr('class', 'vnodes');

    // -- Edges (using <path> so bidirectional pairs can be curved apart) --
    const link = edgesG.selectAll<SVGPathElement, any>('path')
      .data(newSimLinks, (d) => d.key)
      .join(
        enter => enter.append('path').attr('fill', 'none'),
        update => update,
        exit => exit.remove()
      )
      .attr('stroke-width', d => d.isDashed ? 1.5 : 2.5)
      .attr('stroke', d => c(d.color, d.isDashed ? '#6b7280' : '#94a3b8'))
      .attr('stroke-dasharray', d => d.isDashed ? '5,5' : null)
      .attr('stroke-opacity', d => d.isDashed ? 0.8 : 1)
      .attr('marker-end', d => (d.directed || d.hasPair) ? arrowId(d.color) : null);

    link.transition().duration(300)
      .attr('stroke-width', d => d.isDashed ? 1.5 : 2.5)
      .attr('stroke', d => c(d.color, d.isDashed ? '#6b7280' : '#94a3b8'))
      .attr('stroke-opacity', d => d.isDashed ? 0.8 : 1)
      .attr('marker-end', d => (d.directed || d.hasPair) ? arrowId(d.color) : null);

    // -- Edge labels --
    const edgeLabel = elabelsG.selectAll<SVGTextElement, any>('text')
      .data(newSimLinks.filter(d => d.label != null), (d) => d.key)
      .join(
        enter => enter.append('text')
          .attr('font-size', '13px').attr('font-weight', 'bold')
          .attr('text-anchor', 'middle').attr('dominant-baseline', 'middle')
          .attr('font-family', 'monospace')
          .style('paint-order', 'stroke').style('stroke', '#0f0f1f')
          .style('stroke-width', '4px').style('stroke-linecap', 'round')
          .style('stroke-linejoin', 'round').attr('fill', '#e5e7eb'),
        update => update,
        exit => exit.remove()
      )
      .text(d => d.label);

    // -- Nodes --
    const node = nodesG.selectAll<SVGGElement, any>('g.nd')
      .data(newSimNodes, (d) => d.id)
      .join(
        enter => {
          const g = enter.append('g').attr('class', 'nd');
          g.append('circle').attr('r', 20).attr('stroke-width', 2);
          g.append('text')
            .attr('text-anchor', 'middle').attr('dominant-baseline', 'central')
            .attr('font-size', '13px').attr('font-weight', 'bold').attr('font-family', 'monospace');
          return g;
        },
        update => update,
        exit => exit.remove()
      );

    const circle = node.selectAll('circle').data(d => [d]).join('circle')
      .attr('r', 20)
      .attr('fill', d => d.color === 'cyan' ? '#083344' : c(d.color, '#1e1e38'))
      .attr('stroke', d => d.color === 'cyan' ? '#22d3ee' : '#374151')
      .attr('stroke-width', d => d.color === 'cyan' ? 4 : 2)
      .style('filter', d => d.color === 'cyan' ? 'url(#glow)' : null);

    node.select('text')
      .transition().duration(300)
      .text(d => d.label)
      .attr('fill', d => {
        if (d.color) return '#0f172a'; // Dark black for any colored node for consistency
        return '#e2e8f0'; // Light gray for default uncolored nodes
      });

    // -- Drag --
    const drag = d3.drag<SVGGElement, any>()
      .on("start", (event, d) => {
        if (!event.active) simulation.alphaTarget(0.3).restart();
        d.fx = d.x; d.fy = d.y;
      })
      .on("drag", (event, d) => { d.fx = event.x; d.fy = event.y; })
      .on("end", (event, d) => { if (!event.active) simulation.alphaTarget(0); });

    node.call(drag);

    // ── Publish latest selections to refs so the tick handler reads current data ──
    linkSelRef.current = link as any;
    edgeLabelSelRef.current = edgeLabel as any;
    nodeSelRef.current = node;

    // -- Immediate Position Sync (prevents (0,0) jump) --
    const edgePath = (d: any) => {
      const sx = d.source?.x ?? 0, sy = d.source?.y ?? 0;
      let tx = d.target?.x ?? 0, ty = d.target?.y ?? 0;
      const R = 22;

      if (!d.curveDir) {
        const dx = tx - sx, dy = ty - sy;
        const len = Math.sqrt(dx * dx + dy * dy) || 1;
        tx -= (dx / len) * R;
        ty -= (dy / len) * R;
        return `M${sx},${sy}L${tx},${ty}`;
      }
      const mx = (sx + tx) / 2, my = (sy + ty) / 2;
      const dx = tx - sx, dy = ty - sy;
      const len = Math.sqrt(dx * dx + dy * dy) || 1;
      const off = 35 * d.curveDir;
      const cx = mx + (-dy / len) * off, cy = my + (dx / len) * off;

      const tdx = tx - cx, tdy = ty - cy;
      const tlen = Math.sqrt(tdx * tdx + tdy * tdy) || 1;
      tx -= (tdx / tlen) * R;
      ty -= (tdy / tlen) * R;

      return `M${sx},${sy}Q${cx},${cy},${tx},${ty}`;
    };
    link.attr("d", edgePath);

    edgeLabel.attr("transform", (d: any) => {
      const sx = d.source?.x ?? 0, sy = d.source?.y ?? 0;
      const tx = d.target?.x ?? 0, ty = d.target?.y ?? 0;
      const mx = (sx + tx) / 2, my = (sy + ty) / 2;
      const dx = tx - sx, dy = ty - sy;
      const len = Math.sqrt(dx * dx + dy * dy) || 1;
      const off = 35 * (d.curveDir ?? 0);
      const cx = mx + (-dy / len) * off, cy = my + (dx / len) * off;
      const lx = 0.25 * sx + 0.5 * cx + 0.25 * tx;
      const ly = 0.25 * sy + 0.5 * cy + 0.25 * ty;
      const lOff = d.curveDir ? 15 * d.curveDir : -10;
      return `translate(${lx + (-dy / len) * lOff},${ly + (dx / len) * lOff})`;
    });

  }, [nodes, edges, id]);

  return (
    <div className="flex flex-col gap-2 h-full">
      <div className="text-sm text-gray-500 font-mono font-bold uppercase tracking-wider">
        Graph/Tree: {id}
      </div>
      <div ref={containerRef} className="bg-[#0f0f1f] border border-gray-800/50 rounded-xl overflow-hidden min-h-[570px] flex items-center justify-center relative">
        <svg ref={svgRef} viewBox={`0 0 800 550`} className="w-full h-full min-h-[570px]" />
      </div>
    </div>
  );
}