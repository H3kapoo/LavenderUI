#pragma once

#include "src/Utils/Misc.hpp"
#include <cstdint>

namespace src::elementcomposable
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

struct MouseMoveScanEvt : public IEventCRTP<MouseMoveScanEvt>
{};

struct MouseMoveEvt : public IEventCRTP<MouseMoveEvt>
{
    MouseMoveEvt() {}
    MouseMoveEvt(int32_t xIn, int32_t yIn) : x{xIn}, y{yIn} {}
    int32_t x{0}, y{0};
};

struct MouseButtonEvt : public IEventCRTP<MouseButtonEvt>
{
    MouseButtonEvt() {}
    MouseButtonEvt(uint8_t btnIn, uint8_t actionIn) : btn{btnIn}, action{actionIn} {}
    uint8_t btn{0};
    uint8_t action{0};
};

struct MouseLeftClickEvt : public IEventCRTP<MouseLeftClickEvt>
{};

struct MouseLeftReleaseEvt : public IEventCRTP<MouseLeftReleaseEvt>
{};

struct MouseEnterEvt : public IEventCRTP<MouseEnterEvt>
{
    MouseEnterEvt() {}
    MouseEnterEvt(int32_t xIn, int32_t yIn) : x{xIn}, y{yIn} {}
    int32_t x{0}, y{0};
};

struct MouseExitEvt : public IEventCRTP<MouseExitEvt>
{
    MouseExitEvt() {}
    MouseExitEvt(int32_t xIn, int32_t yIn) : x{xIn}, y{yIn} {}
    int32_t x{0}, y{0};
};

struct MouseDragEvt : public IEventCRTP<MouseDragEvt>
{
    MouseDragEvt() {}
    MouseDragEvt(int32_t xIn, int32_t yIn) : x{xIn}, y{yIn} {}
    int32_t x{0}, y{0};
};

struct MouseScrollEvt : public IEventCRTP<MouseScrollEvt>
{
    MouseScrollEvt() {}
    MouseScrollEvt(int8_t xOffsetIn, int8_t yOffsetIn) : xOffset{xOffsetIn}, yOffset{yOffsetIn} {}
    int8_t xOffset{0}, yOffset{0};
};

struct SliderEvt : public IEventCRTP<SliderEvt>
{
    SliderEvt() {}
    SliderEvt(float valueIn) : value{valueIn} {}
    float value{0};
};

struct WindowResizeEvt : public IEventCRTP<WindowResizeEvt>
{
    WindowResizeEvt() {}
    WindowResizeEvt(uint32_t xIn, uint32_t yIn) : x{xIn}, y{yIn} {}
    uint32_t x{0}, y{0};
};
} // namespace src::elementcomposable