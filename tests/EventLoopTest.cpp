#include <gtest/gtest.h>
#include "../EventLoop.h"

TEST(EventLoopTest, PostAndRun) {
    EventLoop loop;
    std::atomic<int> counter(0);

    for (int i = 0; i < 10; ++i) {
        loop.post([&counter]() { ++counter; });
    }

    std::thread t([&loop]() { loop.run(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Give some time for processing
    loop.stop();
    t.join();

    EXPECT_EQ(counter, 10);
}
