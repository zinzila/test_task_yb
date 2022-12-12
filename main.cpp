
#include <iostream>
#include <numeric>

#include "src/IEventProcessor.h"
#include "src/simulation/SingleExecution.h"

int main(int, char**)
{
    constexpr auto k_producers = 50;
    constexpr auto k_events = 25000;

    SingleExecution execution{k_producers, k_events};

    auto stats = execution.Launch();

    auto p_avg =
        std::accumulate(stats.producers_.begin(),
                        stats.producers_.end(),
                        0,
                        [](auto init, auto p) { return init + p.execution_time_.count(); }) /
        stats.producers_.size();

    std::cout << k_producers << ", " << k_events << ", " << stats.execution_time_.count() << ", "
              << p_avg << "\n";
}
