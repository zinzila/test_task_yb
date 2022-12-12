#include "SingleExecution.h"

#include <future>
#include <iostream>

SingleExecution::SingleExecution(size_t producers_count, uint_fast32_t event_count)
    : event_count_{event_count}, event_processor_{std::make_shared<IEventProcessor>()}
{
    producers_.reserve(producers_count);

    for (size_t i = 0; i < producers_count; ++i) {
        producers_.push_back(std::make_shared<TestEventProducer>(event_processor_, event_count));
    }

    threads_.reserve(producers_count);

    Prepare();
}

SingleExecution::~SingleExecution()
{
    for (auto& t : threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void SingleExecution::Prepare()
{
    threads_.clear();

    start_p_ = std::promise<void>{};
    auto start = start_p_.get_future().share();

    for (const auto& p : producers_) {
        threads_.push_back(
            std::thread{[&](TestEventProducer* p, std::shared_future<void> start) -> void {
                            start.wait();

                            p->Run();
                        },
                        p.get(),
                        start});
    }
}

SingleExecution::Stats SingleExecution::Launch()
{
    StopWatch stop_watch;
    Stats stats;

    stop_watch.Start();

    start_p_.set_value();

    for (auto& t : threads_) {
        t.join();
    }

    stats.execution_time_ = stop_watch.Stop();

    for (const auto& p : producers_) {
        stats.producers_.push_back(p->GetStats());
    }

    return stats;
}
