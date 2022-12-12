#pragma once

#include <chrono>
#include <cstdint>
#include <memory>

#include "src/IEventProcessor.h"
#include "src/simulation/StopWatch.h"

class TestEventProducer
{
public:
    struct Stats
    {
        void* id_{nullptr};  // Only used as an identifier
        int call_count_{};
        std::chrono::microseconds execution_time_{};
    };

public:
    TestEventProducer(std::shared_ptr<IEventProcessor> event_processor, uint_fast32_t event_count);

    void Run();

    Stats GetStats() const;

private:
    StopWatch stop_watch_;
    std::shared_ptr<IEventProcessor> event_processor_;
    uint_fast32_t event_count_{1};
    std::chrono::milliseconds event_delay_{0};
    Stats stats_;
};
