#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <memory>
#include <functional>
#include "MessageProcessor.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace ssl = boost::asio::ssl;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class WebSocketHandler : public std::enable_shared_from_this<WebSocketHandler> {
public:
    WebSocketHandler(net::io_context& ioc, ssl::context& ctx, const std::string& symbol, MessageProcessor& messageProcessor);
    void run(const std::string& host, const std::string& port);

private:
    websocket::stream<beast::ssl_stream<tcp::socket>> ws;
    tcp::resolver resolver;
    beast::flat_buffer buffer;
    std::string host;
    std::string symbol;
    MessageProcessor& messageProcessor;

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    void on_connect(boost::system::error_code ec, tcp::endpoint endpoint);
    void on_ssl_handshake(beast::error_code ec);
    void on_handshake(beast::error_code ec);
    void do_read();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void fail(beast::error_code ec, char const* what);
};