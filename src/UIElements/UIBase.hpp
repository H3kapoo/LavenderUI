#pragma once

#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <type_traits>
#include <typeindex>

#include "src/ElementComposable/LayoutAttribs.hpp"
#include "src/ElementComposable/VisualAttribs.hpp"
#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/ElementComposable/LayoutAttribs.hpp"
#include "src/ElementComposable/VisualAttribs.hpp"
#include "src/ElementEvents/EventManager.hpp"
#include "src/FrameState/FrameState.hpp"
#include "src/Utils/Logger.hpp"

namespace src::uielements
{
class UIBase;

template<typename T>
concept UIBaseDerived = 
    std::is_base_of_v<UIBase, typename std::pointer_traits<std::remove_cvref_t<T>>::element_type>;

using UIBasePtr = std::shared_ptr<UIBase>;
using UIBaseWPtr = std::weak_ptr<UIBase>;
using UIBasePtrVec = std::vector<UIBasePtr>;
class UIBase : public std::enable_shared_from_this<UIBase>
             , public elementcomposable::LayoutAttribs
{
public:
    UIBase(const std::type_index type);
    virtual ~UIBase() = default;
    UIBase(const UIBase&) = delete;
    UIBase(UIBase&&) = delete;
    UIBase& operator=(const UIBase&) = delete;
    UIBase& operator=(UIBase&&) = delete;

    auto add(const UIBasePtr& element) -> bool;
    auto add(UIBasePtr&& element) -> bool;
    auto add(const UIBasePtrVec& elements) -> void;
    auto add(UIBasePtrVec&& elements) -> void;
    auto remove(const std::function<bool(const UIBasePtr&)>& pred) -> void;
    auto remove(const UIBasePtr& element) -> bool;
    auto remove(const UIBasePtrVec& elements) -> void;
    auto remove(UIBasePtrVec&& elements) -> void;
    friend auto operator<<(std::ostream& out, const UIBasePtr&) -> std::ostream&;

    auto getId() -> uint32_t;
    auto getLayout() -> elementcomposable::LayoutAttribs&;
    auto getVisual() -> elementcomposable::VisualAttribs&;
    auto getEvents() -> elementevents::EventManager&;
    auto getElements() -> UIBasePtrVec&;

    virtual auto getTypeInfo() const -> std::type_index = 0;
    virtual auto getTypeId() const -> uint32_t = 0;

protected:
    auto renderNext(const glm::mat4& projection) -> void;
    auto layoutNext() -> void;
    auto eventNext(framestate::FrameStatePtr& state) -> void;
    virtual auto render(const glm::mat4& projection) -> void;
    virtual auto layout() -> void;
    virtual auto event(framestate::FrameStatePtr& state) -> void;

private:
    template<UIBaseDerived T>
    auto addInternal(T&& element) -> bool;

    template<UIBaseDerived T>
    auto removeInternal(T&& element) -> bool;

    static auto demangleName(const char* name) -> std::string;

protected:
    uint32_t id_;
    utils::Logger log_;
    resourceloaders::Mesh mesh_;
    resourceloaders::Shader shader_;
    elementcomposable::LayoutAttribs layoutAttr_;
    elementcomposable::VisualAttribs visualAttr_;
    elementevents::EventManager eventManager_;
    bool isParented_;
    uint32_t depth_{0};
    UIBaseWPtr parent_;
    UIBasePtrVec elements_;
};

template<typename Derived>
struct UIBaseCPRT : public UIBase
{
public:
    UIBaseCPRT() : UIBase(typeid(Derived)) {}
    virtual ~UIBaseCPRT() = default;
    auto getTypeInfo() const -> std::type_index override { return typeid(Derived); };
    auto getTypeId() const -> uint32_t override { return typeId; };

    static const uint32_t typeId;
};

template<typename Derived>
const uint32_t UIBaseCPRT<Derived>::typeId = utils::genId();

} // namespace src::uielements

template<typename T>
requires std::is_base_of_v<src::uielements::UIBase, T>
struct std::formatter<std::shared_ptr<T>> : std::formatter<std::string_view>
{
    auto format(const src::uielements::UIBasePtr& obj, format_context& ctx) const
    {
        std::ostringstream os;
        os << obj;
        return std::formatter<std::string_view>::format(std::move(os).str(), ctx);
    }
};