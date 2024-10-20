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
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/functional/hash.hpp>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace http = beast::http;
namespace ssl = boost::asio::ssl;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

WebSocketHandler::WebSocketHandler(net::io_context& ioc, const std::string& symbol, MessageProcessor& messageProcessor)
    : io_context_(ioc), symbol_(symbol), message_processor_(messageProcessor), reconnect_timer_(ioc), is_connected_(false) {
    client_.clear_access_channels(websocketpp::log::alevel::all);
    client_.set_access_channels(websocketpp::log::alevel::connect);
    client_.set_access_channels(websocketpp::log::alevel::disconnect);
    client_.set_access_channels(websocketpp::log::alevel::app);

    client_.init_asio(&io_context_);

    client_.set_message_handler(std::bind(&WebSocketHandler::on_message, this, std::placeholders::_1, std::placeholders::_2));
    client_.set_open_handler(std::bind(&WebSocketHandler::on_connect, this, std::placeholders::_1));
    client_.set_close_handler(std::bind(&WebSocketHandler::handle_disconnect, this));
    client_.set_fail_handler(std::bind(&WebSocketHandler::on_fail, this, std::placeholders::_1));
}

void WebSocketHandler::connect() {
    websocketpp::lib::error_code ec;
    auto conn = client_.get_connection("wss://stream.binance.com:9443/ws/" + symbol_ + "@depth", ec);
    if (ec) {
        std::cout << "Could not create connection: " << ec.message() << std::endl;
        return;
    }

    client_.connect(conn);
}

void WebSocketHandler::stop() {
    is_connected_ = false;
    websocketpp::lib::error_code ec;
    client_.close(connection_, websocketpp::close::status::normal, "Stopping", ec);
    if (ec) {
        std::cout << "Error closing connection: " << ec.message() << std::endl;
    }
}

bool WebSocketHandler::is_connected() const {
    return is_connected_;
}

void WebSocketHandler::set_cpu_affinity(int cpu_id) {
    cpu_id_ = cpu_id;
#ifdef __linux__
    if (cpu_id_ >= 0) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_id_, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    }
#else
    // On non-Linux systems, print a warning or implement an alternative
    std::cerr << "CPU affinity setting is not supported on this platform" << std::endl;
#endif
}

void WebSocketHandler::on_message(websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg) {
    (void)hdl;  // Suppress unused parameter warning
    std::string payload = msg->get_payload();
    message_processor_.add_message(true, std::move(payload));
}

void WebSocketHandler::handle_disconnect() {
    is_connected_ = false;
    std::cout << "WebSocket disconnected. Attempting to reconnect..." << std::endl;
    
    static int retry_count = 0;
    int backoff_time = std::min(1000 * (1 << retry_count), 30000);  // Max 30 seconds
    
    reconnect_timer_.expires_after(std::chrono::milliseconds(backoff_time));
    reconnect_timer_.async_wait([this](const boost::system::error_code& ec) {
        if (!ec) {
            connect();
            retry_count++;
        } else {
            std::cerr << "Error in reconnect timer: " << ec.message() << std::endl;
            // Implement further error recovery logic here
        }
    });
}

void WebSocketHandler::on_connect(websocketpp::connection_hdl hdl) {
    is_connected_ = true;
    connection_ = hdl;
    set_tcp_options(hdl);
    std::cout << "WebSocket connected for symbol: " << symbol_ << std::endl;
    
    auto con = client_.get_con_from_hdl(hdl);
    con->send("{ \"method\": \"SUBSCRIBE\", \"params\": [\"" + symbol_ + "@depth\"], \"id\": 1 }");
}

void WebSocketHandler::set_tcp_options(websocketpp::connection_hdl hdl) {
    auto con = client_.get_con_from_hdl(hdl);
    boost::asio::ip::tcp::socket& socket = con->get_socket();
    
    socket.set_option(boost::asio::ip::tcp::no_delay(true));
    socket.set_option(boost::asio::socket_base::receive_buffer_size(262144));
    socket.set_option(boost::asio::socket_base::send_buffer_size(262144));
    socket.set_option(boost::asio::socket_base::send_low_watermark(1024));
    socket.set_option(boost::asio::socket_base::receive_low_watermark(1024));
}

void WebSocketHandler::on_fail(websocketpp::connection_hdl hdl) {
    auto con = client_.get_con_from_hdl(hdl);
    std::cout << "WebSocket connection failed for symbol " << symbol_ << ": " 
              << con->get_ec().message() << std::endl;
    handle_disconnect();
}

void WebSocketHandler::send_message(const std::string& message) {
    if (is_connected_) {
        websocketpp::lib::error_code ec;
        client_.send(connection_, message, websocketpp::frame::opcode::text, ec);
        if (ec) {
            std::cout << "Error sending message: " << ec.message() << std::endl;
        }
    } else {
        std::cout << "Cannot send message: WebSocket is not connected" << std::endl;
    }
}

void WebSocketHandler::ping() {
    if (is_connected_) {
        websocketpp::lib::error_code ec;
        client_.ping(connection_, "ping", ec);
        if (ec) {
            std::cout << "Error sending ping: " << ec.message() << std::endl;
        }
    }
}

void WebSocketHandler::set_ping_interval(long interval_ms) {
    if (is_connected_) {
        auto con = client_.get_con_from_hdl(connection_);
        con->set_pong_timeout(interval_ms);
        // Instead of setting ping interval, we can start a timer to send pings periodically
        start_ping_timer(interval_ms);
    }
}

void WebSocketHandler::start_ping_timer(long interval_ms) {
    ping_timer_ = std::make_unique<boost::asio::steady_timer>(io_context_);
    ping_timer_->expires_after(std::chrono::milliseconds(interval_ms));
    ping_timer_->async_wait([this, interval_ms](const boost::system::error_code& ec) {
        if (!ec) {
            ping();
            start_ping_timer(interval_ms);  // Reschedule the timer
        }
    });
}
