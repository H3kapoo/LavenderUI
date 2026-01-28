#pragma once

#include "include/LavenderUI/Core/EventHandler/IEvent.hpp"

namespace lav::core
{
struct MouseButtonEvt : public IEventCRTP<MouseButtonEvt>
{
    MouseButtonEvt() {}
    MouseButtonEvt(lav::Mouse btnIn, lav::Action actionIn) : btn{btnIn}, action{actionIn} {}
    lav::Mouse btn{0};
    lav::Action action{0};
};

} // namespace lav::core