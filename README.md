# Binance WebSocket C++ Client

This project provides a C++ client for interacting with Binance's WebSocket and REST APIs, designed to facilitate high-frequency trading applications.

## Features
- **WebSocket and REST Integration**: Supports concurrent WebSocket data streaming and REST API polling for Binance.
- **Order Book Management**: Manages and processes order book data for various trading pairs.
- **Custom Thread Pool**: Efficient handling of concurrent tasks with a custom thread pool implementation.
- **Message Deduplication**: Filters duplicate messages using bloom filters for consistent data.
- **Memory Management**: Uses a custom memory pool for improved performance and resource efficiency.

## Project Structure

1. **Core Modules**:
    - **`BinanceClient.cpp` / `BinanceClient.h`**:
      - The main client class that integrates both WebSocket and REST API capabilities. It manages connection setups, data reception, and interaction with other core components.
    - **`WebSocketHandler.cpp` / `WebSocketHandler.h`**:
      - Manages the connection to Binance's WebSocket endpoint.
      - Handles WebSocket-related events such as connecting, disconnecting, and message processing.
      - Implements reconnection strategies to ensure a persistent data stream.
    - **`RestApiHandler.cpp` / `RestApiHandler.h`**:
      - Manages Binance's REST API requests.
      - Provides functionalities for polling data and executing trades or other commands.
    - **`MessageProcessor.cpp` / `MessageProcessor.h`**:
      - Handles the processing of incoming messages from both WebSocket and REST sources.
      - Uses custom message deduplication logic, likely through `BloomFilter.h`.
    - **`OrderbookManager.cpp` / `OrderbookManager.h`**:
      - Maintains the state of the order book for different trading pairs.
      - Updates order book data based on WebSocket and REST inputs.

2. **Utility Components**:
    - **`ThreadPool.cpp` / `ThreadPool.h`**:
      - Provides thread pool functionality for managing multiple concurrent tasks, ensuring efficient use of resources.
    - **`EventLoop.cpp` / `EventLoop.h`**:
      - Implements an event loop for non-blocking operations, crucial for real-time data handling.
    - **`MemoryPool.h`**:
      - Custom memory management implementation to optimize performance for frequent allocations and deallocations.
    - **`SIMDUtils.h`**:
      - Contains SIMD (Single Instruction, Multiple Data) utility functions for optimizing operations such as data processing and transformations.
    - **`BloomFilter.h`**:
      - Implements a bloom filter, used for fast and memory-efficient duplicate message detection.

3. **Concurrency and Performance Tools**:
    - **`LockFreeQueue.h` / `LockFreePriorityQueue.h`**:
      - Implements lock-free data structures to reduce synchronization bottlenecks.
    - **`Deduplicator.cpp` / `Deduplicator.h`**:
      - Provides additional mechanisms for deduplication, complementing `BloomFilter.h`.

4. **Build Configuration**:
    - **`CMakeLists.txt`**:
      - Configures the project build, defining compilation flags, linking libraries, and organizing project components.
    - **`cmake/FindWebsocketpp.cmake`**:
      - CMake script for finding the WebSocket++ dependency, required for WebSocket handling.

5. **Testing**:
    - **Unit Tests**:
      - **`tests/BinanceClientTest.cpp`**:
        - Tests for the main client interactions.
      - **`tests/WebSocketHandlerTest.cpp`**:
        - Tests WebSocket connection, message handling, and reconnection logic using GoogleTest.
      - **`tests/RestApiHandlerTest.cpp`**:
        - Verifies REST API request handling and polling intervals.
      - **Other Tests**:
        - Unit tests for other components such as `MessageProcessor`, `OrderbookManager`, and utility classes like `ThreadPool` and `EventLoop`.
      - **Testing Framework**: Uses GoogleTest (`gtest`) for writing unit tests, and `gmock` for mocking components where needed.

