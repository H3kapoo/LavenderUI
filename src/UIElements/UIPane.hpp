#pragma once

#include "src/ElementComposable/LayoutAttribs.hpp"
#include "src/ElementComposable/TextAttribs.hpp"
#include "src/ElementComposable/VisualAttribs.hpp"
#include "src/ResourceLoaders/Font.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UISlider.hpp"
#include <memory>

namespace src::uielements
{
using namespace elementcomposable;

class UIPane : public UIBaseCPRT<UIPane>
{
public:
    UIPane();
    ~UIPane() = default;
    UIPane(const UIPane&) = delete;
    UIPane(UIBase&&) = delete;
    UIPane& operator=(const UIPane&) = delete;
    UIPane& operator=(UIPane&&) = delete;

    auto isVerticalOverflow() const -> bool;
    auto isHorizontalOverflow() const -> bool;
    auto getHorizontalSlider() const -> UISliderWPtr;
    auto getVerticalSlider() const -> UISliderWPtr;

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(framestate::FrameStatePtr& state) -> void override;

protected:
    UISliderPtr hSlider_;
    UISliderPtr vSlider_;
};
using UIPanePtr = std::shared_ptr<UIPane>;
using UIPaneWPtr = std::weak_ptr<UIPane>;
} // namespace src::uielements
