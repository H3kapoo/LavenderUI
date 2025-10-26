#include "UISplitPane.hpp"

#include "src/ElementComposable/IEvent.hpp"
#include "src/ElementComposable/LayoutBase.hpp"
#include "src/LayoutCalculator/BasicCalculator.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIPane.hpp"
#include "src/Utils/Misc.hpp"
#include "src/WindowManagement/Input.hpp"

namespace src::uielements
{
using namespace layoutcalculator;

UISplitPane::UISplitPane() : UIBase(getTypeInfo())
{}

auto UISplitPane::render(const glm::mat4& projection) -> void
{
    /* Draw base */
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", getTransform());
    shader_.uploadVec4f("uColor", getColor());
    shader_.uploadVec2f("uResolution", getComputedScale());
    shader_.uploadVec4f("uBorderSize", getBorder());
    shader_.uploadVec4f("uBorderRadii", getBorderRadius());
    shader_.uploadVec4f("uBorderColor", getBorderColor());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    renderNext(projection);
}

auto UISplitPane::layout() -> void
{
    using namespace layoutcalculator;

    const auto& calculator = BasicCalculator::get();
    calculator.calculateSplitPaneElements(this, draggedHandleId_, mousePos_);

    layoutNext();
}

auto UISplitPane::event(state::UIWindowStatePtr& state) -> void
{
    /* Let the base do the generic stuff like mouse move pre-pass. */
    UIBase::event(state);

    if (state->currentEventId == MouseMoveEvt::eventId)
    {
        mousePos_ = state->mousePos;
    }

    eventNext(state);

    /* Only update the cursor after all the children have processed the event. */
    if (wantedCursor_.has_value())
    {
        state->wantedCursorType = wantedCursor_;
        wantedCursor_.reset();
    }
}

auto UISplitPane::createPane(const float relativeSpace, const glm::ivec2& minMax) -> UIPaneWPtr
{
    return create<UIPane>(relativeSpace, minMax);
}

auto UISplitPane::createSubsplit(const float relativeSpace, const glm::ivec2& minMax) -> UISplitPaneWPtr
{
    return create<UISplitPane>(relativeSpace, minMax);
}

template<UISplitPaneElement T>
auto UISplitPane::create(const float relativeSpace, const glm::ivec2& minMax) -> std::weak_ptr<T>
{
    std::shared_ptr<T> uiElement = utils::make<T>();
    uiElement->setColor(utils::randomRGB()); // rm later

    if (layoutType_ == LayoutBase::Type::HORIZONTAL)
    {
        uiElement->setScale({{relativeSpace, ScaleType::REL}, 1.0_rel});
        uiElement->setMinScale({minMax.x, 99999});
        uiElement->setMaxScale({minMax.y, 99999});
    }
    else if (layoutType_ == LayoutBase::Type::VERTICAL)
    {
        uiElement->setScale({1.0_rel, {relativeSpace, ScaleType::REL}});
        uiElement->setMinScale({99999, minMax.x});
        uiElement->setMaxScale({99999, minMax.y});
    }

    /* No need for a handle just for one uiElement. */
    if (elements_.empty())
    {
        add(uiElement);
        return uiElement;
    }

    UIButtonPtr handle = utils::make<UIButton>();
    handle->setText(std::to_string(elements_.size()));
    handle->setColor(utils::hexToVec4("#757575ff")); // rm later

    layoutType_ == LayoutBase::Type::HORIZONTAL
        ? handle->setScale({6_px, 1.0_rel})
        : handle->setScale({1.0_rel, 6_px});

    add(handle);
    add(uiElement);

    const uint32_t handleIdx = elements_.size() - 2;
    handle->listenTo<MouseDragEvt>([this, handleIdx](const auto&)
            {
                draggedHandleId_ = handleIdx;
            })
        .template listenTo<MouseLeftReleaseEvt>(
            [this](const auto&)
            {
                draggedHandleId_ = 0;
                wantedCursor_ = Input::Cursor::ARROW;
            })
        .template listenTo<MouseEnterEvt>(
            [this](const auto&)
            {
                wantedCursor_ = layoutType_ == LayoutBase::Type::HORIZONTAL
                    ? Input::Cursor::HRESIZE : Input::Cursor::VRESIZE;
            })
        .template listenTo<MouseExitEvt>(
            [this](const auto&)
            {
                if (draggedHandleId_) { return; }
                wantedCursor_ = Input::Cursor::ARROW;
            });

    return uiElement;
}

auto UISplitPane::getPaneIdx(const uint32_t idx) -> UIPaneWPtr
{
    //TODO: Add constraints
    return utils::as<UIPane>(elements_.at(idx * 2));
}

auto UISplitPane::getHandleIdx(const uint32_t idx) -> UIButtonWPtr
{
    //TODO: Add constraints
    return utils::as<UIButton>(elements_.at(idx * 2 + 1));
}
} // namespace src::uielements
