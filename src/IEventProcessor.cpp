#include "IEventProcessor.h"

#include <cassert>
#include <memory>
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
    void *event;
    {
        std::shared_lock lock_event{event_mutex_};
        event = reserved_events_[sequence_number].storage_.get();
    }
    {
        std::lock_guard lock_queue{queue_mutex_};
        queue_.push(event);
    }
    queue_cv_.notify_one();
}

std::pair<Integer, void *> IEventProcessor::ReserveEvent()
{
    EventReservation er;
    er.storage_ = std::make_unique<std::byte[]>(k_max_event_size);

    std::lock_guard lock{event_mutex_};
    auto result = std::pair{reserved_events_.size(), er.storage_.get()};
    reserved_events_.push_back(std::move(er));

    return result;
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
