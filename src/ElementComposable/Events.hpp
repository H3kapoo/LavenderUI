#pragma once

#include <functional>
#include <typeindex>
#include <unordered_map>

#include "IEvent.hpp"

namespace src::elementcomposable
{
using EventCallback = std::function<void(const IEvent&)>;

/**
    @brief Manager handling the storage and dispatch of user or window generated events.
*/
class Events
{
public:
    /**
        @brief Listen to a specific template event EventT and call the callback when it happens.

        @param callback Callback to be called upon event triggered.

        @return Myself.
    */
    template<typename EventT>
    auto listenTo(const std::function<void(const EventT)>& callback) -> Events&
    {
        const uint32_t key = std::type_index(typeid(EventT)).hash_code();
        eventMap_[key] = [callback](const IEvent& e)
        {
            if (const auto eCast = dynamic_cast<const EventT*>(&e))
            {
                callback(*eCast);
            }
        };

        return *this;
    }

    /**
        @brief Emit an event EventT in order to trigger the listeners.

        @param event Event to be triggered.
    */
    template<typename EventT>
    auto emitEvent(EventT& event) -> void
    {
        const uint32_t key = std::type_index(typeid(EventT)).hash_code();

        if (!eventMap_.contains(key)) { return; }

        eventMap_[key](event);
    }

private:
    std::unordered_map<uint32_t, EventCallback> eventMap_;
};
} //namespace src::elementcomposable
