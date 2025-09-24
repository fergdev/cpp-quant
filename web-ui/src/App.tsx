import { useEffect, useMemo, useState } from 'react';
import { useWS, type Tick } from './hooks/useWS';
import { TickerTable } from './components/TickerTable';
import { PriceSpark } from './components/PriceSpark';
import './index.css';

const wsUrl = (() => {
  const proto = location.protocol === 'https:' ? 'wss' : 'ws';
  return `${proto}://${location.host}/ws`;
})();

export default function App() {
  const { connected, lastTick } = useWS(wsUrl);
  const [rows, setRows] = useState<Tick[]>([]);
  useEffect(() => { if (lastTick) setRows(prev => [...prev, lastTick].slice(-5000)); }, [lastTick]);
  const latest = useMemo(() => rows.at(-1), [rows]);

  return (
    <div className="p-6 space-y-6 max-w-5xl mx-auto">
      <header className="flex items-center justify-between">
        <h1 className="text-2xl font-bold">cpp-quant Dashboard</h1>
        <div className={`text-sm px-2 py-1 rounded ${connected ? 'bg-green-100 text-green-700' : 'bg-red-100 text-red-700'}`}>
          {connected ? 'Live' : 'Disconnected'}
        </div>
      </header>

      <section className="grid md:grid-cols-2 gap-6">
        <div className="rounded-2xl shadow p-4">
          <h2 className="font-semibold mb-2">Price (spark)</h2>
          <PriceSpark rows={rows} />
        </div>
        <div className="rounded-2xl shadow p-4">
          <h2 className="font-semibold mb-2">Latest</h2>
          <div className="text-3xl font-bold">
            {latest ? latest.px.toLocaleString() : '—'}
          </div>
          <div className="text-gray-500">{latest ? latest.sym : ''}</div>
        </div>
      </section>

      <section className="rounded-2xl shadow p-4">
        <h2 className="font-semibold mb-2">Recent Ticks</h2>
        <TickerTable rows={rows} />
      </section>
    </div>
  );
}
