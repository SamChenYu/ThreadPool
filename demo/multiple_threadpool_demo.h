#pragma once

#include <vector>
#include <iostream>
#include "../src/threadpool.h"

// For sleeping the threads
#include <chrono>
#include <thread>

// Demo showing dependencies placed on different thread pools

struct IO_Data{};

inline IO_Data read() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return IO_Data{};
}

inline IO_Data parse(IO_Data data) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return data;
}

inline IO_Data compress(IO_Data data) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return data;
}

inline void upload(IO_Data data) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

inline void multiple_threadpool_example() {
    // Read file (IO) → Parse (CPU) → Compress (CPU) → Upload (IO)

    threadpool io_pool(5); // IO tasks will be blocking
    threadpool cpu_pool(2); //

    auto read_rv = io_pool.submit<IO_Data>( []() { return read(); });

    auto parse_rv = read_rv.then( cpu_pool, [](IO_Data data) { return parse(data); });

    auto compress_rv = parse_rv.then(cpu_pool, [](IO_Data data) { return compress(data); });

    auto upload_rv = compress_rv.then(cpu_pool, [](IO_Data data) { return upload(data); });

    io_pool.shutdown();
    cpu_pool.shutdown();

}