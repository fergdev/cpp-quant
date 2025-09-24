FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
RUN --mount=type=cache,target=/var/cache/apt \
  apt-get update && apt-get install -y --no-install-recommends \
  build-essential cmake ninja-build git ca-certificates pkg-config curl \
  zip unzip tar ccache linux-libc-dev && \
  rm -rf /var/lib/apt/lists/*

ENV VCPKG_ROOT=/opt/vcpkg
WORKDIR /opt
RUN git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT" && \
  $VCPKG_ROOT/bootstrap-vcpkg.sh

WORKDIR /src
COPY vcpkg.json vcpkg-configuration.json* ./

ENV VCPKG_BINARY_SOURCES="clear;files,/root/.cache/vcpkg/bincache,readwrite"

RUN --mount=type=cache,target=/root/.cache/vcpkg \
  $VCPKG_ROOT/vcpkg install --x-manifest-root=/src --feature-flags=manifests

COPY . .

ENV CCACHE_DIR=/root/.cache/ccache
RUN --mount=type=cache,target=/root/.cache/vcpkg \
  --mount=type=cache,target=/root/.cache/ccache \
  cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_FEATURE_FLAGS=manifests \
  -DCMAKE_C_COMPILER_LAUNCHER=ccache \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && \
  cmake --build build -j
# ---------- runtime ----------
FROM ubuntu:24.04 AS run
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
  ca-certificates \
  libstdc++6 \
  libgcc-s1 \
  libssl3 \
  libsqlite3-0 \
  && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=build /src/build/cpp_quant /app/cpp_quant
EXPOSE 8080
ENV CPPQ_WS_PORT=8080
CMD ["/app/cpp_quant"]
