#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <utility>
#include <vector>

#include "SpinLock.h"

using Integer = int_fast64_t;

template <typename TEvent>
class NewPlacementConstructor
{
public:
    template <class... Args>
    static void Construct(void *buffer, Args &&...args);
};

class IEventProcessor
{
public:
    // Maximum size of an event object
    static constexpr size_t k_max_event_size = 256;

public:
    ////////////////////////////////////////////////////////////////////////
    //
    /// ---
    struct ReservedEvent
    {
        ReservedEvent(void);
        ReservedEvent(const Integer sequence_number, void *const event);

        ReservedEvent(const ReservedEvent &) = delete;
        ReservedEvent &operator=(const ReservedEvent &) = delete;
        ReservedEvent(ReservedEvent &&) = delete;
        ReservedEvent &operator=(ReservedEvent &&) = delete;

        Integer GetSequenceNumber(void) const
        {
            return sequence_number_;
        }

        void *GetEvent(void) const
        {
            return event_;
        }

        bool IsValid() const
        {
            return event_ != nullptr;
        }

    private:
        const Integer sequence_number_;
        void *const event_;
    };

    ////////////////////////////////////////////////////////////////////////
    //
    /// ---
    struct ReservedEvents
    {
        ReservedEvents(const Integer sequence_number,
                       void *const event,
                       const size_t count,
                       const size_t event_size);

        ReservedEvents(const ReservedEvents &) = delete;
        ReservedEvents &operator=(const ReservedEvents &) = delete;
        ReservedEvents(ReservedEvents &&) = delete;
        ReservedEvents &operator=(ReservedEvents &&) = delete;

        template <class TEvent, template <class> class Constructor, class... Args>
        void Emplace(const size_t index, Args &&...args)
        {
            void *const event = GetEvent(index);
            if (event)
                Constructor<TEvent>::Construct(event, std::forward<Args>(args)...);
        }

        template <class TEvent, class... Args>
        void Emplace(const size_t index, Args &&...args)
        {
            void *const event = GetEvent(index);
            if (event)
                NewPlacementConstructor<TEvent>::Construct(event, std::forward<Args>(args)...);
        }

        Integer GetSequenceNumber(void) const
        {
            return sequence_number_;
        }

        void *const GetEvent(const size_t index) const;

        size_t Count(void) const
        {
            return count_;
        }

    private:
        const Integer sequence_number_;
        void *const events_;
        const size_t count_;
        const size_t event_size_;
    };

public:
    IEventProcessor();
    ~IEventProcessor();

    template <class T, template <class> class Constructor, class... Args>
    ReservedEvent Reserve(Args &&...args)
    {
        const auto reservation = ReserveEvent();
        if (!reservation.second)
            return ReservedEvent();

        Constructor<T>::Construct(reservation.second, std::forward<Args>(args)...);
        return ReservedEvent(reservation.first, reservation.second);
    }

    ////////////////////////////////////////////////////////////////////////
    //
    /// ---
    template <class T, class... Args>
    ReservedEvent Reserve(Args &&...args)
    {
        const auto reservation = ReserveEvent();
        if (!reservation.second)
            return ReservedEvent();

        NewPlacementConstructor<T>::Construct(reservation.second, std::forward<Args>(args)...);
        return ReservedEvent(reservation.first, reservation.second);
    }

    ////////////////////////////////////////////////////////////////////////
    //
    /// ---
    std::vector<ReservedEvents> ReserveRange(const size_t size);

    ////////////////////////////////////////////////////////////////////////
    //
    /// ---
    void Commit(const Integer sequence_number);

    ////////////////////////////////////////////////////////////////////////
    //
    /// ---
    void Commit(const Integer sequence_number, const size_t count);

private:
    std::pair<Integer, void *> ReserveEvent();

    void Run();

private:
    struct EventReservation
    {
        std::unique_ptr<std::byte[]> storage_;
    };

private:
    std::atomic_bool stop_{false};

    std::atomic<int_fast64_t> current_number_;

    std::vector<EventReservation> reserved_events_;
    SpinLock event_mutex_;

    std::vector<void *> queue_;
    SpinLock queue_mutex_;
    std::condition_variable queue_cv_;

    std::thread execution_thread_;
};

template <class TEvent>
template <class... Args>
void NewPlacementConstructor<TEvent>::Construct(void *buffer, Args &&...args)
{
    static_assert(sizeof(TEvent) < IEventProcessor::k_max_event_size,
                  "Event size is not supported");
    auto *event = new (buffer) TEvent(std::forward<Args>(args)...);
    assert(event != nullptr);
}
