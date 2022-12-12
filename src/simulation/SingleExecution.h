#pragma once

#include <chrono>
#include <future>
#include <vector>

#include "src/IEventProcessor.h"
#include "src/simulation/TestEventProducer.h"

// Encapsulates single simulation execution
class SingleExecution
{
public:
    struct Stats
    {
        std::chrono::microseconds execution_time_{};
        std::vector<TestEventProducer::Stats> producers_;
    };

public:
    SingleExecution(size_t producers_count, uint_fast32_t event_count);
    ~SingleExecution();

    void Prepare();

    Stats Launch();

private:
    uint_fast32_t event_count_{};
    std::shared_ptr<IEventProcessor> event_processor_;
    std::vector<std::shared_ptr<TestEventProducer>> producers_;
    std::vector<std::thread> threads_;
    std::promise<void> start_p_;
};
