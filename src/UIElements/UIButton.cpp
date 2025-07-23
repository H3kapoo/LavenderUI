#include "UIButton.hpp"

#include "src/ElementComposable/IEvent.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/WindowManagement/NativeWindow.hpp"
#include "src/LayoutCalculator/BasicCalculator.hpp"

namespace src::uielements
{
UIButton::UIButton() : UIBase(getTypeInfo())
{}

auto UIButton::render(const glm::mat4& projection) -> void
{
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", color_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    renderNext(projection);
}

auto UIButton::layout() -> void
{
    using namespace layoutcalculator;
    // BasicCalculator::get().calcElementsPos(shared_from_this());

    // log_.debug("Doing layout for {} {}", id_, UIButton::typeId);
    layoutNext();
}

auto UIButton::event(framestate::FrameStatePtr& state) -> void
{
    using namespace elementcomposable;
    /* Let the base do the generic stuff */
    UIBase::event(state);

    const auto eId = state->currentEventId;
    if (eId == MouseButtonEvt::eventId && state->hoveredId == id_)
    {
        MouseButtonEvt e{state->mouseButton, state->mouseAction};
        /* We can safely ignore bubbling down the tree as we found the clicked element. */
        return events_.emitEvent<MouseButtonEvt>(e);
    }
    else if (eId == MouseEnterEvt::eventId && state->hoveredId == id_)
    {
        MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        return events_.emitEvent<MouseEnterEvt>(e);
    }
    else if (eId == MouseExitEvt::eventId && state->prevHoveredId == id_)
    {
        MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        return events_.emitEvent<MouseExitEvt>(e);
    }

    eventNext(state);
}
} // namespace src::uielements
