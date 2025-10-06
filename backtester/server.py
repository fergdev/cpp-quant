from fastapi import FastAPI, Request
from pydantic import BaseModel
import os
import time as systime
import logging

app = FastAPI(title="Backtester OMS Mock")


class OrderReq(BaseModel):
    symbol: str
    side: str
    type: str
    quantity: float
    timestamp: int


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


@app.post("/api/v3/order", response_model=OrderResp)
async def place_order(req: Request):
    body = await req.body()

    logger.info("Info")
    logger.info(body.decode("utf-8"))  # Decode bytes to a string for printing

    # px = req.last_px_hint or req.px or 0.0
    # px = apply_slippage(req.side, px)
    #
    # return OrderResp(
    #     id=req.id,
    #     status="Filled",
    #     filled_qty=req.qty,
    #     avg_px=px,
    #     ts_ns=req.ts_ns,
    # )
    # px = req.last_px_hint or req.px or 0.0
    # px = apply_slippage(req.side, px)

    return OrderResp(id=1, status="Filled", filled_qty=1.0, avg_px=1, ts_ns=1)


# set logging to debug
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger("uvicorn")


@app.middleware("http")
async def log_requests(request: Request, call_next):
    body = await request.body()
    logger.debug(f"--> {request.method} {request.url}")
    logger.debug(f"Headers: {dict(request.headers)}")
    if body:
        logger.debug(f"Body: {body.decode(errors='ignore')}")

    response = await call_next(request)

    logger.debug(f"<-- {response.status_code} {request.url}")
    return response


class TimeResp(BaseModel):
    serverTime: int


@app.get("/api/v3/time", response_model=TimeResp)
def time():
    t = int(systime.time())
    return TimeResp(serverTime=t)


@app.get("/healthz")
def health():
    return {"ok": True}
