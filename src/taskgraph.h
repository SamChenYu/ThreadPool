#pragma once

#include "threadpool.h"

// Static scheduler that includes topological sorting

class taskgraph : public threadpool {




    //Dependency DAG API
    template<typename... Args>
    auto when_all(Args... args) {
        // Todo: actually implement the logic

    }

};