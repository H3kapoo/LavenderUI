#pragma once

#include "include/LavenderUI/Core/EventHandler/IEvent.hpp"

namespace lav::core
{
struct MouseDragEvt : public IEventCRTP<MouseDragEvt>
{
    MouseDragEvt() {}
    MouseDragEvt(int32_t xIn, int32_t yIn) : x{xIn}, y{yIn} {}
    int32_t x{0}, y{0};
};
} // namespace lav::core