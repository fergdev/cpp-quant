FROM ubuntu:24.04 AS build

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
  --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
  apt-get update && apt-get install -y --no-install-recommends \
  build-essential cmake ninja-build git ca-certificates curl zip unzip tar \
  python3 python3-pip libboost-all-dev \
  && rm -rf /var/lib/apt/lists/*

RUN --mount=type=cache,target=/root/.cache/pip \
  pip3 install --no-cache-dir --break-system-packages "conan==2.*"

ENV CONAN_HOME=/root/.conan2
ENV PATH="/root/.local/bin:${PATH}"
WORKDIR /src

COPY conanfile.* CMakeLists.txt ./
RUN --mount=type=cache,target=/root/.conan2 \
  conan profile detect --force && \
  conan install . -of build -s build_type=Release --build=missing

COPY . .

ENV CMAKE_BUILD_PARALLEL_LEVEL=8
RUN cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=build/build/Release/generators/conan_toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build -j

FROM ubuntu:24.04 AS run
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
  --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
  apt-get update && apt-get install -y --no-install-recommends \
  ca-certificates libstdc++6 libgcc-s1 \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /src/build/quant_engine /app/quant_engine
ENV CPPQ_WS_PORT=8080
EXPOSE 8080
CMD ["/app/quant_engine"]
