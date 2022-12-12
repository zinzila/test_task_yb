#pragma once

#include <chrono>
class StopWatch
{
    using Clock = std::chrono::high_resolution_clock;

public:
    void Start()
    {
        start_ = Clock::now();
    }

    std::chrono::microseconds Stop()
    {
        auto now = Clock::now();
        last_measurement_ = now - start_;
        return std::chrono::duration_cast<std::chrono::microseconds>(last_measurement_);
    }

    std::chrono::microseconds GetLastMeasurement() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(last_measurement_);
    }

private:
    Clock::time_point start_{};
    Clock::duration last_measurement_{};
};
