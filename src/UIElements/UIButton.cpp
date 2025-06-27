#include "UIButton.hpp"

#include "src/ElementEvents/IEvent.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/WindowManagement/NativeWindow.hpp"

namespace src::uielements
{
UIButton::UIButton()
    // : UIBase(typeid(UIButton))
{}

auto UIButton::render(const glm::mat4& projection) -> void
{
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutAttr_.getTransform());
    shader_.uploadVec4f("uColor", visualAttr_.color);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    renderNext(projection);
}

auto UIButton::layout() -> void
{
    layoutNext();
}

auto UIButton::event(framestate::FrameStatePtr& state, const elementevents::IEvent& evt) -> void
{
    using namespace elementevents;
    /* Let the base do the generic stuff */
    UIBase::event(state, evt);

    const auto type = evt.getType();
    if (type == MouseMoveEvt::eventId)
    {
        // do something
    }
    else if (type == MouseButtonEvt::eventId && state->hoveredId == id_)
    {
        MouseButtonEvt e{state->mouseButton, state->mouseAction};
        /* We can safely ignore bubbling down the tree as we found the clicked element. */
        return eventManager_.emit<MouseButtonEvt>(e);
    }
    else if (type == MouseEnterEvt::eventId && state->hoveredId == id_)
    {
        MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        return eventManager_.emit<MouseEnterEvt>(e);
        // log_.debug("Mouse entered me {} ev id{}", id_, MouseEnterEvt::eventId);
    }
    else if (type == MouseExitEvt::eventId && state->prevHoveredId == id_)
    {
        MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        return eventManager_.emit<MouseExitEvt>(e);
        // log_.debug("Mouse EXIT me {} ev id{}", id_, MouseExitEvt::eventId);
    }

    eventNext(state, evt);
}
} // namespace src::uielements
