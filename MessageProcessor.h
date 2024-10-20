#pragma once

#include <string>
#include <atomic>
#include "LockFreeQueue.h"
#include "Deduplicator.h"
#include <prometheus/registry.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <boost/asio.hpp>

class OrderbookManager;

class MessageProcessor {
public:
    MessageProcessor(boost::asio::io_context& ioc, OrderbookManager& orderbook_manager);
    void run();
    void stop();
    void add_message(bool is_websocket, std::string&& message);

private:
    static constexpr size_t MAX_QUEUE_SIZE = 1000000; // 1 million messages

    struct Message {
        bool is_websocket;
        std::string content;
    };

    boost::asio::io_context& ioc_;
    OrderbookManager& orderbook_manager_;
    LockFreeQueue<Message> message_queue_;
    std::atomic<bool> running_;
    Deduplicator deduplicator_;

    std::shared_ptr<prometheus::Registry> prometheus_registry;
    prometheus::Family<prometheus::Counter>* messages_processed;
    prometheus::Family<prometheus::Gauge>* queue_size;

    void process_messages();
    void schedule_processing();
};
