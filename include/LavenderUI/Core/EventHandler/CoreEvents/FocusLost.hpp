#pragma once

#include <LavenderUI/Core/EventHandler/IEvent.hpp>

namespace lav::core
{
struct FocusLostEvt : public IEventCRTP<FocusLostEvt>
{};
} // namespace lav::core