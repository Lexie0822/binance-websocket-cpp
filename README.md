![diagram](https://github.com/user-attachments/assets/59a04622-b443-4a9b-86aa-a936583f61b1)# Binance Client

## Project Structure

- `main.cpp`: Program entry point
- `BinanceClient.cpp` / `BinanceClient.h`: Main client class, manages WebSocket and REST API connections
- `WebSocketHandler.cpp` / `WebSocketHandler.h`: Handles WebSocket connections
- `RestApiHandler.cpp` / `RestApiHandler.h`: Handles REST API connections
- `MessageProcessor.cpp` / `MessageProcessor.h`: Processes messages from WebSocket and REST API
- `OrderbookManager.cpp` / `OrderbookManager.h`: Manages orderbook data
- `ThreadPool.cpp` / `ThreadPool.h`: Thread pool implementation
- `EventLoop.cpp` / `EventLoop.h`: Manages event-driven programming
- `MemoryPool.cpp` / `MemoryPool.h`: Implements custom memory allocation
- `tests/`: Unit tests directory
  - `BinanceClientTests.cpp`
  - `WebSocketHandlerTests.cpp`
  - `RestApiHandlerTests.cpp`
  - `MessageProcessorTests.cpp`
  - `OrderbookManagerTests.cpp`
  - `ThreadPoolTests.cpp`
  - `EventLoopTests.cpp`
  - `MemoryPoolTests.cpp`

## Assignment 1b Answers

### 1. CPU Context Switching Diagram

I utilize a multithreaded architecture to handle both WebSocket connections and REST API polling asynchronously. This design ensures that neither the WebSocket nor REST connectivity blocks the main loop, allowing for efficient and concurrent data processing.

Threading Mechanism:

Main Thread: Initializes the application and manages the overall workflow.
WebSocket Threads: Dedicated threads that handle WebSocket connections for different symbols.
REST Polling Threads: Threads that periodically poll the REST API without blocking.
Message Processing Thread: A thread pool that processes incoming messages, performs deduplication, and triggers the appropriate callbacks (OnOrderbookWs, OnOrderbookRest).

Explanation:

Main Thread: Starts the application, initializes resources, and spawns other threads. It doesn't perform blocking operations, ensuring the main loop remains free.

WebSocket Threads: Each symbol (e.g., BTCUSDT, ETHUSDT) has a dedicated thread handling its WebSocket connection. These threads use asynchronous I/O to read data without blocking.

REST Threads: Similar to WebSocket threads, each symbol has a thread that periodically polls the REST API. The polling interval is managed to prevent resource exhaustion.

Message Processing Thread Pool: A pool of worker threads that process messages from a thread-safe queue. They perform deduplication and invoke the appropriate callbacks.

Co-existence Without Blocking:

Asynchronous Operations: Both WebSocket and REST threads use asynchronous I/O provided by Boost.Asio. This allows threads to initiate I/O operations and continue without waiting for them to complete.

Thread-safe Queues: Incoming messages are placed into thread-safe queues, preventing contention and ensuring safe communication between threads.

Non-blocking Main Loop: The main thread doesn't perform any blocking operations. It oversees the initialization and can handle other tasks like monitoring or scaling.

Efficient Threading: By dedicating threads to specific tasks and using asynchronous operations, we minimize context switching and CPU overhead.

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



**Assignment 1b**

---

### 1. CPU Context Switching Diagram

**Overview**

In our solution, we utilize a multithreaded architecture to handle both WebSocket connections and REST API polling asynchronously. This design ensures that neither the WebSocket nor REST connectivity blocks the main loop, allowing for efficient and concurrent data processing.

**Threading Mechanism:**

- **Main Thread**: Initializes the application and manages the overall workflow.
- **WebSocket Threads**: Dedicated threads that handle WebSocket connections for different symbols.
- **REST Polling Threads**: Threads that periodically poll the REST API without blocking.
- **Message Processing Thread**: A thread pool that processes incoming messages, performs deduplication, and triggers the appropriate callbacks (`OnOrderbookWs`, `OnOrderbookRest`).

**Diagram Explanation**

Below is a simplified diagram illustrating how CPU context switching occurs among different tasks over time. The x-axis represents time, and the y-axis lists the various threads/tasks.

```
Time ↑
      |
      |                   ┌─────────────────────────────────────────┐
      |                   │          Main Thread                    │
      |                   └─────────────────────────────────────────┘
      |
      |   ┌───────────────┐
      |   │ WebSocket Thread (BTCUSDT) │───────────────────────────────▶
      |   └───────────────┘
      |
      |   ┌───────────────┐
      |   │ WebSocket Thread (ETHUSDT) │──────────────────────────────▶
      |   └───────────────┘
      |
      |   ┌───────────────┐
      |   │ REST Thread (BTCUSDT) │────────────────────────────────────▶
      |   └───────────────┘
      |
      |   ┌───────────────┐
      |   │ REST Thread (ETHUSDT) │────────────────────────────────────▶
      |   └───────────────┘
      |
      |   ┌───────────────┐
      |   │ Message Processing Thread Pool │───────────────────────────▶
      |   └───────────────┘
      |
      +──────────────────────────────────────────────────────────────────▶ Tasks
```

**Explanation:**

- **Main Thread**: Starts the application, initializes resources, and spawns other threads. It doesn't perform blocking operations, ensuring the main loop remains free.
  
- **WebSocket Threads**: Each symbol (e.g., BTCUSDT, ETHUSDT) has a dedicated thread handling its WebSocket connection. These threads use asynchronous I/O to read data without blocking.

- **REST Threads**: Similar to WebSocket threads, each symbol has a thread that periodically polls the REST API. The polling interval is managed to prevent resource exhaustion.

- **Message Processing Thread Pool**: A pool of worker threads that process messages from a thread-safe queue. They perform deduplication and invoke the appropriate callbacks.

**Co-existence Without Blocking:**

- **Asynchronous Operations**: Both WebSocket and REST threads use asynchronous I/O provided by Boost.Asio. This allows threads to initiate I/O operations and continue without waiting for them to complete.

- **Thread-safe Queues**: Incoming messages are placed into thread-safe queues, preventing contention and ensuring safe communication between threads.

- **Non-blocking Main Loop**: The main thread doesn't perform any blocking operations. It oversees the initialization and can handle other tasks like monitoring or scaling.

- **Efficient Threading**: By dedicating threads to specific tasks and using asynchronous operations, we minimize context switching and CPU overhead.

---

### 2. Potential Bottlenecks and Monitoring

**Potential Bottlenecks:**

1. **Network Latency and Bandwidth:**
   - **Description**: Delays or limited bandwidth can slow down data retrieval from Binance servers.
   - **Impact**: High latency can cause outdated information, affecting the application's responsiveness.

2. **CPU Utilization:**
   - **Description**: Excessive CPU usage due to inefficient processing or too many active threads.
   - **Impact**: Can lead to increased processing time and delayed message handling.

3. **Memory Consumption:**
   - **Description**: High memory usage due to large data structures or memory leaks.
   - **Impact**: May cause the application to crash or the system to become unresponsive.

4. **Message Queue Backlog:**
   - **Description**: If message production (from WebSocket/REST) outpaces consumption (processing), queues can grow indefinitely.
   - **Impact**: Increased memory usage and delayed processing of messages.

5. **Deduplication Overhead:**
   - **Description**: Inefficient deduplication algorithms can slow down message processing.
   - **Impact**: Higher latency in invoking callbacks and increased CPU usage.

6. **Thread Synchronization Overhead:**
   - **Description**: Excessive locking and unlocking of shared resources.
   - **Impact**: Can lead to contention, deadlocks, or increased context switching.

7. **REST API Rate Limits:**
   - **Description**: Binance imposes rate limits on REST API calls.
   - **Impact**: Exceeding rate limits can result in denied requests or bans.

**What Should Be Monitored:**

1. **CPU and Memory Usage:**
   - **Metrics**: CPU utilization per core, total memory usage, memory leaks.
   - **Tools**: `top`, `htop`, or profiling tools like Valgrind.

2. **Network Performance:**
   - **Metrics**: Latency, throughput, packet loss, error rates.
   - **Tools**: Ping tests, network monitoring tools.

3. **Message Queue Length:**
   - **Metrics**: Number of messages waiting to be processed.
   - **Action**: Implement queue size thresholds and alerts.

4. **Processing Time:**
   - **Metrics**: Time taken to process each message, including deduplication and callback execution.
   - **Tools**: Logging timestamps, profiling.

5. **Thread Pool Utilization:**
   - **Metrics**: Number of active threads, tasks waiting in the thread pool.
   - **Tools**: Custom logging, thread pool statistics.

6. **Error Rates and Exceptions:**
   - **Metrics**: Number and type of errors occurring during network operations or processing.
   - **Tools**: Exception handling logs, error monitoring systems.

7. **REST API Call Rates:**
   - **Metrics**: Number of REST API calls per minute.
   - **Action**: Ensure compliance with Binance's rate limits.

---

### 3. Improving Latency

**Possible Improvements:**

1. **Optimize Deduplication Algorithm:**
   - **Action**: Use more efficient data structures like Bloom filters or LRU caches to reduce deduplication time.
   - **Benefit**: Faster message processing, reduced CPU usage.

2. **Enhance Network Efficiency:**
   - **Action**: Implement persistent connections for REST API calls, use HTTP/2 if supported, and optimize TCP parameters.
   - **Benefit**: Reduced network latency and overhead.

3. **Use Faster JSON Parsing Libraries:**
   - **Action**: Replace `nlohmann/json` with faster alternatives like `rapidjson` or `simdjson`.
   - **Benefit**: Decreased time spent parsing JSON messages.

4. **Implement Asynchronous REST Calls:**
   - **Action**: Modify REST client to use fully asynchronous operations instead of blocking threads.
   - **Benefit**: Better resource utilization, reduced thread count.

5. **Thread Pool Optimization:**
   - **Action**: Adjust the size of the thread pool dynamically based on the workload.
   - **Benefit**: Efficient CPU usage, reduced context switching.

6. **Batch Processing of Messages:**
   - **Action**: Process messages in batches where possible to reduce function call overhead.
   - **Benefit**: Improved throughput, reduced processing time per message.

7. **Reduce Logging Overhead:**
   - **Action**: Limit logging to essential information or use asynchronous logging frameworks.
   - **Benefit**: Lower CPU usage, reduced I/O blocking.

8. **CPU Affinity and Core Binding:**
   - **Action**: Bind certain threads to specific CPU cores to improve cache performance.
   - **Benefit**: Reduced context switching, improved CPU cache utilization.

9. **Utilize Lock-free Data Structures:**
   - **Action**: Implement lock-free queues and data structures to minimize synchronization overhead.
   - **Benefit**: Faster concurrent access, reduced thread contention.

10. **Performance Profiling and Monitoring:**
    - **Action**: Continuously profile the application to identify hotspots and optimize them.
    - **Benefit**: Ongoing performance improvements, proactive bottleneck resolution.

11. **Hardware Improvements:**
    - **Action**: If permissible, upgrade to servers with better CPU performance, faster RAM, or SSDs.
    - **Benefit**: Overall improvement in application responsiveness.

12. **Algorithmic Optimizations:**
    - **Action**: Review and optimize algorithms used in message processing and orderbook management.
    - **Benefit**: Reduced computational complexity, faster execution.

---

