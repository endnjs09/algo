export type TargetType =
  | 'array'
  | 'grid'
  | 'stack'
  | 'queue'
  | 'deque'
  | 'pq'
  | 'variable'
  | 'uf'
  | 'map'
  | 'set'
  | 'graph'
  | 'tree'
  | 'pointer'
  | 'string'
  | 'bitset'
  | 'log';

export interface BaseTraceEvent {
  target: TargetType;
  action: string;
  id?: string; // e.g. "arr", "q1", "g"
}

export interface ArrayEvent extends BaseTraceEvent {
  target: 'array';
  values?: any[];
  i?: number;
  j?: number;
  value?: any;
  color?: string;
  from?: number;
  to?: number;
}

export interface GridEvent extends BaseTraceEvent {
  target: 'grid';
  rows?: number;
  cols?: number;
  defaultVal?: number;
  row?: number;
  col?: number;
  value?: any;
  color?: string;
  r1?: number;
  c1?: number;
  r2?: number;
  c2?: number;
}

export interface LinearCollectionEvent extends BaseTraceEvent {
  target: 'stack' | 'queue' | 'deque' | 'pq';
  value?: any;
  index?: number;
  color?: string;
}

export interface VariableEvent extends BaseTraceEvent {
  target: 'variable';
  name: string;
  value?: any;
  color?: string;
}

export interface UFEvent extends BaseTraceEvent {
  target: 'uf';
  n?: number;
  a?: number;
  b?: number;
  root?: number;
  color?: string;
}

export interface MapEvent extends BaseTraceEvent {
  target: 'map';
  key?: any;
  value?: any;
  color?: string;
}

export interface SetEvent extends BaseTraceEvent {
  target: 'set';
  value?: any;
  color?: string;
}

export interface DocumentEvent extends BaseTraceEvent {
  target: 'string' | 'bitset';
  str?: string;
  n?: number;
  index?: number;
  value?: any;
  bitValue?: number;
  ch?: string;
  color?: string;
  from?: number;
  to?: number;
}

export interface GraphEvent extends BaseTraceEvent {
  target: 'graph' | 'tree';
  nodeId?: string | number;
  valueText?: string;
  x?: number;
  y?: number;
  u?: string | number;
  v?: string | number;
  weight?: string | number;
  directed?: boolean;
  color?: string;
  text?: string;
  style?: string;
  nodeIds?: (string | number)[];
}

export interface PointerEvent extends BaseTraceEvent {
  target: 'pointer';
  name: string;
  targetId?: string;
  targetIndex?: number;
  color?: string;
}

export interface LogEvent extends BaseTraceEvent {
  target: 'log';
  message: string;
}

export type TraceEvent = 
  | ArrayEvent 
  | GridEvent
  | LinearCollectionEvent
  | VariableEvent 
  | UFEvent
  | MapEvent
  | SetEvent
  | DocumentEvent
  | GraphEvent 
  | PointerEvent 
  | LogEvent 
  | BaseTraceEvent;

export interface VisualizeRequest {
  language: 'cpp';
  code: string;
  stdin: string;
  client_version: string;
}

export interface UIState {
  isPlaying: boolean;
  currentStep: number;
  totalSteps: number;
  playbackSpeed: number;
  isLoading: boolean;
  statusMessage: string;
  errorMessage: string | null;
}

