#pragma once

#include <LavenderUI/Node/UIBase.hpp>
#include <LavenderUI/Node/UISlider.hpp>

namespace lav::node
{
/**
    @brief
    Class used to represent the Scroll Bar object of a scrollable area.
    Inherits all the properties of UISlider as they are logically equivalent.
 */
class UIScroll : public UISlider
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIScroll, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UIScroll);

private:
    auto onRender(const glm::mat4& projection) -> void override;
    auto onLayout() -> void override;
    auto onEvent(core::UIStatePtr& state) -> void override;

public:
    static uint32_t scrollIndexOffset /** @brief Scroll bars need to start at a higher z index */;
};
using UIScrollPtr = std::shared_ptr<UIScroll>;
using UIScrollWPtr = std::weak_ptr<UIScroll>;
} // namespace lav::node
