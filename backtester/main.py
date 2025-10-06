import asyncio
import uvicorn
import os

import server
import backtester


async def run_orders_api():
    """Run the FastAPI server (orders API)."""
    host = os.getenv("BT_OMS_HOST", "127.0.0.1")
    port = int(os.getenv("BT_OMS_PORT", "9000"))
    config = uvicorn.Config(
        server.app,
        host=host,
        port=port,
        log_level="debug",
        loop="asyncio",
    )
    server_ = uvicorn.Server(config)
    await server_.serve()


async def run_backtester():
    """Run the websocket feed (backtester)."""
    await backtester.main()


async def main():
    await asyncio.gather(
        run_orders_api(),
        run_backtester(),
    )


if __name__ == "__main__":
    asyncio.run(main())
