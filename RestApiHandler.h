#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <string>
#include <memory>
#include <atomic>
#include "MessageProcessor.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class RestApiHandler : public std::enable_shared_from_this<RestApiHandler> {
public:
    RestApiHandler(net::io_context& ioc, ssl::context& ctx, const std::string& host, const std::string& port, const std::string& target, MessageProcessor& messageProcessor);
    void start_polling();
    void stop();
    bool is_connected() const;
    void set_cpu_affinity(int cpu_id);

    // Add these methods
    void decrease_polling_interval();
    void increase_polling_interval();
    int get_current_polling_interval() const;
    int get_max_polling_interval() const;

private:
    net::io_context& ioc_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    std::string host_;
    std::string port_;
    std::string target_;
    MessageProcessor& message_processor_;
    net::steady_timer poll_timer_;
    std::atomic<bool> is_connected_{false};
    std::atomic<bool> running_{true};
    int cpu_id_ = -1;
    std::chrono::steady_clock::time_point last_request_time_;
    double tokens_ = 1.0;
    const double max_tokens_ = 1.0;
    const double token_rate_ = 1.0; // Tokens generated per second

    // Add these members
    int current_polling_interval_ = 1000; // Start with 1 second
    const int min_polling_interval_ = 100; // Minimum 100ms
    const int max_polling_interval_ = 5000; // Maximum 5 seconds

    void run();
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);
    void on_handshake(beast::error_code ec);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_shutdown(beast::error_code ec);
    void close();
    void fail(beast::error_code ec, char const* what);
    bool can_make_request();
    void poll_orderbook();
};
