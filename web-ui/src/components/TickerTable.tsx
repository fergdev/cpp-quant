import type { Tick } from '../hooks/useWS';

export function TickerTable({ rows }: { rows: Tick[] }) {
  const recent = rows.slice(-50).reverse();
  return (
    <table className="w-full max-w-3xl border-collapse">
      <thead>
        <tr className="text-left border-b border-gray-200">
          <th className="py-2 pr-4">Timestamp</th>
          <th className="py-2 pr-4">Symbol</th>
          <th className="py-2">Last</th>
        </tr>
      </thead>
      <tbody>
        {recent.map((t, i) => (
          <tr key={t.ts + ':' + i} className="border-b border-gray-100">
            <td className="py-2 pr-4">{new Date(Number(String(t.ts).slice(0, 13))).toLocaleTimeString()}</td>
            <td className="py-2 pr-4">{t.sym}</td>
            <td className="py-2 font-medium">{t.px.toLocaleString()}</td>
          </tr>
        ))}
      </tbody>
    </table>
  );
}
