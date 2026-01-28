#pragma once

#include "include/LavenderUI/Core/EventHandler/IEvent.hpp"

namespace lav::core
{
struct WindowResizeEvt : public IEventCRTP<WindowResizeEvt>
{
    WindowResizeEvt() {}
    WindowResizeEvt(uint32_t xIn, uint32_t yIn) : x{xIn}, y{yIn} {}
    uint32_t x{0}, y{0};
};
} // namespace lav::core