import { TraceEvent } from './types';

export function computeState(traces: TraceEvent[], currentStep: number) {
  const state: any = {
    arrays: {},
    grids: {},
    linears: {}, // for stack, queue, deque, pq
    variables: {},
    ufs: {},
    maps: {},
    sets: {},
    strings: {},
    bitsets: {},
    graphs: {},
    pointers: {},
    linkedLists: {},
    bsts: {},
    logs: [],
  };

  for (let i = 0; i <= currentStep; i++) {
    if (!traces[i]) continue;
    const trace = traces[i] as any;
    const id = trace.id || 'default';
    
    switch (trace.target) {
      case 'array':
        if (!state.arrays[id]) state.arrays[id] = { values: [], colors: {} };
        if (trace.action === 'set') {
          state.arrays[id].values = [...trace.values];
          state.arrays[id].colors = {};
        } else if (trace.action === 'swap') {
          const temp = state.arrays[id].values[trace.i];
          state.arrays[id].values[trace.i] = state.arrays[id].values[trace.j];
          state.arrays[id].values[trace.j] = temp;
        } else if (trace.action === 'update_value' || trace.action === 'update') {
          state.arrays[id].values[trace.index] = trace.value;
        } else if (trace.action === 'push_back') {
          state.arrays[id].values.push(trace.value);
        } else if (trace.action === 'pop_back') {
          state.arrays[id].values.pop();
        } else if (trace.action === 'insert') {
          state.arrays[id].values.splice(trace.index, 0, trace.value);
        } else if (trace.action === 'erase') {
          state.arrays[id].values.splice(trace.index, 1);
        } else if (trace.action === 'color') {
          if (trace.indices) {
            trace.indices.forEach((idx: number) => state.arrays[id].colors[idx] = trace.color);
          } else if (trace.i !== undefined || trace.index !== undefined) {
            state.arrays[id].colors[trace.i !== undefined ? trace.i : trace.index] = trace.color;
          }

        } else if (trace.action === 'range_color') {
          for (let c = trace.from; c <= trace.to; c++) state.arrays[id].colors[c] = trace.color;
        } else if (trace.action === 'reset_color') {
          state.arrays[id].colors = {};
        }
        break;
      
      case 'variable':
        if (!state.variables[trace.name]) state.variables[trace.name] = { color: null };
        if (trace.action === 'set' || trace.action === 'update') {
          state.variables[trace.name].value = trace.value;
        } else if (trace.action === 'color') {
          state.variables[trace.name].color = trace.color;
        }
        break;

      case 'stack':
      case 'queue':
      case 'deque':
      case 'pq':
        if (!state.linears[id]) state.linears[id] = { type: trace.target, values: [], colors: {} };
        if (trace.action.startsWith('create')) {
           state.linears[id] = { type: trace.target, values: [], colors: {} };
        } else if (trace.action === 'push' || trace.action === 'push_back') {
           state.linears[id].values.push(trace.value);
        } else if (trace.action === 'pop' || trace.action === 'pop_back') {
           state.linears[id].values.pop();
        } else if (trace.action === 'push_front') {
           state.linears[id].values.unshift(trace.value);
        } else if (trace.action === 'pop_front') {
           state.linears[id].values.shift();
        } else if (trace.action === 'color') {
           state.linears[id].colors[trace.index] = trace.color;
        }
        break;
        
      case 'map':
        if (!state.maps[id]) state.maps[id] = { entries: {}, colors: {} };
        if (trace.action === 'create') state.maps[id] = { entries: {}, colors: {} };
        else if (trace.action === 'set') state.maps[id].entries[trace.key] = trace.value;
        else if (trace.action === 'erase') delete state.maps[id].entries[trace.key];
        else if (trace.action === 'color') state.maps[id].colors[trace.key] = trace.color;
        break;

      case 'set':
        if (!state.sets[id]) state.sets[id] = { values: [], colors: {} };
        if (trace.action === 'create') state.sets[id] = { values: [], colors: {} };
        else if (trace.action === 'insert') {
          if (!state.sets[id].values.includes(trace.value)) state.sets[id].values.push(trace.value);
        }
        else if (trace.action === 'erase') {
          state.sets[id].values = state.sets[id].values.filter((v:any) => v !== trace.value);
        }
        else if (trace.action === 'color') {
          state.sets[id].colors[trace.value] = trace.color;
        }
        break;
      
      case 'pointer':
        if (trace.action === 'set') {
           state.pointers[trace.name] = { targetId: trace.target_obj || trace.targetId, targetIndex: trace.target_id || trace.targetIndex, color: trace.color || 'primary' };
        } else if (trace.action === 'remove') {
           delete state.pointers[trace.name];
        } else if (trace.action === 'color') {
           if (state.pointers[trace.name]) state.pointers[trace.name].color = trace.color;
        }
        break;
        
      case 'grid':
        if (!state.grids[id]) state.grids[id] = { rows: 0, cols: 0, defaultVal: 0, values: {}, colors: {} };
        if (trace.action === 'create') {
           state.grids[id] = { rows: trace.rows, cols: trace.cols, defaultVal: trace.default_val || trace.defaultVal, values: {}, colors: {} };
        } else if (trace.action === 'set_value' || trace.action === 'setValue') {
           state.grids[id].values[`${trace.row},${trace.col}`] = trace.value;
        } else if (trace.action === 'color') {
           state.grids[id].colors[`${trace.row},${trace.col}`] = trace.color;
        } else if (trace.action === 'range_color') {
           for (let r = trace.r1; r <= trace.r2; r++) {
              for (let c = trace.c1; c <= trace.c2; c++) {
                 state.grids[id].colors[`${r},${c}`] = trace.color;
              }
           }
        } else if (trace.action === 'reset_color') {
           state.grids[id].colors = {};
        }
        break;

      case 'uf':
        if (!state.ufs[id]) state.ufs[id] = { parent: {} };
        if (trace.action === 'create') {
          for (let i = 0; i <= trace.n; i++) state.ufs[id].parent[i] = i; 
        } else if (trace.action === 'find') {
          state.ufs[id].parent[trace.a] = trace.root;
        }
        break;

      case 'string':
        if (!state.strings[id]) state.strings[id] = { str: '', colors: {} };
        if (trace.action === 'set') state.strings[id].str = trace.value || trace.str || '';
        else if (trace.action === 'color') state.strings[id].colors[trace.index] = trace.color;
        else if (trace.action === 'range_color') {
           for(let i=trace.from; i<=trace.to; i++) state.strings[id].colors[i] = trace.color;
        }
        else if (trace.action === 'update_char') {
           let arr = state.strings[id].str.split('');
           if (arr[trace.index]) arr[trace.index] = trace.char || trace.ch;
           state.strings[id].str = arr.join('');
        }
        break;
        
      case 'bitset':
        if (!state.bitsets[id]) state.bitsets[id] = { size: 0, values: {}, colors: {} };
        if (trace.action === 'create') state.bitsets[id].size = trace.n;
        else if (trace.action === 'set_bit') state.bitsets[id].values[trace.index] = trace.bit || trace.bitValue;
        else if (trace.action === 'color') state.bitsets[id].colors[trace.index] = trace.color;
        else if (trace.action === 'range_color') {
           for(let i=trace.from; i<=trace.to; i++) state.bitsets[id].colors[i] = trace.color;
        }
        break;

      case 'graph':
      case 'tree':
        const graphId = 'default_graph';
        if (!state.graphs[graphId]) state.graphs[graphId] = { nodes: {}, edges: {} };
        const g = state.graphs[graphId];
        
        const nId = trace.id !== undefined ? trace.id : trace.nodeId;

        if (trace.action === 'create_node') {
          g.nodes[nId] = { id: nId, valueText: trace.value, x: 0, y: 0, color: null };
        } else if (trace.action === 'update_node') {
          if (g.nodes[nId]) g.nodes[nId].valueText = trace.value;
        } else if (trace.action === 'color_node') {
          if (g.nodes[nId]) g.nodes[nId].color = trace.color;
        } else if (trace.action === 'set_node_pos') {
          if (g.nodes[nId]) {
            g.nodes[nId].x = trace.x;
            g.nodes[nId].y = trace.y;
          }
        } else if (trace.action === 'create_edge') {
          const edgeId = `${trace.u}-${trace.v}`;
          g.edges[edgeId] = { u: trace.u, v: trace.v, weight: trace.weight, directed: trace.directed, color: null, text: trace.text || '', style: trace.style || null };
        } else if (trace.action === 'remove_edge') {
          const edgeId = `${trace.u}-${trace.v}`;
          delete g.edges[edgeId];
        } else if (trace.action === 'color_edge') {
          if (g.edges[`${trace.u}-${trace.v}`]) g.edges[`${trace.u}-${trace.v}`].color = trace.color;
          else if (g.edges[`${trace.v}-${trace.u}`]) g.edges[`${trace.v}-${trace.u}`].color = trace.color;
        } else if (trace.action === 'update_edge_weight') {
           if (g.edges[`${trace.u}-${trace.v}`]) g.edges[`${trace.u}-${trace.v}`].weight = trace.weight;
           else if (g.edges[`${trace.v}-${trace.u}`]) g.edges[`${trace.v}-${trace.u}`].weight = trace.weight;
        } else if (trace.action === 'update_edge_text') {
           if (g.edges[`${trace.u}-${trace.v}`]) g.edges[`${trace.u}-${trace.v}`].text = trace.text;
           else if (g.edges[`${trace.v}-${trace.u}`]) g.edges[`${trace.v}-${trace.u}`].text = trace.text;
        } else if (trace.action === 'update_edge_style') {
           if (g.edges[`${trace.u}-${trace.v}`]) g.edges[`${trace.u}-${trace.v}`].style = trace.style;
           else if (g.edges[`${trace.v}-${trace.u}`]) g.edges[`${trace.v}-${trace.u}`].style = trace.style;
        }
        break;

      case 'log':
        state.logs.push(trace.message || trace.text);
        break;

      case 'sll':
      case 'dll':
        if (!state.linkedLists[id]) state.linkedLists[id] = { type: trace.target, nodes: {}, links: {} };
        const ll = state.linkedLists[id];
        if (trace.action === 'create') {
          state.linkedLists[id] = { type: trace.target, nodes: {}, links: {} };
        } else if (trace.action === 'create_node') {
          ll.nodes[trace.node_id] = { id: trace.node_id, value: trace.value, color: null };
        } else if (trace.action === 'set_next') {
          ll.links[trace.node_id] = { ...ll.links[trace.node_id], next: trace.next_id };
        } else if (trace.action === 'set_prev') {
          ll.links[trace.node_id] = { ...ll.links[trace.node_id], prev: trace.prev_id };
        } else if (trace.action === 'delete_node') {
          delete ll.nodes[trace.node_id];
          delete ll.links[trace.node_id];
        } else if (trace.action === 'color_node') {
          if (ll.nodes[trace.node_id]) ll.nodes[trace.node_id].color = trace.color;
        } else if (trace.action === 'update_value') {
          if (ll.nodes[trace.node_id]) ll.nodes[trace.node_id].value = trace.value;
        }
        break;

      case 'bst':
        if (!state.bsts[id]) state.bsts[id] = { nodes: {}, links: {} };
        const bst = state.bsts[id];
        if (trace.action === 'create') {
          state.bsts[id] = { nodes: {}, links: {} };
        } else if (trace.action === 'create_node') {
          bst.nodes[trace.node_id] = { id: trace.node_id, value: trace.value, color: null };
        } else if (trace.action === 'set_left') {
          bst.links[trace.node_id] = { ...bst.links[trace.node_id], left: trace.left_id };
        } else if (trace.action === 'set_right') {
          bst.links[trace.node_id] = { ...bst.links[trace.node_id], right: trace.right_id };
        } else if (trace.action === 'delete_node') {
          delete bst.nodes[trace.node_id];
          delete bst.links[trace.node_id];
        } else if (trace.action === 'color_node') {
          if (bst.nodes[trace.node_id]) bst.nodes[trace.node_id].color = trace.color;
        } else if (trace.action === 'update_value') {
          if (bst.nodes[trace.node_id]) bst.nodes[trace.node_id].value = trace.value;
        }
        break;
    }
  }

  return state;
}
