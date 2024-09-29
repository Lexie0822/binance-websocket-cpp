#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <string>
#include "MessageProcessor.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class RestApiHandler : public std::enable_shared_from_this<RestApiHandler> {
private:
    net::io_context& ioc;
    ssl::context& ctx;
    beast::ssl_stream<beast::tcp_stream> stream;
    beast::flat_buffer buffer;
    http::request<http::string_body> req;
    http::response<http::dynamic_body> res;
    tcp::resolver resolver;
    std::string host;
    std::string symbol;
    MessageProcessor& messageProcessor;

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void on_handshake(beast::error_code ec);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_shutdown(beast::error_code ec);
    void fail(beast::error_code ec, char const* what);

public:
    RestApiHandler(net::io_context& ioc, ssl::context& ctx, const std::string& symbol, MessageProcessor& messageProcessor);
    void run(const std::string& host, const std::string& port, const std::string& target);
    
    std::string getSymbol() const { return symbol; }
};