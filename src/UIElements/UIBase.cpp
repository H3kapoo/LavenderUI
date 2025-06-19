#include "UIBase.hpp"
#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/MeshLoader.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/ResourceLoaders/ShaderLoader.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"
#include <algorithm>
#include <vector>

namespace src::uielements
{
UIBase::UIBase(const std::string& name)
    : id_(utils::genId())
    , name_(name)
    , log_("UIBase/{}", name, id_)
    , mesh_(resourceloaders::MeshLoader::get().loadQuad())
    , shader_(resourceloaders::ShaderLoader::get().load(
        "assets/shaders/basicVert.glsl", "assets/shaders/basicFrag.glsl"))
    , isParented_(false)
    , level_(0)
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
    element->level_ = level_ + 1;
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

auto UIBase::render(const glm::mat4& projection) -> void
{
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layout_.getTransform());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

auto operator<<(std::ostream& out, const UIBasePtr& obj) -> std::ostream&
{
    using namespace std::chrono;
    zoned_time nowLocal{current_zone(), time_point_cast<milliseconds>(system_clock::now())};

    out << std::format("[{:%F %T}]{}[DBG] ", nowLocal, "\033[38;2;150;150;150m");
    out << std::format("{:{}}|-- UIBase[Id:{} L:{}]({})",
        "", obj->level_*2, obj->id_, obj->level_, obj->name_);
    out << "\033[m";
    std::ranges::for_each(obj->elements_, [&out](const UIBasePtr& o){ out << "\n" << o; });
    return out;
}

auto UIBase::getId() -> uint64_t { return id_; }

auto UIBase::getName() -> std::string { return name_; }
} // namespace src::uielements
