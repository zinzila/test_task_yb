#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>

#include "src/IEvent.h"
#include "src/IEventProcessor.h"

struct Event : public IEvent
{
    ~Event() override = default;
    void Process() override
    {
        processed = true;
    }

    std::atomic_bool processed{false};
};

TEST_CASE("IEventProcessor")
{
    GIVEN("A freshly initialized IEventProcessor")
    {
        IEventProcessor event_processor;

        WHEN("An event is reserved")
        {
            auto re = event_processor.Reserve<Event>();

            THEN("New event is reserved")
            {
                REQUIRE(re.IsValid());
                REQUIRE(re.GetEvent() != nullptr);

                AND_WHEN("Event is accessed")
                {
                    auto* event = static_cast<Event*>(re.GetEvent());
                    event->Process();
                    REQUIRE(event->processed == true);
                }

                AND_WHEN("This event is commited for execution")
                {
                    event_processor.Commit(re.GetSequenceNumber());

                    // NOTE: not nice, but works for this simple test
                    std::this_thread::sleep_for(std::chrono::milliseconds{50});

                    THEN("Event is processed")
                    {
                        auto* event = static_cast<Event*>(re.GetEvent());
                        REQUIRE(event->processed == true);
                    }
                }
            }
        }
    }
}
