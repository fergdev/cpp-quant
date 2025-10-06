import asyncio
import csv
import json
import os
import pathlib
from typing import Any, Iterable, Iterator, Tuple

import websockets

DATA_FILE = os.getenv("BT_DATA", "btcusd/btcusd_1-min_last_week.csv")
DELAY = float(os.getenv("BT_SPEED", "10"))
HOST = os.getenv("BT_FEED_HOST", "127.0.0.1")
PORT = int(os.getenv("BT_FEED_PORT", "8081"))
PATH = os.getenv("BT_FEED_PATH", "/ws")
SYMS = {s for s in os.getenv("BT_SYMBOLS", "").split(",") if s}

Row = Tuple[int, str, float, float, float, float]


def _as_int(x: Any) -> int:
    if x is None or x == "":
        return 0
    return int(float(x))


def _as_float(x: Any) -> float:
    if x is None or x == "":
        return 0.0
    return float(x)


def _load_ndjson(path: pathlib.Path) -> Iterator[Row]:
    with path.open() as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            j = json.loads(line)
            ts_ns = _as_int(j.get("ts_ns")) or (_as_int(j.get("ts")) * 1_000_000_000)
            sym = j.get("s") or j.get("sym") or "BTCUSDT"
            last = _as_float(j.get("c") or j.get("last"))
            bid = _as_float(j.get("b") or j.get("bid") or last)
            ask = _as_float(j.get("a") or j.get("ask") or last)
            vol = _as_float(j.get("v") or j.get("vol"))
            yield (ts_ns, sym, last, bid, ask, vol)


def _load_csv(path: pathlib.Path) -> Iterator[Row]:
    with path.open(newline="") as f:
        r = csv.DictReader(f)
        # Detect schema once from header
        fieldnames = [fn.lower() for fn in (r.fieldnames or [])]

        has_engine_schema = {"ts_ns", "sym", "last", "bid", "ask", "vol"}.issubset(
            fieldnames
        )
        has_ohlcv_schema = {"ts", "close", "volume"}.issubset(fieldnames)  # your file

        for row in r:
            row_l = {k.lower(): v for k, v in row.items()}

            if has_engine_schema:
                ts_ns = _as_int(row_l.get("ts_ns"))
                sym = row_l.get("sym") or "BTCUSDT"
                last = _as_float(row_l.get("last"))
                bid = _as_float(row_l.get("bid") or last)
                ask = _as_float(row_l.get("ask") or last)
                vol = _as_float(row_l.get("vol"))
            elif has_ohlcv_schema:
                # Map OHLCV → engine shape
                ts_ns = _as_int(row_l.get("ts")) * 1_000_000_000
                sym = row_l.get("sym") or "BTCUSDT"
                last = _as_float(row_l.get("close"))
                bid = last
                ask = last
                vol = _as_float(row_l.get("volume"))
            else:
                # Best-effort fallback
                ts_ns = _as_int(row_l.get("ts_ns")) or (
                    _as_int(row_l.get("ts")) * 1_000_000_000
                )
                sym = row_l.get("sym") or "BTCUSDT"
                last = _as_float(row_l.get("last") or row_l.get("close"))
                bid = _as_float(row_l.get("bid") or last)
                ask = _as_float(row_l.get("ask") or last)
                vol = _as_float(row_l.get("vol") or row_l.get("volume"))

            yield (ts_ns, sym, last, bid, ask, vol)


def load_rows(path: pathlib.Path) -> list[Row]:
    if not path.exists():
        raise FileNotFoundError(f"Data file not found: {path}")
    rows = list(
        _load_ndjson(path) if path.suffix.lower() == ".ndjson" else _load_csv(path)
    )
    rows.sort(key=lambda r: r[0])
    return rows


async def _paced_send(ws, rows: Iterable[Row]) -> None:
    rows = list(rows)
    if not rows:
        return

    for ts_ns, sym, last, bid, ask, vol in rows:
        if SYMS and sym not in SYMS:
            continue
        await ws.send(json.dumps({"s": sym, "c": last, "b": bid, "a": ask, "v": vol}))
        await asyncio.sleep(DELAY / 1000.0)


async def handler(ws, path=None):
    req_path = getattr(ws, "path", None) if path is None else path
    if req_path is None:
        req_path = PATH
    if req_path != PATH:
        await ws.close(code=1008, reason="invalid path")
        return

    try:
        rows = load_rows(pathlib.Path(DATA_FILE))
    except Exception as e:
        await ws.close(code=1011, reason=f"data error: {e}")
        return

    try:
        await _paced_send(ws, rows)
    except websockets.ConnectionClosedOK:
        pass
    except websockets.ConnectionClosedError:
        pass
    except Exception as e:
        try:
            await ws.close(code=1011, reason=str(e))
        except Exception:
            pass


async def main():
    print(
        f"[feeder] Serving ws://{HOST}:{PORT}{PATH}, "
        f"data={DATA_FILE}, delay={DELAY}millis, symbols={','.join(SYMS) or '(all)'}"
    )
    async with websockets.serve(
        handler,
        HOST,
        PORT,
        ping_interval=None,
        ping_timeout=None,
    ):
        await asyncio.Future()


if __name__ == "__main__":
    asyncio.run(main())
