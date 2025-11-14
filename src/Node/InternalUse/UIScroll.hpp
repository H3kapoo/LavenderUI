#pragma once

#include "src/Node/UIBase.hpp"
#include "src/Node/UISlider.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::node
{
class UIScroll : public UISlider
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIScroll, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UIScroll);

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(node::UIStatePtr& state) -> void override;

public:
    static uint32_t scrollIndexOffset /** @brief Scroll bars need to start at a higher z index, */;
};
using UIScrollPtr = std::shared_ptr<UIScroll>;
using UIScrollWPtr = std::weak_ptr<UIScroll>;
} // namespace lav::node
