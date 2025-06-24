#pragma once

#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <type_traits>
#include <typeindex>

#include "src/ElementComposable/IEvent.hpp"
#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/UIElements/LayoutAttribs.hpp"
#include "src/UIElements/VisualAttribs.hpp"
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
{
public:
    UIBase(const std::string& name, const std::type_index type);
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

    auto getName() -> std::string;
    auto getId() -> uint32_t;
    auto getTypeId() -> uint32_t;
    auto getLayoutRef() -> LayoutAttribs&;
    auto getVisualRef() -> VisualAttribs&;

protected:
    auto renderNext(const glm::mat4& projection) -> void;
    auto layoutNext() -> void;
    auto eventNext(const elementcomposable::IEvent& evt) -> void;
    virtual auto render(const glm::mat4& projection) -> void;
    virtual auto layout() -> void;
    virtual auto event(const elementcomposable::IEvent& evt) -> void;

private:
    template<UIBaseDerived T>
    auto addInternal(T&& element) -> bool;

    template<UIBaseDerived T>
    auto removeInternal(T&& element) -> bool;

    static auto demangleName(const char* name) -> std::string;

protected:
    uint32_t id_;
    uint32_t typeId_;
    std::type_index typeInfo_;
    std::string name_;
    utils::Logger log_;
    resourceloaders::Mesh mesh_;
    resourceloaders::Shader shader_;
    LayoutAttribs layoutAttr_;
    VisualAttribs visualAttr_;

    bool isParented_;
    uint32_t depth_{0};
    UIBaseWPtr parent_;
    UIBasePtrVec elements_;
};
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