#pragma once

#include <atomic>

#include "src/IEvent.h"

class TestEvent : public IEvent
{
public:
    TestEvent() = default;

    virtual ~TestEvent(void) override = default;

    TestEvent(const TestEvent&) = delete;
    TestEvent& operator=(const TestEvent&) = delete;
    TestEvent(TestEvent&&) = delete;
    TestEvent& operator=(TestEvent&&) = delete;

    virtual void Process(void) override
    {
        call_count_.fetch_add(1, std::memory_order_relaxed);
    }

    int GetCallCount() const
    {
        return call_count_;
    }

private:
    std::atomic<int> call_count_{};
};
