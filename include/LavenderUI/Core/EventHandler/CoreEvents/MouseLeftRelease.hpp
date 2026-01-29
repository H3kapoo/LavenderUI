#pragma once

#include <LavenderUI/Core/EventHandler/IEvent.hpp>

namespace lav::core
{
struct MouseLeftReleaseEvt : public IEventCRTP<MouseLeftReleaseEvt>
{};
} // namespace lav::core