#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include "MessageProcessor.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace ssl = boost::asio::ssl;
namespace net = boost::asio;

class WebSocketHandler : public std::enable_shared_from_this<WebSocketHandler> {
public:
    WebSocketHandler(net::io_context& ioc, const std::string& symbol, MessageProcessor& messageProcessor);
    void connect();
    void stop();
    bool is_connected() const;
    void set_cpu_affinity(int cpu_id);
    void send_message(const std::string& message);
    void ping();
    void set_ping_interval(long interval_ms);

private:
    net::io_context& io_context_;
    std::string symbol_;
    MessageProcessor& message_processor_;
    websocketpp::client<websocketpp::config::asio_client> client_;
    websocketpp::connection_hdl connection_;
    net::steady_timer reconnect_timer_;
    std::atomic<bool> is_connected_{false};
    int cpu_id_ = -1;
    std::unique_ptr<boost::asio::steady_timer> ping_timer_;

    void on_message(websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg);
    void handle_disconnect();
    void on_connect(websocketpp::connection_hdl hdl);
    void set_tcp_options(websocketpp::connection_hdl hdl);
    void on_fail(websocketpp::connection_hdl hdl);
    void start_ping_timer(long interval_ms);
};
