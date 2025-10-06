cmake_minimum_required(VERSION 3.23)
project(quant_engine LANGUAGES CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost CONFIG REQUIRED COMPONENTS system)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(opentelemetry-cpp CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

include(CTest)

add_subdirectory(src)

if(BUILD_TESTING)
  find_package(GTest CONFIG REQUIRED)
  add_subdirectory(tests)
endif()