6. **Miscellaneous**:
    - **`.gitignore`**:
      - Defines files and directories to be ignored by Git version control.
    - **`.vscode/c_cpp_properties.json` / `.vscode/settings.json`**:
      - Configuration files for Visual Studio Code, providing settings for C++ IntelliSense, debugging, and compilation.

## Dependencies
- **Boost.Asio**: Used for asynchronous networking.
- **WebSocket++**: Manages WebSocket communication with Binance servers.
- **C++17 or newer**: Project requires C++17 due to usage of modern language features like smart pointers, lambdas, etc.
- **GoogleTest**: Used for unit testing to ensure the reliability of individual components.

  
### Evaluation Summary of Assignment 1a Solution



#### 1. **Event Loop Handling**
   - **Files Involved**:
     - `BinanceClient.cpp`, `EventLoop.cpp`, `RestApiHandler.cpp`, `WebSocketHandler.cpp`
   - **Current Implementation**:
     - You are leveraging Boost.Asio (`io_context`) for handling asynchronous tasks, which is a suitable choice for non-blocking operations. Additionally, there are custom event loop implementations to manage the flow of events effectively.
   - **Feedback**:
     - To further improve, you might consider using `boost::asio::strand` to better coordinate multiple event loops, reducing the risk of race conditions and ensuring smooth concurrency.

#### 2. **Multithreading**
   - **Files Involved**:
     - `BinanceClient.h`, `ThreadPool.cpp`, `EventLoop.cpp`
   - **Current Implementation**:
     - A custom `ThreadPool` is used to manage tasks concurrently, and multithreading has been effectively implemented using C++ standard threading (`std::thread`) and Boost threading utilities.
   - **Feedback**:
     - Improvements could include balancing workloads dynamically across threads, utilizing thread affinity to allocate specific tasks to dedicated cores, and employing a more dynamic thread pool that scales based on the current workload.

#### 3. **Deduplication**
   - **Files Involved**:
     - `BloomFilter.h`, `Deduplicator.cpp`, `MessageProcessor.h`
   - **Current Implementation**:
     - Deduplication is achieved using a combination of bloom filters and a deduplication mechanism that appears to effectively filter out duplicate messages. This is critical in ensuring each callback is unique.
   - **Feedback**:
     - Consider combining bloom filters with additional hashing mechanisms to further reduce false positives, ensuring more precise and reliable message filtering.

#### 4. **Scaling Horizontally**
   - **Files Involved**:
     - `EventLoop.cpp`, `LockFreeQueue.h`, `ThreadPool.h`
   - **Current Implementation**:
     - Your solution includes several lock-free data structures (`LockFreeQueue` and `LockFreePriorityQueue`) that help in scaling the solution horizontally by minimizing locking bottlenecks. The use of thread pools further helps to manage multiple symbols concurrently.
   - **Feedback**:
     - Improvements can include splitting symbols across specific CPU cores using CPU affinity to avoid overloading a single core. Implementing a load balancing strategy to distribute symbols across available CPU resources would also enhance horizontal scalability.

#### 5. **Low Latency**
   - **Files Involved**:
     - `SIMDUtils.h`, `LockFreeQueue.h`, `MemoryPool.h`
   - **Current Implementation**:
     - Lock-free queues and custom memory pooling are used to minimize latency, and `SIMDUtils.h` is utilized to perform SIMD optimizations. These techniques help to reduce the overhead involved in data processing and thread synchronization.
   - **Feedback**:
     - To further improve, consider reducing redundant logging and minimizing the number of context switches by binding critical tasks to specific CPU cores. Using a hybrid approach that combines lock-free data structures with fine-grained locks in critical sections could also reduce contention.

#### 6. **Reliability**
   - **Files Involved**:
     - `WebSocketHandler.cpp`
   - **Current Implementation**:
     - The WebSocket handler incorporates a reconnection mechanism (`fail_handler`) and error handling routines to ensure the stability of the WebSocket connections.
   - **Feedback**:
     - Consider implementing a more adaptive reconnection strategy that adjusts reconnection times based on previous failures. Adding redundant WebSocket connections that can take over when a primary connection fails would further enhance system reliability.


