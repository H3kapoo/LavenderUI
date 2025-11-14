#pragma once

#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Core/ResourceHandler/Mesh.hpp"
#include "src/Core/ResourceHandler/Shader.hpp"
#include "src/Core/EventHandler/Events.hpp"
#include "src/Node/Helpers/UIState.hpp"
#include "src/Utils/Logger.hpp"

namespace lav::node
{

/**
    @brief:
    Forward declaration of UIWindow has the purpose of letting only that class to manage when and how
    all UIBase objects are rendered/layouted/event triggered.
    Befreinding UIWindow is a must in this case as we don't want to otherwise let the user dictate these
    three operations.
*/
class UIWindow;

/* Aliases */
class UIBase;
using UIBasePtr = std::shared_ptr<UIBase>;
using UIBaseWPtr = std::weak_ptr<UIBase>;
using UIBasePtrVec = std::vector<UIBasePtr>;

/**
    @brief
    Each instantiation of UIBase needs to know what vertex/fragment/other shader it needs to load from
    and additionally a name, used mostly for logging. This is passed via move so no copying occurs. This
    can be made even more trivial (static structure with default names/shaders) but object creation is not
    on a critical path anyway and strings are short.*/
struct UIBaseInitData
{
    std::string name;
    std::filesystem::path vertexShader;
    std::filesystem::path fragmentShader;
};

/**
    @brief
    All UIBase objects need to be quickly identified in regards to their type. Each object has a uint32_t id
    and also a pure virtual function to get that id on any instance of an object.
    This id and function need to be implemented per UIBase object and this macro makes it trivial.
    Warning: Without this macro, it is impossible to say what type the object has and as such it is mandatory.
 */
#define INSERT_TYPEINFO(UIElement)\
    auto getTypeId() const -> uint32_t override { return typeId; };\
    inline static const uint32_t typeId = utils::getTypeId<UIElement>();\

/**
    @brief
    Exactly what INSERT_TYPEINFO does but additionaly it deletes move/copy constructors, inserts virt descructor
    and defines the basic constructor for receiving @ref `UIBaseInitData`.
*/
#define INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIElement, vertShader, fragShader)\
    UIElement(UIBaseInitData&& initData = { #UIElement, vertShader, fragShader });\
    virtual ~UIElement() = default;\
    UIElement(const UIElement&) = delete;\
    UIElement(UIElement&&) = delete;\
    auto operator=(const UIElement&) -> UIElement& = delete;\
    auto operator=(UIElement&&) -> UIElement& = delete;\
    INSERT_TYPEINFO(UIElement)

/**
    @brief
    Insert this in case you want to not allow the user to add/remove elements to things it shouldn't.
    Some elements are not allowed to have children added or removed by user, but developers can add/remove
    elements internally by means of UIBase:: direct calls.
*/
#define INSERT_ADD_REMOVE_NOT_ALLOWED(UIElement)\
    auto add(const UIBasePtr&) -> bool override\
    {\
        log_.warn("Element '" #UIElement "' doesn't allow addition of new elements!");\
        return false;\
    }\
    auto add(const UIBasePtrVec&) -> void override\
    {\
        log_.warn("Element '" #UIElement "' doesn't allow addition of new elements!");\
    }\
    auto remove(const std::function<bool(const UIBasePtr&)>&) -> uint32_t override\
    {\
        log_.warn("Element '" #UIElement "' doesn't allow removal elements!");\
        return false;\
    }\
    auto remove(const UIBasePtr&) -> bool override\
    {\
        log_.warn("Element '" #UIElement "' doesn't allow removal elements!");\
        return false;\
    }\
    auto remove(const UIBasePtrVec&) -> void override\
    {\
        log_.warn("Element '" #UIElement "' doesn't allow removal elements!");\
    }\
    auto remove(UIBasePtrVec&&) -> void override\
    {\
        log_.warn("Element '" #UIElement "' doesn't allow removal elements!");\
    }\

/**
    @brief
    Abstract class defining base functionality for all UI Elements.

    @note 1. Whenever a new object is derived from this, ensure to use INSERT_CONSTRUCT_COPY_MOVE_DEFS
        macro in order to insert the mandatory background functions such as getting the typeId and
        deleting unwanted constructors/operators.
    @note 2. Whenever a new object is derived from this, not using INSERT_CONSTRUCT_COPY_MOVE_DEFS macro
        enforces the use of INSERT_TYPEINFO macro instead. Constructors are now user's responsability.
    @note 3. Addition and removal of child elements needs to be implemented per each object type as it
        can happen for that object to not be able to support new user added elements.
        Use INSERT_ADD_REMOVE_NOT_ALLOWED macro so that the object will not be able to add/remove objects
        from external means. Internally it can add/remove via accessing UIBase.
*/
class UIBase : public std::enable_shared_from_this<UIBase>
{
/* Our only friend for managing layout/render/events. */
public: friend class UIWindow;

public:
    UIBase(UIBaseInitData&& initData);
    virtual ~UIBase() = default;
    UIBase(const UIBase&) = delete;
    UIBase(UIBase&&) = delete;
    auto operator=(const UIBase&) -> UIBase& = delete;
    auto operator=(UIBase&&) -> UIBase& = delete;

    virtual auto getTypeId() const -> uint32_t = 0;
    virtual auto add(const UIBasePtr& element) -> bool;
    virtual auto add(const UIBasePtrVec& elements) -> void;
    virtual auto remove(const std::function<bool(const UIBasePtr&)>& pred) -> uint32_t;
    virtual auto remove(const UIBasePtr& element) -> bool;
    virtual auto remove(const UIBasePtrVec& elements) -> void;
    virtual auto remove(UIBasePtrVec&& elements) -> void;

    auto setIgnoreEvents(const bool ignore = true) -> void;
    auto setColor(const glm::vec4& value) -> void;
    auto setBorderColor(const glm::vec4& value) -> void;

    auto isParented() -> bool;
    auto isIgnoringEvents() -> bool;
    auto getId() -> uint32_t;
    auto getParent() -> UIBaseWPtr;
    auto getGrandParent() -> UIBaseWPtr;
    auto getElements() -> UIBasePtrVec&;
    auto getBaseLayoutData() -> core::LayoutBase&;
    auto getEventManager() -> core::Events&;
    auto getColor() -> glm::vec4;
    auto getBorderColor() -> glm::vec4;

    /* Print overload */
    friend auto operator<<(std::ostream& out, const UIBasePtr&) -> std::ostream&;

private:
    virtual auto render(const glm::mat4& projection) -> void = 0;
    virtual auto layout() -> void = 0;
    virtual auto event(UIStatePtr& state) -> void = 0;

    static auto demangleName(const char* name) -> std::string;

protected:
    core::LayoutBase layoutBase_;
    core::Events eventsMgr_;
    std::string nameTag_;
    UIBaseWPtr parent_;
    UIBasePtrVec elements_;
    uint32_t customTagid_;
    uint32_t id_;
    utils::Logger log_;
    core::Mesh mesh_;
    core::Shader shader_;
    glm::vec4 baseColor_;
    glm::vec4 borderColor_;
    uint32_t depth_;
    bool isParented_;
    bool isIgnoringEvents_;
};
} // namespace lav::node

/* Global namespace */
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