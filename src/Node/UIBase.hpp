#pragma once

#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Core/ResourceHandler/Mesh.hpp"
#include "src/Core/ResourceHandler/Shader.hpp"
#include "src/Core/EventHandler/Events.hpp"
#include "src/Core/ResourceHandler/ShaderLoader.hpp"
#include "src/Node/Helpers/UIState.hpp"
#include "src/Utils/Logger.hpp"

namespace lav::node
{
class UIWindow;
class UIBase;
using UIBasePtr = std::shared_ptr<UIBase>;
using UIBaseWPtr = std::weak_ptr<UIBase>;
using UIBasePtrVec = std::vector<UIBasePtr>;

struct UIBaseInitData
{
    std::string name;
    std::string vertexShader;
    std::string fragmentShader;
};

/** @brief Concept denoting any type inheriting from UIBase */
template<typename T>
concept UIBaseDerived = 
    std::is_base_of_v<UIBase, typename std::pointer_traits<std::remove_cvref_t<T>>::element_type>;


/* MANDATORY define when creating any new UIBase derived class. */
#define INSERT_TYPEINFO(UIElement)\
    auto getTypeId() const -> uint32_t override { return typeId; };\
    inline static const uint32_t typeId = utils::getTypeId<UIElement>();\

/* Only some GUI elements can add remove/nodes from the user's perspective. This macro needs to
    be added in the derived class that wants to permit addition/removal of child GUI elements.
    Alternatively, if you only need some of this functionality, use as needed. */
#define BASE_ALLOW_ADD_REMOVE\
    using UIBase::add;\
    using UIBase::remove;\


/**
    @brief Abstract class defining base functionality for any new UI element.

    @note When deriving a new element from this, ensure to use INSERT_TYPEINFO macro.
*/
class UIBase : public std::enable_shared_from_this<UIBase>
{
public:
    friend class UIWindow;

public:
    UIBase(UIBaseInitData&& initData);
    virtual ~UIBase() = default;
    UIBase(const UIBase&) = delete;
    UIBase(UIBase&&) = delete;
    auto operator=(const UIBase&) -> UIBase& = delete;
    auto operator=(UIBase&&) -> UIBase& = delete;

    //TODO: Not needed, UISlider can have custom setting to play as a scrollbar or as a normal slider
    auto setCustomTagId(const uint32_t id) -> void;
    auto setIgnoreEvents(const bool ignore = true) -> void;

    auto isParented() -> bool;
    auto isIgnoringEvents() -> bool;
    auto getCustomTagId() -> uint32_t;
    auto getId() -> uint32_t;
    auto getParent() -> UIBaseWPtr;
    auto getGrandParent() -> UIBaseWPtr;
    auto getElements() -> UIBasePtrVec&;
    auto getBaseLayoutData() -> core::LayoutBase&;
    auto getEventManager() -> core::Events&;

    virtual auto getTypeId() const -> uint32_t = 0;

    /* Print overload */
    friend auto operator<<(std::ostream& out, const UIBasePtr&) -> std::ostream&;

protected:
    /**
        @brief Add a new UI child element to this element.

        @param element Lrvalue UIBase element

        @return True on addition success. False otherwise.
    */
    auto add(const UIBasePtr& element) -> bool;

    /**
        @brief Add a new array of UI child elements to this element.

        @param elements Lrvalue UIBase elements vector.

        @return True on addition success. False otherwise.
    */
    auto add(const UIBasePtrVec& elements) -> void;

    /**
        @brief Add a new array of UI child elements to this element.

        @param elements Rvalue UIBase elements vector

        @return True on addition success. False otherwise.
    */
    auto add(UIBasePtrVec&& elements) -> void;

    /**
        @brief Remove UI child element by predicate.

        @param pred Remove predicate

        @return The number of deleted elements.
    */
    auto remove(const std::function<bool(const UIBasePtr&)>& pred) -> uint32_t;

    /**
        @brief Remove UI child element by element itself.

        @param element To remove

        @return True on success. False otherwise.
    */
    auto remove(const UIBasePtr& element) -> bool;

    /**
        @brief Remove UI children elements.

        @param elements Lrvalue vector to remove
    */
    auto remove(const UIBasePtrVec& elements) -> void;

    /**
        @brief Remove UI children elements.

        @param elements Rvalue vector to remove
    */
    auto remove(UIBasePtrVec&& elements) -> void;

    /**
        @brief Render this element.

        @note Needs to be overriden in the derived class, otherwise the UIBase default logic is used.

        @param projection Projection matrix with which to render
    */
    virtual auto render(const glm::mat4& projection) -> void = 0;

    /**
        @brief Do layout calculation for this element's children.

        @note Needs to be overriden in the derived class, otherwise the UIBase default logic is used.
    */
    virtual auto layout() -> void = 0;

    /**
        @brief Process the current event propagated to this element

        @note Needs to be overriden in the derived class, otherwise the UIBase default logic is used.

        @param state Current state of the window holding the event type
    */
    virtual auto event(UIStatePtr& state) -> void = 0;

private:
    static auto demangleName(const char* name) -> std::string;

protected:
    uint32_t id_;
    std::string nameTag_;
    uint32_t customTagid_;
    utils::Logger log_;
    core::Mesh mesh_;
    core::Shader shader_;
    core::LayoutBase layoutBase_;
    core::Events eventsMgr_;

    bool isParented_;
    bool isIgnoringEvents_{false};
    uint32_t depth_{0};
    UIBaseWPtr parent_;
    UIBasePtrVec elements_;
};
} // namespace lav::node

template<typename T>
requires std::is_base_of_v<lav::node::UIBase, T>
struct std::formatter<std::shared_ptr<T>> : std::formatter<std::string_view>
{
    auto format(const lav::node::UIBasePtr& obj, format_context& ctx) const
    {
        std::ostringstream os;
        os << obj;
        return std::formatter<std::string_view>::format(std::move(os).str(), ctx);
    }
};