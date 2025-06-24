#include "UIBase.hpp"

#include <algorithm>
#include <cxxabi.h>

#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/MeshLoader.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/ResourceLoaders/ShaderLoader.hpp"
#include "src/UIElements/VisualAttribs.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"
#include "src/WindowManagement/NativeWindow.hpp"

namespace src::uielements
{
UIBase::UIBase(const std::string& name, const std::type_index type)
    : id_(utils::genId())
    , typeId_(type.hash_code())
    , typeInfo_(type)
    , name_(name)
    , log_("{}/{}", demangleName(typeInfo_.name()), id_)
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
        log_.warn("Node '{}/{}' already has a parent set!", element->name_, element->id_);
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
        layoutAttr_.viewPos = layoutAttr_.pos;
        layoutAttr_.viewScale = layoutAttr_.scale;
    }

    // compute view pos/scale here for this node then call layout for the children
    std::ranges::for_each(elements_,
        [this](const auto& e)
        {
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

auto UIBase::eventNext(const elementcomposable::IEvent& evt) -> void
{
    std::ranges::for_each(elements_, [&evt](const auto& e){ e->event(evt); });
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
}

auto UIBase::event(const elementcomposable::IEvent& evt) -> void
{
    // do default event handling stuff
}

auto operator<<(std::ostream& out, const UIBasePtr& obj) -> std::ostream&
{
    using namespace std::chrono;
    zoned_time nowLocal{current_zone(), time_point_cast<milliseconds>(system_clock::now())};

    out << std::format("[{:%F %T}]{}[DBG] ", nowLocal, "\033[38;2;150;150;150m");
    out << std::format("{:{}}|-- {}[Id:{} L:{}]({})",
        "", obj->depth_ * 2, UIBase::demangleName(obj->typeInfo_.name()),
        obj->id_, obj->layoutAttr_.index, obj->name_);
    out << "\033[m";
    std::ranges::for_each(obj->elements_, [&out](const UIBasePtr& o){ out << "\n" << o; });
    return out;
}

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

auto UIBase::getName() -> std::string { return name_; }

auto UIBase::getId() -> uint32_t { return id_; }

auto UIBase::getTypeId() -> uint32_t { return typeId_; }

auto UIBase::getLayoutRef() -> LayoutAttribs& { return layoutAttr_; }

auto UIBase::getVisualRef() -> VisualAttribs& { return visualAttr_; }
} // namespace src::uielements
