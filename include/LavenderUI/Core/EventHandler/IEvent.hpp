#pragma once

#include <cstdint>

#include "include/LavenderUI/Utils/Misc.hpp"

namespace lav::core
{
struct IEvent
{
    virtual ~IEvent() = default;
    virtual auto getEventId() const -> uint32_t = 0;
};

/**
    @brief Base event from which every new custom event needs to inherit from.
        Ids are given automatically through CRTP.
*/
template<typename Derived>
struct IEventCRTP : public IEvent
{
    virtual ~IEventCRTP() = default;
    auto getEventId() const -> uint32_t override { return Derived::eventId; };

    static const uint32_t eventId;
};

template<typename Derived>
const uint32_t IEventCRTP<Derived>::eventId = utils::genId();
} // namespace lav::core