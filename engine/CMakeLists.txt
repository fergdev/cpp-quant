cmake_minimum_required(VERSION 3.24)
project(quant_engine LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(opentelemetry-cpp CONFIG REQUIRED)

add_executable(quant_engine
  src/main.cpp
  src/md_binance.cpp
  src/strategy_sma.cpp
  src/risk.cpp
  src/oms_paper.cpp
  src/ws_server.cpp
  src/tracing.cpp
)

target_link_libraries(quant_engine PRIVATE
  Boost::system
  OpenSSL::SSL OpenSSL::Crypto
  nlohmann_json::nlohmann_json
  opentelemetry-cpp::opentelemetry-cpp
  opentelemetry-cpp::trace
  opentelemetry-cpp::resources
  opentelemetry-cpp::exporter_otlp_http
  opentelemetry-cpp::http_client_curl
)
