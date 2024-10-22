#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <unordered_map>
#include "WebSocketHandler.h"
#include "RestApiHandler.h"
#include "BinanceClient.h"
#include <iostream>
#include <functional>
#include <pthread.h>
#include <sys/types.h>
#include <sched.h>
#include "EventLoop.h"
#include <random>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#ifdef __linux__
#include <numa.h>
#endif

// CircuitBreaker implementation
CircuitBreaker::CircuitBreaker(int failure_threshold, std::chrono::seconds reset_timeout)
    : failure_threshold_(failure_threshold), reset_timeout_(reset_timeout) {}

bool CircuitBreaker::allow_request() {
    if (state_ == State::Closed) {
        return true;
    } else if (state_ == State::Open) {
        auto now = std::chrono::steady_clock::now();
        if (now - last_failure_time_ > reset_timeout_) {
            state_ = State::HalfOpen;
            return true;
        }
        return false;
    } else { // HalfOpen
        return true;
    }
}

void CircuitBreaker::record_success() {
    if (state_ == State::HalfOpen) {
        state_ = State::Closed;
        failure_count_ = 0;
    }
}

void CircuitBreaker::record_failure() {
    last_failure_time_ = std::chrono::steady_clock::now();
    if (state_ == State::Closed) {
        if (++failure_count_ >= failure_threshold_) {
            state_ = State::Open;
        }
    } else if (state_ == State::HalfOpen) {
        state_ = State::Open;
    }
}

// BinanceClient implementation
BinanceClient::BinanceClient(size_t thread_count) 
    : event_loop_pool_(std::make_unique<EventLoopPool>(thread_count)),
      market_data_loop_(std::make_unique<EventLoop>()),
      orderbook_manager_(std::make_unique<OrderbookManager>()),
      message_processor_(std::make_unique<MessageProcessor>(market_data_loop_->get_io_context(), *orderbook_manager_)),
      running_(false),
      circuit_breaker_(5, std::chrono::seconds(30)),
      work_(std::make_unique<boost::asio::io_context::work>(io_context_)),
      ws_handler_pool_(),
      rest_handler_pool_()
{
    spdlog::info("BinanceClient initialized with {} threads", thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        worker_threads_.emplace_back([this] { io_context_.run(); });
    }
}

BinanceClient::~BinanceClient() {
    stop();
}

void BinanceClient::start(const std::vector<std::string>& symbols) {
    spdlog::info("Starting BinanceClient with {} symbols", symbols.size());
    running_ = true;
    balance_symbols(symbols);
    event_loop_pool_->run();
    market_data_loop_->run();

    for (const auto& symbol : symbols) {
        create_handlers_for_symbol(symbol);
    }

    market_data_loop_->post([this]() { message_processor_->run(); });
}

