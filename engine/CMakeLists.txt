cmake_minimum_required(VERSION 3.23)
project(quant_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost CONFIG REQUIRED COMPONENTS system)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(opentelemetry-cpp CONFIG REQUIRED)
find_package(Threads REQUIRED)
# find_package(Curl REQUIRED)
find_package(CURL CONFIG REQUIRED)


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
    boost::boost
    Boost::system
    nlohmann_json::nlohmann_json
    OpenSSL::SSL OpenSSL::Crypto
    opentelemetry-cpp::exporter_otlp_http
    opentelemetry-cpp::exporter_otlp_http_client
    opentelemetry-cpp::http_client_curl
    Threads::Threads
    CURL::libcurl
)

