#pragma once

#include <memory>

#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIPane.hpp"
#include "src/UIElements/UISlider.hpp"

namespace src::uielements
{
using namespace elementcomposable;

class UITreeView : public UIPane
{
public:
    UITreeView();
    ~UITreeView() = default;
    UITreeView(const UITreeView&) = delete;
    UITreeView(UIBase&&) = delete;
    UITreeView& operator=(const UITreeView&) = delete;
    UITreeView& operator=(UITreeView&&) = delete;

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(framestate::FrameStatePtr& state) -> void override;
};
using UITreeViewPtr = std::shared_ptr<UITreeView>;
using UITreeViewWPtr = std::weak_ptr<UITreeView>;
} // namespace src::uielements
