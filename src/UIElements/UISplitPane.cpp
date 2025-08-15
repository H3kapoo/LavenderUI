#include "UISplitPane.hpp"

#include "src/ElementComposable/IEvent.hpp"
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
    const auto& calculator = BasicCalculator::get();

    using namespace layoutcalculator;
    BasicCalculator::get().calculateSplitPaneElements(this);

    const auto diff = mousePos_ - (elements_[1]->getComputedPos() + elements_[1]->getComputedScale() * 0.5f);
    // const auto diff2 = (mousePos_ - mouseDiff_) - (elements_[1]->getComputedPos() + elements_[1]->getComputedScale() * 0.0f);

    log_.debug("diff {}", diff);
    if (locked.leftReached)
    {
        adjustPane(diff, 1);
    }
    layoutNext();
}

auto UISplitPane::event(state::UIWindowStatePtr& state) -> void
{
    /* Let the base do the generic stuff like mouse move pre-pass. */
    UIBase::event(state);

    if (state->currentEventId == MouseMoveEvt::eventId)
    {
        mouseDiff_ = state->mouseDiff;
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
        pane->setScale({{startFractions[i], ScaleType::REL}, 1.0_rel});
        add(std::move(pane));

        if (i + 1 < size)
        {
            UIButtonPtr handle = utils::make<UIButton>();
            handle->setColor(utils::hexToVec4("#757575ff"));
            handle->setText(std::to_string(i));
            handle->setScale({20_px, 1.0_rel});

            add(std::move(handle));
        }
    }

    for (uint32_t i = 0; i < size - 1; ++i)
    {
        const uint32_t handleIdx = i * 2 + 1;

        utils::as<UIButton>(elements_[handleIdx])
            ->listenTo<MouseDragEvt>([this, handleIdx](const auto&)
                {
                    locked.leftReached = true;
                    // adjustPane(mouseDiff_, handleIdx);
                })
            .listenTo<MouseButtonEvt>(
                [this](const auto& e)
                {
                    using namespace windowmanagement;
                    if (e.btn == Input::LEFT && e.action == Input::RELEASE)
                    {
                        log_.debug("RELEASED");
                        locked.leftReached = false;
                        mousePrevPos_.x = 0;
                    }
                });
    }
}

auto UISplitPane::adjustPane(glm::ivec2 mouseDiff, const uint32_t handleIdx) -> void
{
    const uint32_t lPaneIdx = handleIdx - 1;
    const uint32_t rPaneIdx = handleIdx + 1;

    const auto& contentScale = getContentBoxScale() - glm::vec2{40, 0};

    float wantedOffsetRel = mouseDiff.x / contentScale.x;

    const glm::vec2 lpMinScaleRel = elements_[lPaneIdx]->getMinScale() / contentScale;
    const glm::vec2 lpMaxScaleRel = elements_[lPaneIdx]->getMaxScale() / contentScale;
    const glm::vec2 rpMinScaleRel = elements_[rPaneIdx]->getMinScale() / contentScale;
    const glm::vec2 rpMaxScaleRel = elements_[rPaneIdx]->getMaxScale() / contentScale;

    // assumption: only one IF can be true at any given time
    if (elements_[lPaneIdx]->getScale().x.val + wantedOffsetRel < lpMinScaleRel.x)
    {
        // log_.warn("Fucked up {}", wantedOffsetRel);
        // log_.warn("Can only take {}", lpMinScaleRel.x - elements_[lPaneIdx]->getScale().x.val);
        wantedOffsetRel = lpMinScaleRel.x - elements_[lPaneIdx]->getScale().x.val;
    }

    if (elements_[rPaneIdx]->getScale().x.val - wantedOffsetRel < rpMinScaleRel.x)
    {
        // log_.warn("Fucked up 2 {}", wantedOffsetRel);
        // log_.warn("Can only take 2 {}", (elements_[rPaneIdx]->getScale().x.val - rpMinScaleRel.x));
        wantedOffsetRel = (elements_[rPaneIdx]->getScale().x.val - rpMinScaleRel.x);
    }

    if (elements_[lPaneIdx]->getScale().x.val + wantedOffsetRel > lpMaxScaleRel.x)
    {
        log_.warn("Fucked up {}", wantedOffsetRel);
        log_.warn("Can only take {}", lpMaxScaleRel.x - elements_[lPaneIdx]->getScale().x.val);
        wantedOffsetRel = lpMaxScaleRel.x - elements_[lPaneIdx]->getScale().x.val;
    }

    if (elements_[rPaneIdx]->getScale().x.val - wantedOffsetRel > rpMaxScaleRel.x)
    {
        log_.warn("Fucked up {}", wantedOffsetRel);
        log_.warn("Can only take {}", rpMaxScaleRel.x - elements_[rPaneIdx]->getScale().x.val);
        wantedOffsetRel = rpMaxScaleRel.x - elements_[rPaneIdx]->getScale().x.val;
    }
    /* Min/Max bounds checks shall go here. */
    // if (elements_[lPaneIdx]->getScale().x.val + wantedOffsetRel >= lpMinScaleRel.x &&
    //     elements_[rPaneIdx]->getScale().x.val - wantedOffsetRel >= rpMinScaleRel.x &&
    // if(    elements_[lPaneIdx]->getScale().x.val + wantedOffsetRel <= lpMaxScaleRel.x &&
    //     elements_[rPaneIdx]->getScale().x.val - wantedOffsetRel <= rpMaxScaleRel.x
    // )
    {
        // mousePrevPos_ = mousePos;
        /* Apply the relative offsets. */
        {
            auto scale = elements_[lPaneIdx]->getScale();
            scale.x.val += wantedOffsetRel;
            elements_[lPaneIdx]->setScale(scale);
        }

        {
            auto scale = elements_[rPaneIdx]->getScale();
            scale.x.val -= wantedOffsetRel;
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
