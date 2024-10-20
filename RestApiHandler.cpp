#include "RestApiHandler.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <queue>
#include <unordered_map>
#include <functional>

RestApiHandler::RestApiHandler(net::io_context& ioc, ssl::context& ctx, const std::string& host, const std::string& port, const std::string& target, MessageProcessor& messageProcessor)
    : ioc_(ioc), stream_(ioc, ctx), host_(host), port_(port), target_(target), message_processor_(messageProcessor), poll_timer_(ioc)
{
    req_.version(11);
    req_.method(http::verb::get);
    req_.target(target);
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
}

void RestApiHandler::start_polling() {
    running_ = true;
    poll_timer_.expires_after(std::chrono::seconds(1));
    poll_timer_.async_wait(std::bind(&RestApiHandler::run, shared_from_this()));
}

void RestApiHandler::stop() {
    running_ = false;
    boost::system::error_code ec;
    stream_.shutdown(ec);
}

bool RestApiHandler::is_connected() const {
    return is_connected_;
}

void RestApiHandler::set_cpu_affinity(int cpu_id) {
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

void RestApiHandler::run() {
    if (!running_) return;

    if (!can_make_request()) {
        // If we can't send a request, delay and retry
        poll_timer_.expires_after(std::chrono::milliseconds(100));
        poll_timer_.async_wait(std::bind(&RestApiHandler::run, shared_from_this()));
        return;
    }

    tcp::resolver resolver(ioc_);
    resolver.async_resolve(host_, port_,
        beast::bind_front_handler(&RestApiHandler::on_resolve, shared_from_this()));
}

void RestApiHandler::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if(ec) return fail(ec, "resolve");

    beast::get_lowest_layer(stream_).async_connect(results,
        beast::bind_front_handler(&RestApiHandler::on_connect, shared_from_this()));
}

void RestApiHandler::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
    if(ec) {
        fail(ec, "connect");
        return;
    }

    // Perform the SSL handshake
    stream_.async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(
            &RestApiHandler::on_handshake,
            shared_from_this()));
}

void RestApiHandler::on_handshake(beast::error_code ec) {
    if(ec) return fail(ec, "handshake");

    http::async_write(stream_, req_,
        beast::bind_front_handler(&RestApiHandler::on_write, shared_from_this()));
}

void RestApiHandler::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec) return fail(ec, "write");

    http::async_read(stream_, buffer_, res_,
        beast::bind_front_handler(&RestApiHandler::on_read, shared_from_this()));
}

void RestApiHandler::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail(ec, "read");

    // Process the response
    std::string response_body = res_.body();
    message_processor_.add_message(false, std::move(response_body));

    // Close the connection
    stream_.async_shutdown(
        beast::bind_front_handler(
            &RestApiHandler::on_shutdown,
            shared_from_this()));
}

void RestApiHandler::on_shutdown(beast::error_code ec) {
    if(ec == net::error::eof) {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec = {};
    }
    if(ec)
        return fail(ec, "shutdown");

    // If we get here then the connection is closed gracefully

    // Schedule the next poll
    poll_timer_.expires_after(std::chrono::milliseconds(current_polling_interval_));
    poll_timer_.async_wait(std::bind(&RestApiHandler::run, shared_from_this()));
}

void RestApiHandler::close() {
    beast::error_code ec;
    stream_.shutdown(ec);
}

void RestApiHandler::fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
    is_connected_ = false;
}

bool RestApiHandler::can_make_request() {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - last_request_time_).count();
    tokens_ = std::min(max_tokens_, tokens_ + elapsed * token_rate_);
    if (tokens_ >= 1.0) {
        tokens_ -= 1.0;
        last_request_time_ = now;
        return true;
    }
    return false;
}

void RestApiHandler::decrease_polling_interval() {
    current_polling_interval_ = std::max(current_polling_interval_ / 2, min_polling_interval_);
}

void RestApiHandler::increase_polling_interval() {
    current_polling_interval_ = std::min(current_polling_interval_ * 2, max_polling_interval_);
}

int RestApiHandler::get_current_polling_interval() const {
    return current_polling_interval_;
}

int RestApiHandler::get_max_polling_interval() const {
    return max_polling_interval_;
}

void RestApiHandler::poll_orderbook() {
    while (running_) {
        try {
            // Create the request
            req_ = http::request<http::string_body>(http::verb::get, target_, 11);
            req_.set(http::field::host, host_);
            req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

            // Send the request
            http::write(stream_, req_);

            // Receive the response
            http::read(stream_, buffer_, res_);

            // Process the response
            std::string response_body = res_.body();
            message_processor_.add_message(false, std::move(response_body));

            // Wait for the polling interval
            std::this_thread::sleep_for(std::chrono::milliseconds(current_polling_interval_));
        } catch (const std::exception& e) {
            std::cerr << "Error polling orderbook: " << e.what() << std::endl;
        }
    }
}
