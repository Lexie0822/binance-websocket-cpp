# Binance Client

## Project Structure

- `main.cpp`: Program entry point
- `BinanceClient.cpp` / `BinanceClient.h`: Main client class, manages WebSocket and REST API connections
- `WebSocketHandler.cpp` / `WebSocketHandler.h`: Handles WebSocket connections
- `RestApiHandler.cpp` / `RestApiHandler.h`: Handles REST API connections
- `MessageProcessor.cpp` / `MessageProcessor.h`: Processes messages from WebSocket and REST API
- `OrderbookManager.cpp` / `OrderbookManager.h`: Manages orderbook data
- `ThreadPool.cpp` / `ThreadPool.h`: Thread pool implementation

## Assignment 1b Answers

### 1. CPU Context Switching Diagram

[Insert your CPU context switching diagram here]

Explanation:
- The x-axis represents different tasks (WebSocket handling, REST API handling, message processing, etc.)
- The y-axis represents time
- The diagram shows how multiple CPUs switch context between different tasks
- WebSocket and REST API connections can run in parallel without blocking the main loop
- Threads in the thread pool can flexibly handle different types of tasks

### 2. Potential System Bottlenecks and Monitoring Metrics

1. Network latency: Connections to Binance servers may be affected by network conditions.
   Monitor: Round-trip time (RTT), connection failure rate

2. Message queue backlog: If message processing can't keep up with incoming messages, queue backlog may occur.
   Monitor: Queue length, message processing time

3. CPU utilization: High load may cause processing delays.
   Monitor: CPU usage per core

4. Memory usage: Large amounts of data may cause memory pressure.
   Monitor: Memory usage, memory allocation/deallocation frequency

5. Thread pool saturation: If task count exceeds thread pool capacity, it may cause processing delays.
   Monitor: Thread pool utilization, waiting queue length

6. Orderbook update frequency: High-frequency updates may cause processing pressure.
   Monitor: Orderbook updates per second

### 3. Methods to Improve System Latency

1. Use more efficient data structures: For example, use optimized tree structures or hash tables for the orderbook.

2. Implement batch processing: Combine multiple small updates into one large update to reduce processing frequency.

3. Use memory-mapped files: For large amounts of data, this can reduce memory copies.

4. Optimize network configuration: Adjust TCP parameters, use faster DNS resolution, etc.

5. Use faster JSON parsing libraries: Such as rapidjson or simdjson.

6. Implement predictive caching: Pre-load data that may be needed soon.

7. Use FPGA or GPU acceleration: For specific compute-intensive tasks.

8. Optimize thread pool: Dynamically adjust thread count based on actual load.

9. Use lock-free data structures: Reduce thread synchronization overhead where possible.

10. Implement finer-grained task division: Allow for more flexible parallel processing.
