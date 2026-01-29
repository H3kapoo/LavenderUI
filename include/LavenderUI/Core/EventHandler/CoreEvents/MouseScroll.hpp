#pragma once

#include <LavenderUI/Core/EventHandler/IEvent.hpp>

namespace lav::core
{
struct MouseScrollEvt : public IEventCRTP<MouseScrollEvt>
{
    MouseScrollEvt() {}
    MouseScrollEvt(int32_t xOffsetIn, int32_t yOffsetIn) : xOffset{xOffsetIn}, yOffset{yOffsetIn} {}
    int32_t xOffset{0}, yOffset{0};
};
} // namespace lav::core