1. **Event Loop Handling**:
   - Boost.Asio and custom event loops are used to manage asynchronous tasks effectively.
   - Potential improvements include using `boost::asio::strand` to improve coordination across threads and event loops.

2. **Multithreading**:
   - A custom `ThreadPool` is utilized for concurrent task handling, using both Boost threading and C++ threading utilities.
   - Improvements could focus on dynamic thread allocation, CPU affinity, and scaling the thread pool based on current demands.

3. **Deduplication**:
   - Implemented through bloom filters and a deduplication mechanism to ensure only unique messages trigger callbacks.
   - Improvements could include hybrid deduplication techniques to further reduce false positives and improve accuracy.

4. **Scaling Horizontally**:
   - Lock-free data structures and custom memory pooling are used to manage multiple connections and avoid synchronization bottlenecks.
   - Improvements could focus on symbol distribution using load balancing and CPU affinity to avoid overloading specific CPU cores.

5. **Low Latency**:
   - Achieved through lock-free data structures, SIMD optimizations, and memory pooling.
   - Improvements can be made by reducing unnecessary context switches, binding specific tasks to dedicated CPU cores, and reducing logging to prevent I/O bottlenecks.

6. **Reliability**:
   - Error handling and reconnection strategies have been implemented to ensure stable WebSocket connectivity.
   - Improvements could include implementing redundant connections and adaptive reconnection logic.

## Assignment 1b Answers

### 1. CPU Context Switching Diagram

![diagram](TR.png)


#### CPU Context Switching Diagram


The diagram provided illustrates how the threading mechanism is utilized across different tasks to ensure efficient CPU context switching and non-blocking operations for WebSocket and REST connectivity.

- **WebSocket Connectivity (Thread 1)**: Manages WebSocket operations asynchronously, ensuring that the connection is established and messages are deduplicated before processing. WebSocket updates are triggered through callbacks to keep the order book current.
  - The process begins with **WebSocket Connect**, followed by **Receive Data**, **Parse Data**, and **Add to Queue**. After establishing the connection, the system waits for messages and handles **Deduplicate Message** and **Orderbook Update**, calling the **OnOrderbookWs** callback.

- **REST Connectivity (Thread 2)**: Handles REST API polling periodically. This thread is responsible for retrieving order book information and pushing responses to a queue for further processing.
  - The REST thread initiates with a **REST Request**, **Receive Response**, **Parse Response**, and finally **Add to Queue**. The REST polling repeats at intervals, with deduplication and calling the **OnOrderbookRest** callback.

- **Queue and Message Processing (Thread Pool)**: Handles incoming messages by deduplicating them and processing them through a thread pool, ensuring efficient, concurrent execution without blocking.
  - Messages are deduplicated and queued for processing, reducing the chance of duplicates affecting the integrity of the order book.

- **Orderbook Management (Thread Pool)**: Updates the order book with the processed data from WebSocket and REST sources. This step involves sorting prices and maintaining an up-to-date snapshot of market data.
  - Involves operations like **Update Orderbook (Bid/Ask)**, **Sort Prices**, and **Add to Shard** to maintain accuracy and efficiency.

- **Circuit Breaker and Error Handling**: Provides error handling and circuit breaker logic, ensuring reliability and stable execution.
  - The **Circuit Breaker Active** section handles faults gracefully to prevent cascading failures and allows for smooth reconnection or recovery.

- **Concurrent Execution**: Illustrates how WebSocket monitoring and REST polling occur concurrently without interference.
  - Both **WebSocket Monitoring (Thread 1)** and **REST Polling (Thread 2)** are managed concurrently without blocking each other, thanks to asynchronous operations and proper task scheduling.


- The **x-axis** represents the different tasks, including WebSocket connectivity, REST polling, message processing, and orderbook management. The **y-axis** represents time, showing how these tasks are distributed over a given period.
  
