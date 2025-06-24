#include "UIButton.hpp"

#include "src/UIElements/UIBase.hpp"
#include "src/WindowManagement/NativeWindow.hpp"

namespace src::uielements
{
UIButton::UIButton(const std::string& name)
    : UIBase(name, typeid(UIButton))
{}

auto UIButton::render(const glm::mat4& projection) -> void
{
    // log_.debug("rendering..");

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
    // log_.debug("layouting..");
    // layoutAttr_.pos.z = 1;
    // layoutAttr_.pos.x = 100;
    // layoutAttr_.pos.y = 100;

    // compute scale and pos based on layout params
    // compute view pos/scale
    layoutNext();
}

auto UIButton::event(const elementcomposable::IEvent& e) -> void
{
    const auto type = e.getType();
    if (type == elementcomposable::EventType::MOUSE_MOVE)
    {
        const auto mme = static_cast<const elementcomposable::MouseMoveEvt*>(&e);
        std::println("from button: x: {}, y: {}", mme->x, mme->y);
    }
    eventNext(e);
}

} // namespace src::uielements
