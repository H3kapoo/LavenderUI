#include "UIScroll.hpp"

#include "include/LavenderUI/Node/UIBase.hpp"
#include "include/LavenderUI/Node/UISlider.hpp"

namespace lav::node
{
uint32_t UIScroll::scrollIndexOffset = 250;

UIScroll::UIScroll(UIBaseInitData&& initData) : UISlider(std::move(initData)) 
{}

auto UIScroll::onRender(const glm::mat4& projection) -> void
{
    UISlider::onRender(projection);
}

auto UIScroll::onLayout() -> void
{
    UISlider::onLayout();
}

auto UIScroll::onEvent(core::UIStatePtr& state) -> void
{
    return UISlider::onEvent(state);
}
} // namespace lav::node