- Each CPU core is tasked with managing a specific aspect of the Binance Client operation:
  - **Thread 1** manages **WebSocket connectivity**, which includes establishing the connection, receiving data, deduplicating messages, and triggering the appropriate callbacks for updates.
  - **Thread 2** handles **REST API polling**, which involves sending requests, parsing responses, and adding results to the queue for further processing.
  - A **Thread Pool** is used to manage **message processing**. This thread pool is responsible for deduplication, message sorting, and ensuring that the order book remains accurate.
  - Another **Thread Pool** manages **Orderbook Management**, which includes sorting bid/ask prices and adding data to the appropriate shard.
  
- **Context Switching**:
  - Context switching occurs efficiently between different threads, enabling the system to handle data from both WebSocket and REST APIs simultaneously without blocking the main loop.
  - The **Thread Pools** play a critical role in maintaining smooth and effective context switching, ensuring that no single task monopolizes CPU resources.
  
- **Concurrency and Task Scheduling**:
  - The use of asynchronous tasks ensures that WebSocket and REST operations do not block each other. WebSocket messages are received and processed independently from REST API polling, which ensures that both data sources can be handled concurrently.
  - The **Circuit Breaker** mechanism also runs independently, ensuring that any errors or connection issues are dealt with without affecting the main execution flow.

This setup allows for seamless interaction between the WebSocket and REST components, minimizing latency and ensuring reliability. The tasks are distributed across the available CPU cores to achieve optimal performance, and the usage of multiple threads ensures that the system remains non-blocking and responsive.



Main Thread: Starts the application, initializes resources, and spawns other threads. It doesn't perform blocking operations, ensuring the main loop remains free.

WebSocket Threads: Each symbol (e.g., BTCUSDT, ETHUSDT) has a dedicated thread handling its WebSocket connection. These threads use asynchronous I/O to read data without blocking.

REST Threads: Similar to WebSocket threads, each symbol has a thread that periodically polls the REST API. The polling interval is managed to prevent resource exhaustion.

Message Processing Thread Pool: A pool of worker threads that process messages from a thread-safe queue. They perform deduplication and invoke the appropriate callbacks.

Co-existence Without Blocking:

Asynchronous Operations: Both WebSocket and REST threads use asynchronous I/O provided by Boost.Asio. This allows threads to initiate I/O operations and continue without waiting for them to complete.

Thread-safe Queues: Incoming messages are placed into thread-safe queues, preventing contention and ensuring safe communication between threads.

Non-blocking Main Loop: The main thread doesn't perform any blocking operations. It oversees the initialization and can handle other tasks like monitoring or scaling.

Efficient Threading: By dedicating threads to specific tasks and using asynchronous operations, I minimize context switching and CPU overhead.



#### Answer to Question 2: Potential Bottlenecks and Monitoring Recommendations

1. **CPU Overload**:
   - With multiple threads managing WebSocket connections, REST polling, and order book updates, certain CPU cores could become overloaded, especially under high market volatility with rapid data flow.
   - **Monitoring**: Monitor CPU utilization per core. Tools like `htop` or Prometheus can be used to observe and track how each CPU core is handling tasks, ensuring no single core becomes a bottleneck.

2. **Thread Contention and Synchronization**:
   - Thread contention could occur when multiple threads attempt to access shared resources like queues or memory, especially if lock-free structures are not used effectively.
   - **Monitoring**: Monitor thread contention using tools like `perf` or `Thread Sanitizer` to ensure that contention issues do not degrade performance. Look out for thread blocking or frequent context switches.

3. **Network Bandwidth**:
   - WebSocket data streaming and REST API polling consume a lot of network bandwidth. Network saturation could become a bottleneck, affecting data timeliness.
   - **Monitoring**: Network utilization should be monitored using tools like `iftop` or Prometheus. It is crucial to ensure sufficient bandwidth, especially during high trading volume.

