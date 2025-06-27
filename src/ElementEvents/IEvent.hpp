#pragma once

#include "src/Utils/Misc.hpp"
#include <cstdint>

namespace src::elementevents
{
struct IEvent
{
    virtual ~IEvent() = default;
    virtual auto getType() const -> uint32_t = 0;
};

template<typename Derived>
struct IEventExtra : public IEvent
{
    virtual ~IEventExtra() = default;
    auto getType() const -> uint32_t override { return Derived::eventId; };

    static const uint32_t eventId;
};

template<typename Derived>
const uint32_t IEventExtra<Derived>::eventId = utils::genId();

struct MouseMoveScanEvt : public IEventExtra<MouseMoveScanEvt>
{};

struct MouseMoveEvt : public IEventExtra<MouseMoveEvt>
{
    MouseMoveEvt() {}
    MouseMoveEvt(uint32_t xIn, uint32_t yIn) : x{xIn}, y{yIn} {}
    uint32_t x{0}, y{0};
};

struct MouseButtonEvt : public IEventExtra<MouseButtonEvt>
{
    MouseButtonEvt() {}
    MouseButtonEvt(uint8_t btnIn, uint8_t actionIn) : btn{btnIn}, action{actionIn} {}
    uint8_t btn{0};
    uint8_t action{0};
};

struct MouseEnterEvt : public IEventExtra<MouseEnterEvt>
{
    MouseEnterEvt() {}
    MouseEnterEvt(uint32_t xIn, uint32_t yIn) : x{xIn}, y{yIn} {}
    uint32_t x{0}, y{0};
};

struct MouseExitEvt : public IEventExtra<MouseExitEvt>
{
    MouseExitEvt() {}
    MouseExitEvt(uint32_t xIn, uint32_t yIn) : x{xIn}, y{yIn} {}
    uint32_t x{0}, y{0};
};
} // namespace src::elementevents