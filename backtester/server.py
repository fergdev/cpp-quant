from fastapi import FastAPI
from pydantic import BaseModel
import os
import time

app = FastAPI(title="Backtester OMS Mock")


class OrderReq(BaseModel):
    id: int
    sym: str
    side: str
    type: str
    qty: float
    px: float
    ts_ns: int
    last_px_hint: float | None = None


class OrderResp(BaseModel):
    id: int
    status: str
    filled_qty: float
    avg_px: float
    ts_ns: int


SLIPPAGE_BPS = float(os.getenv("BT_SLIPPAGE_BPS", "0.0"))


def apply_slippage(side: str, px: float) -> float:
    if SLIPPAGE_BPS <= 0:
        return px
    slip = px * (SLIPPAGE_BPS * 1e-4)
    return px + (slip if side.lower() == "buy" else -slip)


@app.post("/orders", response_model=OrderResp)
def place_order(req: OrderReq):
    px = req.last_px_hint or req.px or 0.0
    px = apply_slippage(req.side, px)

    return OrderResp(
        id=req.id,
        status="Filled",
        filled_qty=req.qty,
        avg_px=px,
        ts_ns=req.ts_ns,
    )


@app.get("/healthz")
def health():
    return {"ok": True}

