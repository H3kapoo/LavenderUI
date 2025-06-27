#pragma once

#include "src/ElementEvents/IEvent.hpp"
#include <cstdint>
#include <functional>
#include <print>
#include <typeindex>
#include <unordered_map>

namespace src::elementevents
{
using EventCallback = std::function<void(const IEvent&)>;

class EventManager
{
public:
    template<typename EventT>
    void listen(const std::function<void(const EventT)>& callback)
    {
        const uint32_t key = std::type_index(typeid(EventT)).hash_code();
        eventMap_[key] = [callback](const IEvent& e)
        {
            if (const auto eCast = dynamic_cast<const EventT*>(&e))
            {
                callback(*eCast);
            }
        };
    }

    template<typename EventT>
    void emit(EventT& event)
    {
        const uint32_t key = std::type_index(typeid(EventT)).hash_code();

        if (!eventMap_.contains(key)) { return; }

        eventMap_[key](event);
    }

private:
    std::unordered_map<uint32_t, EventCallback> eventMap_;
};
} //namespace src::elementevents
