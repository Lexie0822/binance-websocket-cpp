#include "WebSocketHandler.h"
#include <iostream>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/detail/socket_option.hpp>
#include <boost/asio/socket_base.hpp>
#include <chrono>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace http = beast::http;
namespace ssl = boost::asio::ssl;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

WebSocketHandler::WebSocketHandler(net::io_context& ioc, ssl::context& ctx, const std::string& symbol, MessageProcessor& messageProcessor)
    : ws(net::make_strand(ioc), ctx),
      resolver(net::make_strand(ioc)), 
      symbol(symbol), 
      messageProcessor(messageProcessor) {}

void WebSocketHandler::run(const std::string& host, const std::string& port) {
    resolver.async_resolve(host, port,
        [this, self = shared_from_this()](boost::system::error_code ec, tcp::resolver::results_type results) {
            if (!ec) {
                boost::asio::async_connect(
                    ws.next_layer().next_layer(),
                    results.begin(),
                    results.end(),
                    [this, self](boost::system::error_code ec, tcp::resolver::iterator) {
                        if (!ec) {
                            this->on_connect(ec, ws.next_layer().next_layer().remote_endpoint());
                        } else {
                            std::cerr << "Connect failed: " << ec.message() << std::endl;
                        }
                    }
                );
            } else {
                std::cerr << "Resolve failed: " << ec.message() << std::endl;
            }
        }
    );
}

void WebSocketHandler::on_connect(boost::system::error_code ec, tcp::endpoint endpoint) {
    if(ec) {
        std::cerr << "WebSocket connection failed: " << ec.message() << std::endl;
        return;
    }
    std::cout << "WebSocket connected successfully for symbol: " << symbol << std::endl;

    // 更新连接字符串
    std::string host = endpoint.address().to_string();
    host += ':' + std::to_string(endpoint.port());

    // ... 其他连接后的逻辑 ...
}

void WebSocketHandler::on_ssl_handshake(beast::error_code ec) {
    if(ec)
        return fail(ec, "ssl_handshake");

    ws.async_handshake(host, "/",
        beast::bind_front_handler(&WebSocketHandler::on_handshake, shared_from_this()));
}

void WebSocketHandler::on_handshake(beast::error_code ec) {
    if(ec)
        return fail(ec, "handshake");

    do_read();
}

void WebSocketHandler::do_read() {
    ws.async_read(
        buffer,
        beast::bind_front_handler(&WebSocketHandler::on_read, shared_from_this()));
}

void WebSocketHandler::on_read(boost::system::error_code ec, std::size_t bytes_transferred) {
    if(ec) {
        std::cerr << "WebSocket read error: " << ec.message() << std::endl;
        return;
    }
    std::cout << "Received " << bytes_transferred << " bytes from WebSocket for symbol: " << symbol << std::endl;
    
    // ... 处理接收到的数据 ...
}

void WebSocketHandler::fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}