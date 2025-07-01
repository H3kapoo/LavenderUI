#include "UIBase.hpp"

#include <algorithm>
#include <cxxabi.h>

#include "src/ElementEvents/IEvent.hpp"
#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/MeshLoader.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/ResourceLoaders/ShaderLoader.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"
#include "src/WindowManagement/NativeWindow.hpp"

namespace src::uielements
{
UIBase::UIBase(const std::type_index type)
    : id_(utils::genId())
    , log_("{}/{}", demangleName(type.name()), id_)
    , mesh_(resourceloaders::MeshLoader::get().loadQuad())
    , shader_(resourceloaders::ShaderLoader::get().load(
        "assets/shaders/basicVert.glsl", "assets/shaders/basicFrag.glsl"))
    , isParented_(false)
{}

template<UIBaseDerived T>
auto UIBase::addInternal(T&& element) -> bool
{
    if (!element)
    {
        log_.warn("Can't parent null or moved from node!");
        return false;
    }

    if (element->id_ == id_)
    {
        log_.warn("Cannot parent me to myself!");
        return false;
    }
    
    if (element->isParented_)
    {
        log_.warn("Node '{}' already has a parent set!", element->id_);
        return false;
    }
    element->isParented_ = true;
    element->parent_ = shared_from_this();
    elements_.emplace_back(std::forward<T>(element));
    return true;
}

template<UIBaseDerived T>
auto UIBase::removeInternal(T&& element) -> bool
{
    if (!element)
    {
        log_.warn("Can't remove null or moved from node!");
        return false;
    }
    element->parent_.reset();
    std::erase(elements_, element);
    return true;
}

auto UIBase::add(const UIBasePtr& element) -> bool
{
    return addInternal(element);
}

auto UIBase::add(UIBasePtr&& element) -> bool
{
    return addInternal(std::move(element));
}

auto UIBase::add(const UIBasePtrVec& elements) -> void
{
    std::ranges::for_each(elements, [this](const UIBasePtr& e){ addInternal(e); });
}

auto UIBase::add(UIBasePtrVec&& elements) -> void
{
    std::ranges::for_each(elements, [this](UIBasePtr& e){ addInternal(std::move(e)); });
}

auto UIBase::remove(const std::function<bool(const UIBasePtr&)>& pred) -> void
{
    std::ranges::for_each(elements_, [this, pred](const UIBasePtr& e){ if (pred(e)) removeInternal(e); });
}

auto UIBase::remove(const UIBasePtr& element) -> bool
{
    return removeInternal(element);
}

auto UIBase::remove(const UIBasePtrVec& elements) -> void
{
    std::ranges::for_each(elements, [this](const UIBasePtr& e){ removeInternal(e); });
}

auto UIBase::remove(UIBasePtrVec&& elements) -> void
{
    std::ranges::for_each(elements, [this](UIBasePtr& e){ removeInternal(std::move(e)); });
}

auto UIBase::renderNext(const glm::mat4& projection) -> void
{
    std::ranges::for_each(elements_, [&projection](const auto& e)
    {
        windowmanagement::NativeWindow::updateScissors(
        {
            e->layoutAttr_.viewPos.x,
            std::floor((-2.0f / projection[1][1])) - e->layoutAttr_.viewPos.y - e->layoutAttr_.viewScale.y,
            e->layoutAttr_.viewScale.x,
            e->layoutAttr_.viewScale.y
        });
        e->render(projection);
    });
}

auto UIBase::layoutNext() -> void
{
    /* If is the root element, scissor area is the whole object area. */
    if (layoutAttr_.index == 1)
    {
        layoutAttr_.viewPos = layoutAttr_.cPos;
        layoutAttr_.viewScale = layoutAttr_.cScale;
    }

    std::ranges::for_each(elements_,
        [this](const auto& e)
        {
            /* After calculating my elements, compute how much of them is still visible inside of the parent. */
            e->layoutAttr_.computeViewBox(layoutAttr_);
            /* Index is used for layer rendering order. Can be custom. */
            if (!e->layoutAttr_.isCustomLevel)
            {
                e->layoutAttr_.index = layoutAttr_.index + 1;
            }

            /* Depth is used mostly for printing. */
            e->depth_ = depth_ + 1;
            e->layout();
        });
}

auto UIBase::eventNext(framestate::FrameStatePtr& state, const elementevents::IEvent& evt) -> void
{
    std::ranges::for_each(elements_, [&state, &evt](const auto& e){ e->event(state, evt); });
}

auto UIBase::render(const glm::mat4& projection) -> void
{
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutAttr_.getTransform());
    shader_.uploadVec4f("uColor", visualAttr_.color);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

auto UIBase::layout() -> void
{
    // do default layout stuff
    log_.warn("{} element has no layout behavior overriden!", demangleName(getTypeInfo().name()));
}

auto UIBase::event(framestate::FrameStatePtr& state, const elementevents::IEvent& evt) -> void
{
    using namespace elementevents;
    const auto type = evt.getType();

    /* Determine in the scan pass who's the hovered element. */
    if (type == MouseMoveScanEvt::eventId)
    {
        if (layoutAttr_.isPointInsideView(state->mousePos))
        {
            state->hoveredId = id_;
        }
    }
}

auto UIBase::getId() -> uint32_t { return id_; }

auto UIBase::getLayout() -> elementcomposable::LayoutAttribs& { return layoutAttr_; }

auto UIBase::getVisual() -> elementcomposable::VisualAttribs& { return visualAttr_; }

auto UIBase::getEvents() -> elementevents::EventManager& { return eventManager_; }

auto UIBase::getElements() -> UIBasePtrVec& { return elements_; }

auto UIBase::demangleName(const char* name) -> std::string
{
    /* Works on linux for now, not sure about windows. */

    int status = 0;
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, nullptr, nullptr, &status),
        std::free
    };
    if (status == 0)
    {
        std::string s{res.get()};
        return s.substr(s.find_last_of(":") + 1);
    }
    return (status == 0) ? res.get() : name;
}

auto operator<<(std::ostream& out, const UIBasePtr& obj) -> std::ostream&
{
    using namespace std::chrono;
    zoned_time nowLocal{current_zone(), time_point_cast<milliseconds>(system_clock::now())};

    out << std::format("[{:%F %T}]{}[DBG] ", nowLocal, "\033[38;2;150;150;150m");
    out << std::format("{:{}}|-- {}[Id:{} L:{}]",
        "", obj->depth_ * 2,
        UIBase::demangleName(obj->getTypeInfo().name()),
        obj->id_, obj->layoutAttr_.index);
    out << "\033[m";
    std::ranges::for_each(obj->elements_, [&out](const UIBasePtr& o){ out << "\n" << o; });
    return out;
}
} // namespace src::uielements
