#pragma once

#include "EventLoop.h"
#include "WebSocketHandler.h"
#include "RestApiHandler.h"
#include "MessageProcessor.h"
#include "OrderbookManager.h"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <boost/asio.hpp>
#ifdef __linux__
#include <sys/cpu_set.h>
#endif
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <chrono>
#include "MemoryPool.h"

struct TradingStats {
    double volume;
    double price_change;
};

struct ResourceUsage {
    double cpu_usage;
    double memory_usage;
};

class CircuitBreaker {
public:
    CircuitBreaker(int failure_threshold, std::chrono::seconds reset_timeout);
    bool allow_request();
    void record_success();
    void record_failure();

private:
    enum class State { Closed, Open, HalfOpen };
    State state_ = State::Closed;
    int failure_count_ = 0;
    int failure_threshold_;
    std::chrono::seconds reset_timeout_;
    std::chrono::steady_clock::time_point last_failure_time_;
};

class BinanceClient {
public:
    BinanceClient(size_t thread_count = std::thread::hardware_concurrency());
    ~BinanceClient();

    void start(const std::vector<std::string>& symbols);
    void stop();
    std::string get_orderbook_snapshot(const std::string& symbol, int depth) const;

    void add_symbol(const std::string& symbol);
    void remove_symbol(const std::string& symbol);
    std::vector<std::string> get_active_symbols() const;

    void monitor_system_health();
    void reconnect_failed_connections();
    TradingStats get_trading_stats(const std::string& symbol) const;
    void clean_old_data();
    void update_configuration();
    ResourceUsage get_resource_usage() const;
    bool need_load_balancing() const;
    void perform_load_balancing();
    bool has_new_market_data() const;
    void process_new_market_data();
    void generate_report() const;
    int check_network_latency() const;
    void update_trading_strategy();
    void perform_risk_management_check();
    void update_market_depth();
    bool has_new_trading_signals() const;
    void process_trading_signals();
    void process_market_data();
    void check_load_balancing();

    void set_cpu_affinity(int cpu_id);

private:
    std::unique_ptr<EventLoopPool> event_loop_pool_;
    std::unique_ptr<EventLoop> market_data_loop_;
    std::unique_ptr<OrderbookManager> orderbook_manager_;
    std::unique_ptr<MessageProcessor> message_processor_;
    tbb::concurrent_hash_map<std::string, std::shared_ptr<WebSocketHandler>> ws_handlers_;
    tbb::concurrent_hash_map<std::string, std::shared_ptr<RestApiHandler>> rest_handlers_;
    std::vector<std::vector<std::string>> symbol_groups_;

    mutable std::shared_mutex symbols_mutex_;
    tbb::concurrent_vector<std::string> active_symbols_;
    std::atomic<bool> running_{false};

    size_t symbol_hash(const std::string& symbol) const;
    void balance_symbols(const std::vector<std::string>& symbols);
    void create_handlers_for_symbol(const std::string& symbol);
    void remove_handlers_for_symbol(const std::string& symbol);

    void log_error(const std::string& error_message);

    std::atomic<size_t> next_event_loop_{0};
    CircuitBreaker circuit_breaker_;
    boost::asio::io_context io_context_;
    std::vector<std::thread> worker_threads_;
    std::unique_ptr<boost::asio::io_context::work> work_;
    MemoryPool<WebSocketHandler> ws_handler_pool_;
    MemoryPool<RestApiHandler> rest_handler_pool_;

    class SymbolManager {
    public:
        void add_symbol(const std::string& symbol);
        void remove_symbol(const std::string& symbol);
        std::vector<std::string> get_symbols_for_shard(size_t shard_id);
    private:
        std::vector<std::unordered_set<std::string>> shards_{std::thread::hardware_concurrency()};
        std::mutex mutex_;
    };
    SymbolManager symbol_manager_;

    size_t get_shard(const std::string& symbol) const;
};
