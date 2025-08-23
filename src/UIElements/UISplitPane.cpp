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
    const auto& handlesSize = calculator.calculateSplitPaneElements(this);

    layoutNext();

    if (!draggedHandleId_) { return; }
    adjustPane(handlesSize, draggedHandleId_);
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
}

auto UISplitPane::createPanes(const std::vector<float> startFractions) -> void
{
    if (const auto size = startFractions.size(); size < 2)
    {
        log_.warn("Too few elements for split pane: {}", size);
        return;
    }

    const auto size = startFractions.size();
    for (uint32_t i = 0; i < size; ++i)
    {
        UIPanePtr pane = utils::make<UIPane>();
        pane->setColor(utils::randomRGB());
        if (layoutType_ == LayoutBase::Type::HORIZONTAL)
        {
            pane->setScale({{startFractions[i], ScaleType::REL}, 1.0_rel});
        }
        else if (layoutType_ == LayoutBase::Type::VERTICAL)
        {
            pane->setScale({1.0_rel, {startFractions[i], ScaleType::REL}});
        }
        add(std::move(pane));

        if (i + 1 < size)
        {
            UIButtonPtr handle = utils::make<UIButton>();
            handle->setColor(utils::hexToVec4("#757575ff"));
            handle->setText(std::to_string(i));
            if (layoutType_ == LayoutBase::Type::HORIZONTAL)
            {
                handle->setScale({10_px, 1.0_rel});
            }
            else if (layoutType_ == LayoutBase::Type::VERTICAL)
            {
                handle->setScale({1.0_rel, 10_px});
            }

            add(std::move(handle));
        }
    }

    for (uint32_t i = 0; i < size - 1; ++i)
    {
        const uint32_t handleIdx = i * 2 + 1;

        utils::as<UIButton>(elements_[handleIdx])
            ->listenTo<MouseDragEvt>([this, handleIdx](const auto&)
                {
                    draggedHandleId_ = handleIdx;
                })
            .listenTo<MouseLeftReleaseEvt>(
                [this](const auto&)
                {
                    draggedHandleId_ = 0;
                });
    }
}

auto UISplitPane::adjustPane(const glm::vec2 handlesOccupiedSpace,
    const uint32_t handleIdx) -> void
{
    /* Calculate difference between the current mouse position and the handle's center. */
    const auto handleCenter = elements_[draggedHandleId_]->getComputedPos()
        + elements_[draggedHandleId_]->getComputedScale() * 0.5f;
    const auto mouseDiff = mousePos_ - handleCenter;

    const uint32_t lPaneIdx = handleIdx - 1;
    const uint32_t rPaneIdx = handleIdx + 1;

    const auto& contentScale = getContentBoxScale() - handlesOccupiedSpace;

    glm::vec2 wantedOffsetRel = mouseDiff / contentScale;

    const glm::vec2 lpMinScaleRel = elements_[lPaneIdx]->getMinScale() / contentScale;
    const glm::vec2 lpMaxScaleRel = elements_[lPaneIdx]->getMaxScale() / contentScale;
    const glm::vec2 rpMinScaleRel = elements_[rPaneIdx]->getMinScale() / contentScale;
    const glm::vec2 rpMaxScaleRel = elements_[rPaneIdx]->getMaxScale() / contentScale;

    if (layoutType_ == LayoutBase::Type::HORIZONTAL)
    {
        // assumption: only one IF can be true at any given time
        if (elements_[lPaneIdx]->getScale().x.val + wantedOffsetRel.x < lpMinScaleRel.x)
        {
            wantedOffsetRel.x = lpMinScaleRel.x - elements_[lPaneIdx]->getScale().x.val;
        }

        if (elements_[rPaneIdx]->getScale().x.val - wantedOffsetRel.x < rpMinScaleRel.x)
        {
            wantedOffsetRel.x = (elements_[rPaneIdx]->getScale().x.val - rpMinScaleRel.x);
        }

        if (elements_[lPaneIdx]->getScale().x.val + wantedOffsetRel.x > lpMaxScaleRel.x)
        {
            wantedOffsetRel.x = lpMaxScaleRel.x - elements_[lPaneIdx]->getScale().x.val;
        }

        if (elements_[rPaneIdx]->getScale().x.val - wantedOffsetRel.x > rpMaxScaleRel.x)
        {
            wantedOffsetRel.x =  elements_[rPaneIdx]->getScale().x.val - rpMaxScaleRel.x;
        }

        /* Apply the relative offsets. */
        {
            auto scale = elements_[lPaneIdx]->getScale();
            scale.x.val += wantedOffsetRel.x;
            elements_[lPaneIdx]->setScale(scale);
        }

        {
            auto scale = elements_[rPaneIdx]->getScale();
            scale.x.val -= wantedOffsetRel.x;
            elements_[rPaneIdx]->setScale(scale);
        }
    }
    else if (layoutType_ == LayoutBase::Type::VERTICAL)
    {
        // assumption: only one IF can be true at any given time
        if (elements_[lPaneIdx]->getScale().y.val + wantedOffsetRel.y < lpMinScaleRel.y)
        {
            wantedOffsetRel.y = lpMinScaleRel.y - elements_[lPaneIdx]->getScale().y.val;
        }

        if (elements_[rPaneIdx]->getScale().y.val - wantedOffsetRel.y < rpMinScaleRel.y)
        {
            wantedOffsetRel.y = (elements_[rPaneIdx]->getScale().y.val - rpMinScaleRel.y);
        }

        if (elements_[lPaneIdx]->getScale().y.val + wantedOffsetRel.y > lpMaxScaleRel.y)
        {
            wantedOffsetRel.y = lpMaxScaleRel.y - elements_[lPaneIdx]->getScale().y.val;
        }

        if (elements_[rPaneIdx]->getScale().y.val - wantedOffsetRel.y > rpMaxScaleRel.y)
        {
            wantedOffsetRel.y =  elements_[rPaneIdx]->getScale().y.val - rpMaxScaleRel.y;
        }

        /* Apply the relative offsets. */
        {
            auto scale = elements_[lPaneIdx]->getScale();
            scale.y.val += wantedOffsetRel.y;
            elements_[lPaneIdx]->setScale(scale);
        }

        {
            auto scale = elements_[rPaneIdx]->getScale();
            scale.y.val -= wantedOffsetRel.y;
            elements_[rPaneIdx]->setScale(scale);
        }
    }
}

auto UISplitPane::getPaneIdx(const uint32_t idx) -> UIPaneWPtr
{
    return utils::as<UIPane>(elements_.at(idx * 2));
}

auto UISplitPane::getHandleIdx(const uint32_t idx) -> UIButtonWPtr
{
    return utils::as<UIButton>(elements_.at(idx * 2 + 1));
}
} // namespace src::uielements