void BinanceClient::stop() {
    running_ = false;
    for (auto& [symbol, handler] : ws_handlers_) {
        handler->stop();
    }
    for (auto& [symbol, handler] : rest_handlers_) {
        handler->stop();
    }
    message_processor_->stop();
    event_loop_pool_->stop();
    market_data_loop_->stop();

    work_.reset();
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

std::string BinanceClient::get_orderbook_snapshot(const std::string& symbol, int depth) const {
    return orderbook_manager_->getOrderbookSnapshot(symbol, depth);
}

void BinanceClient::add_symbol(const std::string& symbol) {
    symbol_manager_.add_symbol(symbol);
    std::unique_lock<std::shared_mutex> lock(symbols_mutex_);
    if (std::find(active_symbols_.begin(), active_symbols_.end(), symbol) == active_symbols_.end()) {
        active_symbols_.push_back(symbol);
        create_handlers_for_symbol(symbol);
    }
}

void BinanceClient::remove_symbol(const std::string& symbol) {
    symbol_manager_.remove_symbol(symbol);
    std::unique_lock<std::shared_mutex> lock(symbols_mutex_);
    
    // Use std::remove and lambda functions to remove elements
    auto new_end = std::remove_if(active_symbols_.begin(), active_symbols_.end(),
        [&symbol](const std::string& s) { return s == symbol; });
    
    // erase cannot be called directly here because tbb::concurrent_vector does not have an erase method.
    // simulate a delete operation by setting the element to be deleted to an empty string
    for (auto it = new_end; it != active_symbols_.end(); ++it) {
        *it = "";
    }
    
    remove_handlers_for_symbol(symbol);
}

std::vector<std::string> BinanceClient::get_active_symbols() const {
    std::shared_lock<std::shared_mutex> lock(symbols_mutex_);
    std::vector<std::string> result;
    for (const auto& symbol : active_symbols_) {
        if (!symbol.empty()) {
            result.push_back(symbol);
        }
    }
    return result;
}

void BinanceClient::monitor_system_health() {
    // Implement system health monitoring logic
    spdlog::info("Monitoring system health...");
    // Implement system health monitoring logic
}

void BinanceClient::reconnect_failed_connections() {
    for (auto& [symbol, handler] : ws_handlers_) {
        if (!handler->is_connected()) {
            handler->connect();
        }
    }
    for (auto& [symbol, handler] : rest_handlers_) {
        if (!handler->is_connected()) {
            handler->start_polling();
        }
    }
}

TradingStats BinanceClient::get_trading_stats(const std::string& symbol) const {
    // Implement actual trading statistics logic here
    // Now returning random data as an example
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> volume_dis(1000, 10000);
    static std::uniform_real_distribution<> price_change_dis(-5, 5);

    (void)symbol;  // Suppress unused parameter warning
    return TradingStats{volume_dis(gen), price_change_dis(gen)};
}

void BinanceClient::clean_old_data() {
    std::cout << "Cleaning old data..." << std::endl;
    // Implement old data cleaning logic
}

void BinanceClient::update_configuration() {
    std::cout << "Updating configuration..." << std::endl;
    // Implement configuration update logic
}

ResourceUsage BinanceClient::get_resource_usage() const {
    // Implement actual resource usage statistics logic here
    // Now returning random data as an example
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> cpu_dis(0, 100);
    static std::uniform_real_distribution<> mem_dis(100, 1000);

    return ResourceUsage{cpu_dis(gen), mem_dis(gen)};
}

bool BinanceClient::need_load_balancing() const {
    // Implement load balancing check logic
    return false;
}

void BinanceClient::perform_load_balancing() {
    std::cout << "Performing load balancing..." << std::endl;
    // Implement load balancing logic
}

bool BinanceClient::has_new_market_data() const {
    // Implement new market data check logic
    return false;
}

void BinanceClient::process_new_market_data() {
    std::cout << "Processing new market data..." << std::endl;
    // Implement new market data processing logic
}

void BinanceClient::generate_report() const {
    std::cout << "Generating report..." << std::endl;
    // Implement report generation logic
}

int BinanceClient::check_network_latency() const {
    // Implement network latency check logic
    return 50; // Return an example value in milliseconds
}

void BinanceClient::update_trading_strategy() {
    std::cout << "Updating trading strategy..." << std::endl;
    // Implement trading strategy update logic
}

void BinanceClient::perform_risk_management_check() {
    std::cout << "Performing risk management check..." << std::endl;
    // Implement risk management check logic
}

void BinanceClient::update_market_depth() {
    std::cout << "Updating market depth..." << std::endl;
    // Implement market depth update logic
}

bool BinanceClient::has_new_trading_signals() const {
    // Implement new trading signals check logic
    return false;
}

void BinanceClient::process_trading_signals() {
    std::cout << "Processing trading signals..." << std::endl;
    // Implement trading signals processing logic
}

void BinanceClient::process_market_data() {
    // Implement market data processing logic
}

void BinanceClient::check_load_balancing() {
    if (need_load_balancing()) {
        perform_load_balancing();
    }
}

size_t BinanceClient::get_shard(const std::string& symbol) const {
    std::hash<std::string> hasher;
    return hasher(symbol) % event_loop_pool_->size();
}

void BinanceClient::balance_symbols(const std::vector<std::string>& symbols) {
    size_t num_groups = event_loop_pool_->size();
    symbol_groups_.resize(num_groups);

    for (const auto& symbol : symbols) {
        size_t group = get_shard(symbol);
        symbol_groups_[group].push_back(symbol);
    }
}

void BinanceClient::create_handlers_for_symbol(const std::string& symbol) {
    EventLoop& event_loop = event_loop_pool_->get_next_event_loop();

    boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12_client);
    ctx.set_default_verify_paths();

    auto ws_handler = std::shared_ptr<WebSocketHandler>(
        ws_handler_pool_.allocate(),
        [this](WebSocketHandler* p) { 
            p->~WebSocketHandler();
            ws_handler_pool_.deallocate(p); 
        }
    );
    new (ws_handler.get()) WebSocketHandler(event_loop.get_io_context(), symbol, *message_processor_);

    auto rest_handler = std::shared_ptr<RestApiHandler>(
        rest_handler_pool_.allocate(),
        [this](RestApiHandler* p) { 
            p->~RestApiHandler();
            rest_handler_pool_.deallocate(p); 
        }
    );
    new (rest_handler.get()) RestApiHandler(event_loop.get_io_context(), ctx, "api.binance.com", "443", "/api/v3/depth?symbol=" + symbol, *message_processor_);

    ws_handlers_.insert(std::make_pair(symbol, ws_handler));
    rest_handlers_.insert(std::make_pair(symbol, rest_handler));

    ws_handler->connect();
    rest_handler->start_polling();
}

void BinanceClient::remove_handlers_for_symbol(const std::string& symbol) {
    ws_handlers_.erase(symbol);
    rest_handlers_.erase(symbol);
}

void BinanceClient::log_error(const std::string& error_message) {
    spdlog::error("Error: {}", error_message);
}

void BinanceClient::set_cpu_affinity(int cpu_id) {
#ifdef __linux__
    if (cpu_id >= 0) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_id, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    }
#else
    spdlog::warn("CPU affinity setting is not supported on this platform (cpu_id: {})", cpu_id);
#endif
}

// SymbolManager implementation
void BinanceClient::SymbolManager::add_symbol(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t shard = std::hash<std::string>{}(symbol) % shards_.size();
    shards_[shard].insert(symbol);
}

void BinanceClient::SymbolManager::remove_symbol(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t shard = std::hash<std::string>{}(symbol) % shards_.size();
    shards_[shard].erase(symbol);
}

std::vector<std::string> BinanceClient::SymbolManager::get_symbols_for_shard(size_t shard_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::vector<std::string>(shards_[shard_id].begin(), shards_[shard_id].end());
}
