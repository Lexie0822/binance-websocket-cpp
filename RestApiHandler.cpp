#include "RestApiHandler.h"
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

RestApiHandler::RestApiHandler(net::io_context& ioc, ssl::context& ctx, const std::string& symbol, MessageProcessor& messageProcessor)
    : ioc(ioc), ctx(ctx), stream(ioc, ctx), resolver(ioc), symbol(symbol), messageProcessor(messageProcessor) {}

void RestApiHandler::run(const std::string& host, const std::string& port, const std::string& target) {
    this->host = host;
    
    // Set up an HTTP GET request message
    req = http::request<http::string_body>(http::verb::get, target, 11);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Look up the domain name
    resolver.async_resolve(
        host,
        port,
        beast::bind_front_handler(
            &RestApiHandler::on_resolve,
            shared_from_this()));
}

void RestApiHandler::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if(ec)
        return fail(ec, "resolve");

    // Set a timeout on the operation
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(stream).async_connect(
        results,
        beast::bind_front_handler(
            &RestApiHandler::on_connect,
            shared_from_this()));
}

void RestApiHandler::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
    if(ec)
        return fail(ec, "connect");

    // Perform the SSL handshake
    stream.async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(
            &RestApiHandler::on_handshake,
            shared_from_this()));
}

void RestApiHandler::on_handshake(beast::error_code ec) {
    if(ec)
        return fail(ec, "handshake");

    // Set a timeout on the operation
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    http::async_write(stream, req,
        beast::bind_front_handler(
            &RestApiHandler::on_write,
            shared_from_this()));
}

void RestApiHandler::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail(ec, "write");
    
    // Receive the HTTP response
    http::async_read(stream, buffer, res,
        beast::bind_front_handler(
            &RestApiHandler::on_read,
            shared_from_this()));
}

void RestApiHandler::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail(ec, "read");

    // Write the message to standard out
    std::string msg = beast::buffers_to_string(res.body().data());
    messageProcessor.addMessage(false, msg);

    // Set a timeout on the operation
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Gracefully close the stream
    stream.async_shutdown(
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
}

void RestApiHandler::fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}
