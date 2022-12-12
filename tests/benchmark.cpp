#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

#include "src/IEventProcessor.h"
#include "src/simulation/SingleExecution.h"
#include "src/simulation/TestEvent.h"

TEST_CASE("IEventProcessor benchmark")
{
    IEventProcessor event_processor;

    BENCHMARK("Reserve event")
    {
        event_processor.Reserve<TestEvent>();
    };

    BENCHMARK("Reserve and Commit event")
    {
        auto event = event_processor.Reserve<TestEvent>();
        event_processor.Commit(event.GetSequenceNumber());
    };
}

TEST_CASE("Threaded benchmark")
{
    constexpr uint_least32_t k_event_count{5000};

    SECTION("Validate")
    {
        SingleExecution exec{3, k_event_count};
        auto stats = exec.Launch();

        for (const auto& ps : stats.producers_) {
            REQUIRE(ps.call_count_ == k_event_count);
        }
    }

    auto create_instances = [&](int runs, size_t count) {
        std::vector<std::shared_ptr<SingleExecution>> instances;
        for (int i = 0; i < runs; ++i) {
            instances.push_back(std::make_shared<SingleExecution>(count, k_event_count));
        }
        return instances;
    };

    auto name = std::string{"1 producer, "} + std::to_string(k_event_count) + " events";
    BENCHMARK_ADVANCED(name.c_str())(Catch::Benchmark::Chronometer meter)
    {
        auto instances = create_instances(meter.runs(), 1);
        meter.measure([&](int i) { instances[i]->Launch(); });
    };

    name = std::string{"10 producers, "} + std::to_string(k_event_count) + " events";
    BENCHMARK_ADVANCED(name.c_str())(Catch::Benchmark::Chronometer meter)
    {
        auto instances = create_instances(meter.runs(), 10);
        meter.measure([&](int i) { instances[i]->Launch(); });
    };
}
