import { LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer } from 'recharts';
import type { Tick } from '../hooks/useWS';

export function PriceSpark({ rows }: { rows: Tick[] }) {
  const data = rows.slice(-200).map(t => ({
    x: new Date(Number(String(t.ts).slice(0,13))).toLocaleTimeString(),
    y: t.px
  }));
  return (
    <div className="w-full h-48">
      <ResponsiveContainer>
        <LineChart data={data}>
          <XAxis dataKey="x" hide />
          <YAxis domain={['auto','auto']} width={60} />
          <Tooltip />
          <Line dataKey="y" dot={false} strokeWidth={2} />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}