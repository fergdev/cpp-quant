import { useEffect, useRef, useState } from 'react';
export type Tick = { px: number; sym: string; ts: number; type: 'tick' };

export function useWS(url: string) {
  const [connected, setConnected] = useState(false);
  const [lastTick, setLastTick] = useState<Tick | null>(null);
  const wsRef = useRef<WebSocket | null>(null);
  const retryRef = useRef(0);

  useEffect(() => {
    let stopped = false;
    const connect = () => {
      const ws = new WebSocket(url);
      wsRef.current = ws;
      ws.onopen = () => { setConnected(true); retryRef.current = 0; };
      ws.onclose = () => {
        setConnected(false);
        if (stopped) return;
        const backoff = Math.min(1000 * 2 ** retryRef.current, 10000);
        retryRef.current++;
        setTimeout(connect, backoff);
      };
      ws.onerror = () => ws.close();
      ws.onmessage = (ev) => {
        try {
          const t = JSON.parse(ev.data);
          if (t?.type === 'tick') setLastTick(t);
        } catch {
          console.log('ws message parse error')
        }
      };
    };
    connect();
    return () => { stopped = true; wsRef.current?.close(); };
  }, [url]);

  return { connected, lastTick };
}
