#pragma once

#include "include/LavenderUI/Core/EventHandler/IEvent.hpp"

namespace lav::core
{
struct SlideEvt : public IEventCRTP<SlideEvt>
{
    SlideEvt() {}
    SlideEvt(double valueIn) : value{valueIn} {}
    double value{0};
};
} // namespace lav::core