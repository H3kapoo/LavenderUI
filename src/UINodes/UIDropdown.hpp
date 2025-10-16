#pragma once

#include "src/ElementComposable/TextAttribs.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIPane.hpp"

namespace src::uielements
{
using namespace elementcomposable;

/**
    @brief Simple generic button which generic click functionality.

    @note Can be inherited to create custom button types.
*/
class UIDropdown;
using UIDropdownPtr = std::shared_ptr<UIDropdown>;
using UIDropdownWPtr = std::weak_ptr<UIDropdown>;
using UIDropdownPtrVec = std::vector<UIDropdownPtr>;

class UIDropdown : public UIBase
{
public:
    enum class OpenDir
    {
        TOP, BOTTOM, LEFT, RIGHT
    };

public:
    UIDropdown();
    ~UIDropdown() = default;
    UIDropdown(const UIDropdown&) = delete;
    UIDropdown(UIBase&&) = delete;
    auto operator=(const UIDropdown&) -> UIDropdown& = delete;
    auto operator=(UIDropdown&&) -> UIDropdown& = delete;

    auto addOption(const std::string& optName) -> UIButtonWPtr;
    auto addSubMenu(const std::string& subMenuName) -> UIDropdownWPtr;

    auto setPreferredOpenDir(const OpenDir od) -> void;
    auto setFont(const std::filesystem::path& fontPath) -> void;
    auto setText(const std::string& text) -> void;
    auto isOpen() const -> bool;
    auto isClosed() const -> bool;
    auto getOpenDirection() const -> OpenDir;

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UIDropdown);

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(state::UIStatePtr& state) -> void override;

    auto closeDropdown() -> bool;
    auto isSelectedChainDropdown(const uint32_t selectedId) -> bool;

protected:
    UIPanePtr optionsHolder_;
    OpenDir openDir_{OpenDir::BOTTOM};

    TextAttribs textAttribs_;
    glm::vec4 originalColor_{utils::hexToVec4("#c73e3eff")};
    glm::vec4 onEnterColor_{utils::hexToVec4("#c95959ff")};
    glm::vec4 onClickColor_{utils::hexToVec4("#c41c1cff")};
};
} // namespace src::uielements
