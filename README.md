# cpp-quant

⚡ High-performance single-binary quantitative trading engine in modern C++20.  
Includes in-process actors for market data, strategy, risk, order management, persistence, and a WebSocket gateway.

![Build](https://github.com/fergdev/cpp-quant/actions/workflows/cpp.yml/badge.svg)

---

## Features

- **Market Data Actor** – connects to Binance (WebSocket) and normalizes ticks  
- **Strategy Actor** – demo SMA strategy generating buy/sell signals  
- **Risk Actor** – simple position limits  
- **OMS (Paper)** – fills orders at last tick price  
- **Persistence** – writes ticks and fills into SQLite  
- **WebSocket Server** – broadcasts ticks and fills to clients in real-time  

All actors run **in a single executable** with lock-free queues for maximum speed.

---

## Build

Requirements:

- CMake ≥ 3.24
- Ninja
- vcpkg
- A modern C++20 compiler (Clang/LLVM on macOS, GCC/Clang on Linux)

Clone and build:

```bash
git clone https://github.com/fergdev/cpp-quant.git
cd cpp-quant

# configure & build
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_FEATURE_FLAGS=manifests
cmake --build build -j
