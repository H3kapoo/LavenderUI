#pragma once

#include "include/LavenderUI/Core/EventHandler/IEvent.hpp"

namespace lav::core
{
struct WindowFocusEvt : public IEventCRTP<WindowFocusEvt>
{};

} // namespace lav::core