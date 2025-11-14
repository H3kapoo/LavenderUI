#include "UIScroll.hpp"

#include "src/Node/UIBase.hpp"
#include "src/Node/UISlider.hpp"

namespace lav::node
{
uint32_t UIScroll::scrollIndexOffset = 250;

UIScroll::UIScroll(UIBaseInitData&& initData) : UISlider(std::move(initData)) 
{}

auto UIScroll::render(const glm::mat4& projection) -> void
{
    UISlider::render(projection);
}

auto UIScroll::layout() -> void
{
    UISlider::layout();
}

auto UIScroll::event(node::UIStatePtr& state) -> void
{
    UISlider::event(state);
}
} // namespace lav::node
