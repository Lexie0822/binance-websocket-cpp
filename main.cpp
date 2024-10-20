#include "BinanceClient.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <fstream>
#include <csignal>
#include <x86intrin.h>

std::atomic<bool> running(true);

void signal_handler(int signal) {
    (void)signal;  // Suppress unused parameter warning
    running = false;
}

void process_user_input(BinanceClient& client) {
    std::string input;
    while (running) {
        std::getline(std::cin, input);
        if (input == "exit") {
            running = false;
        } else if (input == "status") {
            std::cout << "System is running. Enter 'exit' to stop." << std::endl;
        } else if (input.substr(0, 4) == "add ") {
            std::string symbol = input.substr(4);
            client.add_symbol(symbol);
            std::cout << "Added symbol: " << symbol << std::endl;
        } else if (input.substr(0, 7) == "remove ") {
            std::string symbol = input.substr(7);
            client.remove_symbol(symbol);
            std::cout << "Removed symbol: " << symbol << std::endl;
        } else if (input == "list") {
            auto symbols = client.get_active_symbols();
            std::cout << "Active symbols: ";
            for (const auto& symbol : symbols) {
                std::cout << symbol << " ";
            }
            std::cout << std::endl;
        } else {
            std::cout << "Unknown command. Available commands: exit, status, add <symbol>, remove <symbol>, list" << std::endl;
        }
    }
}

void log_to_file(const std::string& message) {
    static std::mutex log_mutex;
    std::lock_guard<std::mutex> lock(log_mutex);
    std::ofstream log_file("binance_client.log", std::ios_base::app);
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    log_file << std::put_time(std::localtime(&now_c), "%F %T") << " - " << message << std::endl;
}

void market_data_thread_func(BinanceClient& client) {
    while (running) {
        client.process_market_data();
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

void system_monitor_thread_func(BinanceClient& client) {
    while (running) {
        client.monitor_system_health();
        client.reconnect_failed_connections();
        client.check_load_balancing();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "Initializing BinanceClient..." << std::endl;
    BinanceClient client(std::thread::hardware_concurrency());
    
    std::vector<std::string> symbols = {"btcusdt", "ethusdt", "bnbusdt", "adausdt"};
    std::cout << "Starting BinanceClient..." << std::endl;
    client.start(symbols);

    std::cout << "BinanceClient started. Enter commands (type 'exit' to stop):" << std::endl;
    
    std::thread input_thread(process_user_input, std::ref(client));
    std::thread market_data_thread(market_data_thread_func, std::ref(client));
    std::thread system_monitor_thread(system_monitor_thread_func, std::ref(client));

    while (running) {
        uint64_t start = __rdtsc();
        client.update_trading_strategy();
        client.perform_risk_management_check();
        client.update_market_depth();

        if (client.has_new_trading_signals()) {
            client.process_trading_signals();
        }

        uint64_t end = __rdtsc();
        uint64_t cycles = end - start;
        (void)cycles;  // Suppress unused variable warning
        // TODO: Use 'cycles' for performance monitoring

        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    std::cout << "Stopping BinanceClient..." << std::endl;
    client.stop();

    if (input_thread.joinable()) input_thread.join();
    if (market_data_thread.joinable()) market_data_thread.join();
    if (system_monitor_thread.joinable()) system_monitor_thread.join();

    std::cout << "BinanceClient stopped." << std::endl;
    return 0;
}
