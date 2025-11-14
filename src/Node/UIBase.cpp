#include "UIBase.hpp"

#include <algorithm>
#include <cxxabi.h>

#include "src/Core/ResourceHandler/ShaderLoader.hpp"
#include "src/Core/ResourceHandler/MeshLoader.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::node
{
UIBase::UIBase(UIBaseInitData&& initData)
    : nameTag_(initData.name)
    , id_(utils::genId())
    , log_("{}/{}", initData.name, id_)
    , mesh_(core::MeshLoader::get().loadQuad())
    , shader_(core::ShaderLoader::get().load(
        "assets/shaders" / initData.vertexShader,
        "assets/shaders" / initData.fragmentShader))
    , baseColor_{utils::hexToVec4("#ffffffff")}
    , borderColor_{utils::hexToVec4("#979797ff")}
    , depth_(0)
    , isParented_(false)
    , isIgnoringEvents_(false)
{}

auto UIBase::add(const UIBasePtr& element) -> bool
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
    elements_.emplace_back(element);
    return true;
}

auto UIBase::add(const UIBasePtrVec& elements) -> void
{
    std::ranges::for_each(elements, [this](const UIBasePtr& e){ add(e); });
}

auto UIBase::remove(const std::function<bool(const UIBasePtr&)>& pred) -> uint32_t
{
    return std::erase_if(elements_,
        [this, pred](const UIBasePtr& e)
        {
            if (pred(e))
            {
                if (!e)
                {
                    log_.warn("Can't remove null or moved from node!");
                    return false;
                }
                e->parent_.reset();
                e->isParented_ = false;
                return true;
            };
            return false;
        });
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
    //TODO: May be more efficient to do it with std::erase_if so it's only one pass.
    std::ranges::for_each(std::move(elements), [this](const UIBasePtr& e){ remove(e); });
}

auto UIBase::setIgnoreEvents(const bool ignore) -> void { isIgnoringEvents_ = ignore; }

auto UIBase::setColor(const glm::vec4& value) -> void { baseColor_ = value; }

auto UIBase::setBorderColor(const glm::vec4& value) -> void { borderColor_ = value; }

auto UIBase::isParented() -> bool { return isParented_; }

auto UIBase::isIgnoringEvents() -> bool { return isIgnoringEvents_; }

auto UIBase::getId() -> uint32_t { return id_; }

auto UIBase::getParent() -> UIBaseWPtr { return parent_; }

auto UIBase::getGrandParent() -> UIBaseWPtr
{
    if (const auto p = getParent().lock()) { return p->getParent(); }
    return std::weak_ptr<UIBase>();
}

auto UIBase::getElements() -> UIBasePtrVec& { return elements_; }

auto UIBase::getColor() -> glm::vec4 { return baseColor_; }

auto UIBase::getBorderColor() -> glm::vec4 { return borderColor_; }

auto UIBase::getBaseLayoutData() -> core::LayoutBase& { return layoutBase_; }

auto UIBase::getEventManager() -> core::Events& { return eventsMgr_; }

auto operator<<(std::ostream& out, const UIBasePtr& obj) -> std::ostream&
{
    /* Note: Printing before the first layoutNext() will print elements with an incorrect number of tabs. */
    using namespace std::chrono;
    zoned_time nowLocal{current_zone(), time_point_cast<milliseconds>(system_clock::now())};

    out << std::format("[{:%F %T}]{}[DBG] ", nowLocal, "\033[38;2;150;150;150m");
    out << std::format("{:{}}|-- {}[Id:{} L:{}]",
        "", obj->depth_ * 2, obj->nameTag_, obj->id_, obj->getBaseLayoutData().getZIndex());
    out << "\033[m";
    std::ranges::for_each(obj->elements_, [&out](const UIBasePtr& o){ out << "\n" << o; });
    return out;
}

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
} // namespace lav::node
