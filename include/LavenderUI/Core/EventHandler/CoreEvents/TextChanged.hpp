#pragma once

#include <LavenderUI/Core/EventHandler/IEvent.hpp>

namespace lav::core
{
struct TextChangedEvt : public IEventCRTP<TextChangedEvt>
{
    TextChangedEvt() {}
    TextChangedEvt(const std::string& textIn) : text{textIn} {}
    std::string text;
};
} // namespace lav::core