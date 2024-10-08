![diagram](https://github.com/user-attachments/assets/59a04622-b443-4a9b-86aa-a936583f61b1)# Binance Client

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

### Detailed Explanation of the Diagram: CPU Context Switching for Binance Client

This timeline diagram demonstrates how **CPU context switching** happens between tasks in the **Binance Client** system for **WebSocket** and **REST API** operations, as managed by threading mechanism and task schedulers. Below is a breakdown of each section.

---

#### **1. WebSocket Connectivity (Thread 1)**

- **0s: WebSocket Connect**: 
   - The `WebSocketHandler` establishes a connection to the Binance WebSocket API to start receiving live market data.
   - This task is assigned to **Thread 1**.
  
- **0s - 1s: Receive Data and Add to Queue**: 
   - After connecting, the WebSocket receives data, such as orderbook updates or trade information. The `WebSocketHandler` processes this data and adds it to a queue for further processing by the `MessageProcessor`.
   - This is done asynchronously to avoid blocking the main loop.

- **1s - 2s: Callback `OnOrderbookWs`**: 
   - Once the WebSocket data is queued, a callback function (`OnOrderbookWs`) is triggered, which calls the `MessageProcessor` to handle the incoming data.
  
- **2s: WebSocket Wait and Handle New Data**:
   - After the initial data is handled, Thread 1 goes into a waiting state, monitoring for more incoming data from the WebSocket.
   - New data received from the WebSocket will be handled similarly, repeating this cycle.

---

#### **2. REST Connectivity (Thread 2)**

- **0s: REST Request**:
   - Simultaneously, **Thread 2** initiates a REST API request to periodically poll Binance for market data (e.g., orderbook snapshots).
   - This is managed by the `RestApiHandler`.

- **1s - 2s: Handle Response and Add to Queue**: 
   - Once the response from the REST API is received, it is parsed by the `RestApiHandler` and added to a queue, similar to the WebSocket process.

- **2s - 3s: Callback `OnOrderbookRest`**: 
   - A callback (`OnOrderbookRest`) is triggered, sending the REST data to the `MessageProcessor` for deduplication and further processing.

- **3s: REST Polling and Handle New Data**:
   - The system continues to poll the REST API periodically, retrieving new data and repeating the process.

---

#### **3. Queue and Message Processing (Thread Pool)**

- **1s - 2s: Deduplicate Messages**:
   - Once WebSocket or REST data is added to the queue, the `MessageProcessor` is responsible for deduplicating the messages to ensure that only unique updates are processed. This happens in the background using the **thread pool**.
   
- **2s - 3s: Process WebSocket and REST Data**:
   - After deduplication, the `MessageProcessor` processes the messages (e.g., updating the orderbook) and forwards them to the `OrderbookManager`.

---

#### **4. Concurrent Execution**

- **0s - 5s: WebSocket Monitoring, REST Polling, and Thread Pool**:
   - Throughout the execution, WebSocket monitoring, REST polling, and message processing occur concurrently.
   - The **thread pool** efficiently handles tasks by dynamically distributing them across available threads, ensuring both WebSocket and REST tasks can coexist without blocking each other.
   - This enables the system to handle multiple tasks simultaneously, optimizing CPU usage.

---

### Co-existence Without Blocking:

1. **Asynchronous Operation**: 
   - The WebSocket (`WebSocketHandler`) and REST API (`RestApiHandler`) run asynchronously using separate threads, ensuring that neither blocks the other.

2. **Thread Pool Efficiency**: 
   - The thread pool is used for tasks such as message deduplication and processing, dynamically assigning available threads to avoid CPU bottlenecks and allow smooth handling of multiple data sources.

3. **Task Scheduling**:
   - Tasks such as WebSocket message reception, REST API polling, and queue processing are scheduled efficiently to maximize CPU utilization without causing delays.

4. **Callbacks (`OnOrderbookWs` and `OnOrderbookRest`)**:
   - Callbacks ensure that data from both WebSocket and REST connections are processed in a timely manner, feeding the processed data into the `OrderbookManager`.

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

