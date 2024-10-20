find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_SOURCES
    BinanceClientTest.cpp
    MessageProcessorTest.cpp
    EventLoopTest.cpp
    WebSocketHandlerTest.cpp
    RestApiHandlerTest.cpp
    OrderbookManagerTest.cpp
)

add_executable(unit_tests ${TEST_SOURCES})

target_link_libraries(unit_tests PRIVATE
    cpp_websocket_TR_lib
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
)

add_test(NAME unit_tests COMMAND unit_tests)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}
)