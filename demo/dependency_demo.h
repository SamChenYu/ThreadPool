#pragma once

#include <vector>
#include <iostream>
#include "../src/threadpool.h"

// For sleeping the threads
#include <chrono>
#include <thread>

// Demo showing threadpool usage with task dependencies

struct data {
    data() = default;
    explicit data(int value) : m_value(value) {}
    int m_value;
};

inline data fetch_api_a() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return data{1};
}

inline data fetch_api_b() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return data{2};
}

inline data clean_api_a(data d) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return data{3};
}

inline data clean_api_b(data d) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return data{4};
}

inline data merge_data(data a, data b) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return data{5};
}

inline data run_analysis(data d) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return data{6};
}


inline void dependency_dag_example() {
    //threadpool tp(2);

    // Simulated ETL pipeline
    // Fetch → Clean → Merge → Analysis


    // auto api_a = tp.submit<data>([] { return fetch_api_a(); });
    // auto api_b = tp.submit<data>([] { return fetch_api_b(); });
    //
    // auto clean_a = api_a.then(
    //     tp,
    //     std::function<data(data)>(
    //         [](data d) { return clean_api_a(d);}
    //     )
    // );
    //
    // auto clean_b = api_b.then(
    //     tp,
    //     std::function<data(data)>(
    //         [](data d) { return clean_api_b(d);}
    //     )
    // );

    // auto merge = tp.when_all(clean_a, clean_b)
    //                .then(tp, [](data a, data b) {
    //                    return merge_data(a, b);
    //                });
    //
    // auto analysis = merge.then(tp, [](data m) {
    //     return run_analysis(m);
    // });

    //tp.shutdown();

}