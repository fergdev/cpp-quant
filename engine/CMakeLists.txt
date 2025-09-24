cmake_minimum_required(VERSION 3.24)
project(quant_engine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED CONFIG COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)

add_executable(quant_engine
  src/main.cpp
  src/md_binance.cpp
  src/strategy_sma.cpp
  src/risk.cpp
  src/oms_paper.cpp
  src/persist_sqlite.cpp
  src/ws_server.cpp
)

target_link_libraries(quant_engine PRIVATE
  Boost::system
  OpenSSL::SSL OpenSSL::Crypto
  nlohmann_json::nlohmann_json
  SQLite::SQLite3
)
