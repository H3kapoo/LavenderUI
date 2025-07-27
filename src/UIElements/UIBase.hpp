#pragma once

#include "src/ElementComposable/LayoutBase.hpp"
#include "src/ElementComposable/PropsBase.hpp"
#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/ElementComposable/Events.hpp"
#include "src/State/UIWindowState.hpp"
#include "src/Utils/Logger.hpp"

namespace src::uielements
{
/* MANDATORY insertion when creating any new UIBase derived class. */
#define INSERT_TYPEINFO(UIElement)\
    auto getTypeInfo() const -> std::type_index override { return typeid(UIElement); };\
    auto getTypeId() const -> uint32_t override { return typeId; };\
    inline static const uint32_t typeId = utils::getTypeId<UIElement>();\

class UIBase;
using UIBasePtr = std::shared_ptr<UIBase>;
using UIBaseWPtr = std::weak_ptr<UIBase>;
using UIBasePtrVec = std::vector<UIBasePtr>;

template<typename T>
concept UIBaseDerived = 
    std::is_base_of_v<UIBase, typename std::pointer_traits<std::remove_cvref_t<T>>::element_type>;

/**
    @brief:
        Abstract class defining base functionality for any new UI element.

    @notes:
    (1) When deriving a new element from this, ensure to use INSERT_TYPEINFO macro.
*/
class UIBase : public std::enable_shared_from_this<UIBase>,
    public elementcomposable::LayoutBase,
    public elementcomposable::PropsBase,
    public elementcomposable::Events
{
public:
    UIBase(const std::type_index& typeIndex);
    virtual ~UIBase() = default;
    UIBase(const UIBase&) = delete;
    UIBase(UIBase&&) = delete;
    UIBase& operator=(const UIBase&) = delete;
    UIBase& operator=(UIBase&&) = delete;

    /**
        @brief:
            Add a new UI child element to this element.

        @params:
            element - lrvalue UIBase element

        @returns:
            True on addition success.
            False otherwise.
    */
    auto add(const UIBasePtr& element) -> bool;

    /**
        @brief:
            Add a new UI child element to this element.

        @params:
            element - rvalue UIBase element

        @returns:
            True on addition success.
            False otherwise.
    */
    auto add(UIBasePtr&& element) -> bool;

    /**
        @brief:
            Add a new array of UI child elements to this element.

        @params:
            elements - lrvalue UIBase elements vector.

        @returns:
            True on addition success.
            False otherwise.
    */
    auto add(const UIBasePtrVec& elements) -> void;

    /**
        @brief:
            Add a new array of UI child elements to this element.

        @params:
            elements - rvalue UIBase elements vector

        @returns:
            True on addition success.
            False otherwise.
    */
    auto add(UIBasePtrVec&& elements) -> void;

    /**
        @brief:
            Remove UI child element by predicate.

        @params:
            pred - remove predicate

        @returns:
            The number of deleted elements.
    */
    auto remove(const std::function<bool(const UIBasePtr&)>& pred) -> uint32_t;

    /**
        @brief:
            Remove UI child element by element itself.

        @params:
            element - to remove

        @returns:
            True on success.
            False otherwise.
    */
    auto remove(const UIBasePtr& element) -> bool;

    /**
        @brief:
            Remove UI children elements.

        @params:
            elements - lrvalue vector to remove
    */
    auto remove(const UIBasePtrVec& elements) -> void;

    /**
        @brief:
            Remove UI children elements.

        @params:
            elements - rvalue vector to remove
    */
    auto remove(UIBasePtrVec&& elements) -> void;

    auto setCustomTagId(const uint32_t id) -> void;

    auto isParented() -> bool;
    auto getCustomTagId() -> uint32_t;
    auto getId() -> uint32_t;
    auto getElements() -> UIBasePtrVec&;

    virtual auto getTypeInfo() const -> std::type_index = 0;
    virtual auto getTypeId() const -> uint32_t = 0;

    /* Print overload */
    friend auto operator<<(std::ostream& out, const UIBasePtr&) -> std::ostream&;

protected:
    auto renderNext(const glm::mat4& projection) -> void;
    auto renderNextExcept(const glm::mat4& projection,
        const std::function<bool(const UIBasePtr&)> filterFunc) -> void;
    auto renderNextSingle(const glm::mat4& projection, const UIBasePtr& element) -> void;

    auto layoutNext() -> void;
    auto eventNext(state::UIWindowStatePtr& state) -> void;
    virtual auto render(const glm::mat4& projection) -> void;
    virtual auto layout() -> void;
    virtual auto event(state::UIWindowStatePtr& state) -> void;

    static auto demangleName(const char* name) -> std::string;

private:
    template<UIBaseDerived T>
    auto addInternal(T&& element) -> bool;

    template<UIBaseDerived T>
    auto removeInternal(T&& element) -> bool;

protected:
    uint32_t id_;
    uint32_t customTagid_;
    utils::Logger log_;
    resourceloaders::Mesh mesh_;
    resourceloaders::Shader shader_;

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