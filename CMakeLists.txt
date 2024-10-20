cmake_minimum_required(VERSION 3.10)
project(cpp_websocket_TR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native -mavx2 -mfma")
endif()

find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)
find_package(TBB REQUIRED)
find_package(simdjson REQUIRED)
find_package(spdlog REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(xxHash REQUIRED)
find_package(GTest REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)

set(WEBSOCKETPP_INCLUDE_DIR "/usr/local/include" CACHE PATH "Path to websocketpp headers")

add_library(cpp_websocket_TR_lib STATIC
    BinanceClient.cpp
    MessageProcessor.cpp
    OrderbookManager.cpp
    EventLoop.cpp
    WebSocketHandler.cpp
    RestApiHandler.cpp
    Deduplicator.cpp
)

target_include_directories(cpp_websocket_TR_lib PUBLIC 
    ${Boost_INCLUDE_DIRS} 
    ${OPENSSL_INCLUDE_DIR}
    ${RAPIDJSON_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
    ${SIMDJSON_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
    ${XXHASH_INCLUDE_DIRS}
    ${WEBSOCKETPP_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIRS}
    ${PROMETHEUS_CPP_INCLUDE_DIR}
)

target_link_libraries(cpp_websocket_TR_lib PUBLIC 
    Boost::system 
    Boost::thread
    OpenSSL::SSL 
    OpenSSL::Crypto 
    TBB::tbb
    pthread
    xxHash::xxhash
    spdlog::spdlog
    simdjson::simdjson
    prometheus-cpp::core
    prometheus-cpp::pull
)

add_executable(cpp_websocket_TR main.cpp)
target_link_libraries(cpp_websocket_TR PRIVATE cpp_websocket_TR_lib)

target_compile_options(cpp_websocket_TR_lib PRIVATE -Wall -Wextra -pedantic -Werror)
target_compile_options(cpp_websocket_TR PRIVATE -Wall -Wextra -pedantic -Werror)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_options(cpp_websocket_TR PRIVATE -flto)
endif()

enable_testing()
add_subdirectory(tests)