4. **Memory Usage and Garbage Collection**:
   - High-frequency data flow requires efficient memory management. Memory fragmentation and improper pooling could lead to increased latency or even crashes.
   - **Monitoring**: Use tools like `valgrind` or Prometheus to monitor memory usage and detect leaks or inefficient allocation patterns. Focus on monitoring the usage of the memory pool to ensure optimal performance.

5. **Latency in Message Deduplication and Processing**:
   - Deduplication of messages, especially if the volume is high, can become a bottleneck if not handled effectively. Inefficient deduplication could lead to stale data or increased processing delays.
   - **Monitoring**: Track message processing latency and queue sizes. Tools like OpenTelemetry can provide metrics on how long each message takes from reception to processing.

6. **Circuit Breaker Overhead**:
   - While circuit breakers are essential for handling failures, frequent triggering could add overhead and impact the system’s responsiveness.
   - **Monitoring**: Monitor the number of times the circuit breaker is triggered. An increasing frequency of circuit breaker activations could indicate underlying network or processing issues.

#### Answer to Question 3: Potential Improvements to Reduce Latency

Given more time, the following improvements can be made to further reduce latency and enhance performance:

1. **Adaptive REST Polling Intervals**:
   - Implement adaptive polling intervals that change based on market volatility. For instance, increase the polling frequency during high activity periods and decrease it during low activity times.
   - **Benefit**: Reducing the number of requests during low-activity periods can free up resources, allowing the system to respond faster when market activity spikes.

2. **CPU Affinity and Core Binding**:
   - Bind critical threads (such as WebSocket connectivity and REST polling) to specific CPU cores to ensure that context switching overhead is minimized and CPU cache is used effectively.
   - **Benefit**: This can reduce cache misses and improve the predictability of task execution, resulting in lower latency for critical operations.

3. **Batch Processing of Messages**:
   - Instead of processing messages individually, batch them together where feasible to reduce processing overhead.
   - **Benefit**: Batch processing reduces the number of function calls and context switches, decreasing the overall processing time.

4. **Improved Deduplication Mechanism**:
   - Enhance the deduplication logic by combining bloom filters with a more sophisticated hashing mechanism to reduce false positives and improve overall efficiency.
   - **Benefit**: Faster and more accurate deduplication leads to reduced processing times and ensures the order book remains current.

5. **Offloading Computational Tasks to Specialized Hardware**:
   - Offload heavy computations to specialized hardware like FPGAs or GPUs to reduce the load on the CPU.
   - **Benefit**: Hardware acceleration can lead to significant improvements in processing time for specific computational tasks, such as data parsing or deduplication.

6. **Asynchronous Memory Allocation and Optimized Memory Pooling**:
   - Refactor the memory pool to make use of asynchronous memory allocation techniques and optimize the allocation strategy to minimize fragmentation.
   - **Benefit**: Reducing memory allocation time will help minimize the latency associated with creating and destroying objects in high-frequency trading scenarios.

7. **Optimized Thread Pool Management**:
   - Implement a dynamic thread pool management system that scales the number of threads based on workload in real-time.
   - **Benefit**: Maintaining an optimal number of threads ensures efficient CPU usage without overloading the system, reducing the likelihood of bottlenecks.

8. **Network Optimization**:
   - Use a more efficient network protocol or compression mechanisms to minimize the amount of data being transmitted, especially during periods of high traffic.
   - **Benefit**: Lower network load results in faster data transmission, which directly impacts the latency of WebSocket messages and REST responses.

9. **SIMD Vectorization for Data Processing**:
   - Use SIMD instructions for processing multiple elements simultaneously when deduplicating or parsing incoming data.
   - **Benefit**: SIMD enables parallel data processing at the hardware level, which is highly effective for reducing the time taken for repetitive tasks.




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

11. **Algorithmic Optimizations:**
    - **Action**: Review and optimize algorithms used in message processing and orderbook management.
    - **Benefit**: Reduced computational complexity, faster execution.

---

