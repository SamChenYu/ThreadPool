#pragma once

#include "../src/threadpool.h"

// For sleeping the threads
#include <chrono>
#include <thread>
#include <iostream>

// Demo showing dependencies placed on different thread pools

struct IO_Data {
    IO_Data() = default;
    explicit IO_Data(int value) : m_value(value) {}
    int m_value;
};

inline IO_Data read() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return IO_Data{1};
}

inline IO_Data parse(IO_Data data) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return IO_Data{2};
}

inline IO_Data compress(IO_Data data) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return IO_Data{3};
}

inline void upload(IO_Data data) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

inline void multiple_threadpool_example() {
    // Read file (IO) → Parse (CPU) → Compress (CPU) → Upload (IO)

    //threadpool io_pool(5); // IO tasks will be blocking
    //threadpool cpu_pool(2); //

    // abhorrent API - needs to be fixed!
    // auto read_rv = io_pool.submit<IO_Data>( []() { return read(); });
    //
    // auto parse_rv = read_rv.then<IO_Data>(
    //     cpu_pool,
    //         std::function<IO_Data(IO_Data)>(
    //                 [](IO_Data data) ->IO_Data { return parse(data); }
    //             )
    //         );
    //
    // auto compress_rv = parse_rv.then<IO_Data>(
    //     cpu_pool,
    //     std::function<IO_Data(IO_Data)>(
    //         [](IO_Data data) -> IO_Data { return compress(data); }
    //         )
    //     );
    //
    // auto upload_rv = compress_rv.then<void>(
    //     io_pool,
    //     std::function<void(IO_Data)>(
    //         [](IO_Data data) { upload(data); }
    //         )
    //     );
    // there may be mismatch here!
    // io_pool shutdowns first, finished all the tasks, but
    // some tasks on the cpu_pool enqueue onto the io_pool??

    // cpu_pool.shutdown();
    // io_pool.shutdown();
    //
    //
    // std::cout << "read_rv: " << read_rv.get().m_value <<  std::endl;
    // std::cout << "parse_rv: " << parse_rv.get().m_value << std::endl;
    // std::cout << "compress_rv: " << compress_rv.get().m_value << std::endl;

}