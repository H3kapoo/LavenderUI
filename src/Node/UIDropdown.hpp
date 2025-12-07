#pragma once

#include "src/Node/UIBase.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Node/UIPane.hpp"

namespace lav::node
{
/**
    @brief Dropdown menu which generic click functionality.

    @note Can be inherited to create custom button types.
*/
class UIDropdown;
using UIDropdownPtr = std::shared_ptr<UIDropdown>;
using UIDropdownWPtr = std::weak_ptr<UIDropdown>;
using UIDropdownPtrVec = std::vector<UIDropdownPtr>;

class UIDropdown : public UIButton
{
public:
    enum class OpenDir
    {
        TOP, BOTTOM, LEFT, RIGHT
    };

public:
    /* Mandatory typeinfo */
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIDropdown, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UIDropdown);

    auto addOption(UIButtonPtr&& opt) -> void;
    auto addSubMenu(UIDropdownPtr&& subMenu) -> void;
    auto addOption(const std::string& optName) -> UIButtonWPtr;
    auto addSubMenu(const std::string& subMenuName) -> UIDropdownWPtr;

    auto setPreferredOpenDir(const OpenDir od) -> UIDropdown&;
    auto isOpen() const -> bool;
    auto isClosed() const -> bool;
    auto getOpenDirection() const -> OpenDir;
    auto getOptionsHolder() -> UIPaneWPtr;

public:
    static uint32_t dropdownIndexOffset /** @brief Dropdown's pane child needs to start at a higher z index */;

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(node::UIStatePtr& state) -> void override;

    auto closeDropdown() -> bool;
    auto isSelectedMyDropdownChildRecursive(const uint32_t selectedId) -> bool;
    auto isSelectedMyButtonChildRecursive(const uint32_t selectedId) -> UIBaseWPtr;

protected:
    UIPanePtr optionsHolder_{utils::make<UIPane>()};
    OpenDir openDir_{OpenDir::BOTTOM};

    glm::vec4 originalColor_{utils::hexToVec4("#c73e3eff")};
    glm::vec4 onEnterColor_{utils::hexToVec4("#c95959ff")};
    glm::vec4 onClickColor_{utils::hexToVec4("#c41c1cff")};
};
} // namespace src::uielements
