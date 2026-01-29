#pragma once

#include <LavenderUI/Core/EventHandler/IEvent.hpp>

namespace lav::core
{
struct FocusGainEvt : public IEventCRTP<FocusGainEvt>
{};
} // namespace lav::core