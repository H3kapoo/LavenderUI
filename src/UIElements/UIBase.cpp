#include "UIBase.hpp"

#include <algorithm>
#include <cxxabi.h>

#include "src/ElementComposable/IEvent.hpp"
#include "src/ElementComposable/PropsBase.hpp"
#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/MeshLoader.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/ResourceLoaders/ShaderLoader.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"
#include "src/WindowManagement/NativeWindow.hpp"

namespace src::uielements
{
UIBase::UIBase(const std::type_index& typeIndex)
    : id_(utils::genId())
    , log_("{}/{}", demangleName(typeIndex.name()), id_)
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
    element->parent_ = weak_from_this();
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
    element->isParented_ = false;
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

auto UIBase::remove(const std::function<bool(const UIBasePtr&)>& pred) -> uint32_t
{
    return std::erase_if(elements_,
        [this, pred](const UIBasePtr& e){ if (pred(e)) { return removeInternal(e); }; return false; });
}

auto UIBase::remove(const UIBasePtr& element) -> bool
{
    return remove([&element](auto&& e) { return e->id_ == element->id_; });
}

auto UIBase::remove(const UIBasePtrVec& elements) -> void
{
    std::ranges::for_each(elements, [this](const UIBasePtr& e){ remove(e); });
}

auto UIBase::remove(UIBasePtrVec&& elements) -> void
{
    std::ranges::for_each(std::move(elements), [this](const UIBasePtr& e){ remove(e); });
}

auto UIBase::renderNext(const glm::mat4& projection) -> void
{
    renderNextExcept(projection, [](auto&){ return false; });
}

auto UIBase::renderNextExcept(const glm::mat4& projection,
        const std::function<bool(const UIBasePtr&)> filterFunc) -> void
{
    std::ranges::for_each(elements_, [this, &projection, &filterFunc](const auto& e)
    {
        if (filterFunc(e)) { return; }

        renderNextSingle(projection, e);
    });
}

auto UIBase::renderNextSingle(const glm::mat4& projection, const UIBasePtr& e) -> void
{
    //TODO: Do not render nodes that aint visible
    if (!e || !e->isParented()) { return; }

    const auto& viewPos = e->getViewPos();
    const auto& viewScale = e->getViewScale();
    windowmanagement::NativeWindow::updateScissors(
        {
            viewPos.x,
            std::floor((-2.0f / projection[1][1])) - viewPos.y - viewScale.y,
            viewScale.x,
            viewScale.y
        });
    e->render(projection);
}

auto UIBase::layoutNext() -> void
{
    //TODO: Do not calculate the layout for nodes that aint visible
    /* If is the root element, scissor area is the whole object area. */
    if (getIndex() == 1)
    {
        setViewPos(getComputedPos());
        setViewScale(getComputedScale());
    }

    std::ranges::for_each(elements_,
        [this](const auto& e)
        {
            /* After calculating my elements, compute how much of them is still visible inside of the parent. */
            e->computeViewBox(*this);
            /* Index is used for layer rendering order. Can be custom. */
            if (!e->isCustomIndex())
            {
                e->setIndex(getIndex() + 1);
            }

            /* Depth is used mostly for printing. */
            e->depth_ = depth_ + 1;
            e->layout();
        });
}

auto UIBase::eventNext(state::UIWindowStatePtr& state) -> void
{
    /* An event might have the effect of adding/removing elements of this object and thus
    invalidating elements_'s interators. If this happens, reset the iterator and process
    the event from the beginning. */
    auto size = elements_.size();
    for (auto it = elements_.begin(); it != elements_.end();)
    {
        (*it)->event(state);
        if (elements_.size() != size)
        {
            it = elements_.begin();
            size = elements_.size();
        }
        else { ++it; }
    }
}

auto UIBase::render(const glm::mat4& projection) -> void
{
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", getTransform());
    shader_.uploadVec4f("uColor", getColor());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

auto UIBase::layout() -> void
{
    log_.warn("{} element has no layout behavior overriden!", demangleName(getTypeInfo().name()));
}

auto UIBase::event(state::UIWindowStatePtr& state) -> void
{
    /* Determine in the scan pass who's the hovered element. */
    if (state->currentEventId == elementcomposable::MouseMoveScanEvt::eventId)
    {
        /* Due to the way we are processing events, we need to ensure that the user's input will
        go to the highest index element. */
        if (getIndex() > state->hoveredZIndex && isPointInsideView(state->mousePos))
        {
            state->hoveredId = id_;
            state->hoveredZIndex = getIndex();
        }
    }
}

auto UIBase::setCustomTagId(const uint32_t id) -> void { customTagid_ = id; }

auto UIBase::isParented() -> bool { return isParented_; }

auto UIBase::getCustomTagId() -> uint32_t { return customTagid_; }

auto UIBase::getId() -> uint32_t { return id_; }

auto UIBase::getElements() -> UIBasePtrVec& { return elements_; }

auto UIBase::demangleName(const char* name) -> std::string
{
    /* Works on linux+gcc for now, not sure about windows. */
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
    /* Note: Printing before the first layoutNext() will print elements with an incorrect number of tabs. */
    using namespace std::chrono;
    zoned_time nowLocal{current_zone(), time_point_cast<milliseconds>(system_clock::now())};

    out << std::format("[{:%F %T}]{}[DBG] ", nowLocal, "\033[38;2;150;150;150m");
    out << std::format("{:{}}|-- {}[Id:{} L:{}]",
        "", obj->depth_ * 2,
        UIBase::demangleName(obj->getTypeInfo().name()),
        obj->id_, obj->getIndex());
    out << "\033[m";
    std::ranges::for_each(obj->elements_, [&out](const UIBasePtr& o){ out << "\n" << o; });
    return out;
}
} // namespace src::uielements
