set(ENGINE_SOURCES
  md_binance.cpp
  strategy_sma.cpp
  risk.cpp
  oms_paper.cpp
  ws_server.cpp
  tracing.cpp
)

add_library(quant_engine_lib ${ENGINE_SOURCES})
target_compile_features(quant_engine_lib PUBLIC cxx_std_20)

target_include_directories(quant_engine_lib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(quant_engine_lib
  PUBLIC
    Boost::headers
    Boost::system
    nlohmann_json::nlohmann_json
    OpenSSL::SSL OpenSSL::Crypto
    opentelemetry-cpp::exporter_otlp_http
    opentelemetry-cpp::exporter_otlp_http_client
    opentelemetry-cpp::http_client_curl
    Threads::Threads
    CURL::libcurl
    spdlog::spdlog
)

add_executable(quant_engine main.cpp)
target_compile_features(quant_engine PUBLIC cxx_std_20)
target_link_libraries(quant_engine PRIVATE quant_engine_lib)
