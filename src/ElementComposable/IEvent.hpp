#pragma once

#include <cstdint>
namespace src::elementcomposable
{
enum class EventType
{
    MOUSE_MOVE,
    MOUSE_CLICK
};

struct IEvent
{
    virtual ~IEvent() = default;
    virtual auto getType() const -> EventType = 0;
};

struct MouseMoveEvt : public IEvent
{
    MouseMoveEvt(int32_t xIn, int32_t yIn) : x{xIn}, y{yIn} {}
    int32_t x{0};
    int32_t y{0};

    auto getType() const -> EventType { return EventType::MOUSE_MOVE; }
};
} // namespace src::elementcomposable