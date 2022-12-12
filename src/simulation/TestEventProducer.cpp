#include "TestEventProducer.h"

#include <cstdint>
#include <thread>

//
#include "TestEvent.h"

TestEventProducer::TestEventProducer(std::shared_ptr<IEventProcessor> event_processor,
                                     uint_fast32_t event_count)
    : event_processor_{std::move(event_processor)}, event_count_{event_count}
{
}

void TestEventProducer::Run()
{
    auto event = event_processor_->Reserve<TestEvent>();
    auto* test_event = static_cast<TestEvent*>(event.GetEvent());

    stop_watch_.Start();
    uint_fast32_t counter{};
    while (++counter <= event_count_) {
        event_processor_->Commit(event.GetSequenceNumber());
        std::this_thread::sleep_for(event_delay_);
    }

    while (test_event->GetCallCount() < event_count_) {
        std::this_thread::yield();
    }

    stats_.id_ = this;
    stats_.execution_time_ = stop_watch_.Stop();
    stats_.call_count_ = test_event->GetCallCount();
}

TestEventProducer::Stats TestEventProducer::GetStats() const
{
    return stats_;
}
