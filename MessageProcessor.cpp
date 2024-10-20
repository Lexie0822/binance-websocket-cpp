#include "MessageProcessor.h"
#include "OrderbookManager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <simdjson.h>
#include <spdlog/spdlog.h>

MessageProcessor::MessageProcessor(boost::asio::io_context& ioc, OrderbookManager& orderbook_manager)
    : ioc_(ioc), orderbook_manager_(orderbook_manager), running_(false), deduplicator_(100000, 1000)
{
    prometheus_registry = std::make_shared<prometheus::Registry>();
    
    messages_processed = &prometheus::BuildCounter()
        .Name("messages_processed_total")
        .Help("Total number of messages processed")
        .Register(*prometheus_registry);
    
    queue_size = &prometheus::BuildGauge()
        .Name("message_queue_size")
        .Help("Current size of the message queue")
        .Register(*prometheus_registry);
}

void MessageProcessor::run() {
    running_ = true;
    schedule_processing();
}

void MessageProcessor::stop() {
    running_ = false;
}

void MessageProcessor::add_message(bool is_websocket, std::string&& message) {
    if (message_queue_.size() > MAX_QUEUE_SIZE) {
        spdlog::warn("Message queue full, dropping message");
        return; // Back-pressure: drop messages if queue is full
    }
    message_queue_.push({is_websocket, std::move(message)});
    queue_size->Add({}).Set(message_queue_.size());
}

void MessageProcessor::process_messages() {
    Message msg;
    while (message_queue_.pop(msg)) {
        if (!deduplicator_.is_duplicate(msg.content)) {
            simdjson::dom::parser parser;
            simdjson::dom::element doc = parser.parse(msg.content);
            
            try {
                if (msg.is_websocket) {
                    orderbook_manager_.OnOrderbookWs("", doc);
                } else {
                    orderbook_manager_.OnOrderbookRest("", doc);
                }
                messages_processed->Add({}).Increment();
            } catch (const std::exception& e) {
                spdlog::error("Error processing message: {}", e.what());
            }
        }
    }
    queue_size->Add({}).Set(message_queue_.size());

    if (running_) {
        schedule_processing();
    }
}

void MessageProcessor::schedule_processing() {
    ioc_.post([this]() { process_messages(); });
}
