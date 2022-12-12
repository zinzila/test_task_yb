#include "IEventProcessor.h"

#include <cassert>
#include <mutex>

#include "IEvent.h"

IEventProcessor::ReservedEvent::ReservedEvent(void) : sequence_number_{0}, event_{nullptr}
{
}

IEventProcessor::ReservedEvent::ReservedEvent(const Integer sequence_number, void *const event)
    : sequence_number_{sequence_number}, event_{event}
{
    assert(event_ != nullptr);
}

IEventProcessor::IEventProcessor() : execution_thread_{[this]() { Run(); }}
{
}

IEventProcessor::~IEventProcessor()
{
    stop_.store(true);
    queue_cv_.notify_one();
    execution_thread_.join();
}

void IEventProcessor::Commit(const Integer sequence_number)
{
    // NOTE: Since we don't have event release logic, it's OK to pass the pointer to the execution
    // queue like this.
    auto *event = [&, this]() {
        std::lock_guard lock_event{event_mutex_};
        return events_.at(sequence_number);
    }();

    {
        std::lock_guard lock_queue{queue_mutex_};
        queue_.push(event);
    }
    queue_cv_.notify_one();
}

std::pair<Integer, void *> IEventProcessor::ReserveEvent()
{
    // NOTE: Memory leak is intentional to avoid implementing cleanup in an intermediate version
    void *new_object = new char[kMaxEventSize];

    auto sequence_number = current_number_.fetch_add(1);

    auto const [it, inserted] = [&, this]() {
        std::lock_guard lock{event_mutex_};
        return events_.emplace(sequence_number, new_object);
    }();

    // if a value was replaced then something went wrong
    assert(inserted == true);

    return *it;
}

void IEventProcessor::Run()
{
    while (!stop_.load()) {
        void *event = nullptr;

        std::unique_lock lock(queue_mutex_);
        queue_cv_.wait(lock, [this] { return !queue_.empty() || stop_.load(); });

        if (!queue_.empty()) {
            event = queue_.front();
            queue_.pop();

            lock.unlock();

            assert(event != nullptr);

            static_cast<IEvent *>(event)->Process();
        }
    }
}
