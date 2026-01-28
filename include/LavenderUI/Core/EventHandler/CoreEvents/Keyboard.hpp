#pragma once

#include "include/LavenderUI/Core/EventHandler/IEvent.hpp"

namespace lav::core
{
struct KeyboardEvt : public IEventCRTP<KeyboardEvt>
{};
} // namespace lav::core