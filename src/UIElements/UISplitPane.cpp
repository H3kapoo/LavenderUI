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
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    renderNext(projection);
}

auto UISplitPane::layout() -> void
{
    const auto& calculator = BasicCalculator::get();

    using namespace layoutcalculator;
    BasicCalculator::get().calcSplitPaneElements(this);
    // BasicCalculator::get().calcElementsScale(this);
    // BasicCalculator::get().calcElementsPos(this);

    layoutNext();
}

auto UISplitPane::event(state::UIWindowStatePtr& state) -> void
{
    /* Let the base do the generic stuff like mouse move pre-pass. */
    UIBase::event(state);

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
        log_.debug("id {}", i);
        const uint32_t lPaneIdx = i * 2 + 0;
        const uint32_t handleIdx = i * 2 + 1;
        const uint32_t rPaneIdx = i * 2 + 2;

        utils::as<UIButton>(elements_[handleIdx])
            ->listenTo<MouseDragEvt>([this, handleIdx, lPaneIdx, rPaneIdx](const auto& e)
                {
                    const int32_t offset = prevMousePos.x == -1 ? 0 : e.x - prevMousePos.x;
                    prevMousePos.x = e.x;

                    {
                        auto scale = elements_[lPaneIdx]->getScale();
                        scale.x.val += offset / getComputedScale().x;
                        elements_[lPaneIdx]->setScale(scale);
                    }

                    {
                        auto scale = elements_[rPaneIdx]->getScale();
                        scale.x.val -= offset / getComputedScale().x;
                        elements_[rPaneIdx]->setScale(scale);
                    }
                })
            .listenTo<MouseButtonEvt>(
                [this](const auto& e)
                {
                    using namespace windowmanagement;
                    if (e.btn == Input::LEFT && e.action == Input::RELEASE)
                    {
                        log_.debug("RELEASED");
                        prevMousePos.x = -1;
                    }
                });
    }
    /*
        P H P H P
        0 1 2 3 4
    */
}

auto UISplitPane::getPaneIdx(const uint32_t idx) -> UIPaneWPtr
{
    // return panes_.at(0);
    return std::weak_ptr<UIPane>();
}
} // namespace src::uielements
