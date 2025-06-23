#include "UIBase.hpp"

#include <algorithm>
#include <cxxabi.h>

#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/MeshLoader.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/ResourceLoaders/ShaderLoader.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"

namespace src::uielements
{
UIBase::UIBase(const std::string& name, const std::type_index type)
    : id_(utils::genId())
    , typeId_(type.hash_code())
    , typeInfo_(type)
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
    element->parent_ = shared_from_this();
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

auto operator<<(std::ostream& out, const UIBasePtr& obj) -> std::ostream&
{
    /* Works on linux for now, not sure about windows. */
    static auto demangle = [](const char* name) -> std::string {
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
    };

    using namespace std::chrono;
    zoned_time nowLocal{current_zone(), time_point_cast<milliseconds>(system_clock::now())};

    out << std::format("[{:%F %T}]{}[DBG] ", nowLocal, "\033[38;2;150;150;150m");
    out << std::format("{:{}}|-- {}[Id:{} L:{}]({})",
        "", obj->level_*2, demangle(obj->typeInfo_.name()), obj->id_, obj->level_, obj->name_);
    out << "\033[m";
    std::ranges::for_each(obj->elements_, [&out](const UIBasePtr& o){ out << "\n" << o; });
    return out;
}

auto UIBase::getName() -> std::string { return name_; }

auto UIBase::getId() -> uint32_t { return id_; }

auto UIBase::getTypeId() -> uint32_t { return typeId_; }
} // namespace src::uielements
