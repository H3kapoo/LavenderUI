#pragma once

#include <LavenderUI/Node/UIButton.hpp>
#include <LavenderUI/Node/UIBase.hpp>

namespace lav::node
{
class UIViewItem : public UIBase
{
public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIViewItem, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UITreeItem);

    auto getButtonPair() -> std::pair<UIButtonPtr, UIButtonPtr>; /* No need for weak ptr, internal specific use. */

protected:
    virtual auto onRender(const glm::mat4& projection) -> void override;
    virtual auto onLayout() -> void override;
    virtual auto onEvent(core::UIStatePtr& state) -> void override;

private:
    UIButtonPtr button1_;
    UIButtonPtr button2_;
};
using UITreeItemPtr = std::shared_ptr<UIViewItem>;
using UITreeItemWPtr = std::weak_ptr<UIViewItem>;
} // namespace lav::node
