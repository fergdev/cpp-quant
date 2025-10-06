add_executable(quant_engine_test
  test_risk.cpp
)

target_link_libraries(quant_engine_test
  PRIVATE
    quant_engine_lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(quant_engine_test)
