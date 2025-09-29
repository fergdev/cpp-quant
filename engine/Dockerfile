FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
  --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
  apt-get update && apt-get install -y --no-install-recommends \
  clang lld libc++-dev libc++abi-dev \
  build-essential cmake ninja-build git ca-certificates curl zip unzip tar \
  python3 python3-pip \
  && rm -rf /var/lib/apt/lists/*

RUN --mount=type=cache,target=/root/.cache/pip \
  pip3 install --no-cache-dir --break-system-packages "conan==2.*"

ENV CONAN_HOME=/root/.conan2
WORKDIR /src

COPY . .

ENV CC=clang CXX=clang++ \
  CMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
  CMAKE_MODULE_LINKER_FLAGS="-fuse-ld=lld" \
  CMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld"

RUN conan profile detect --force \
  && conan install . -s build_type=Release --build=missing

RUN cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \
  && cmake --build build -j

FROM ubuntu:24.04 AS run
ENV DEBIAN_FRONTEND=noninteractive
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
  --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
  apt-get update && apt-get install -y --no-install-recommends \
  ca-certificates libgcc-s1 \
  && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y --no-install-recommends libc++1 libc++abi1 && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /src/build/quant_engine /app/quant_engine
EXPOSE 8080
CMD ["/app/quant_engine"]
