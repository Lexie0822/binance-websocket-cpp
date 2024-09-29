cmake_minimum_required(VERSION 3.10)
project(cpp_websocket_TR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.86.0 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

# 添加可执行文件
add_executable(cpp_websocket_TR 
    main.cpp
    WebSocketHandler.cpp
    RestApiHandler.cpp
    BinanceClient.cpp
    MessageProcessor.cpp
    OrderbookManager.cpp
    ThreadPool.cpp
)

target_include_directories(cpp_websocket_TR PRIVATE 
    ${Boost_INCLUDE_DIRS} 
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(cpp_websocket_TR PRIVATE 
    Boost::system 
    Boost::thread
    OpenSSL::SSL 
    OpenSSL::Crypto 
    pthread
)