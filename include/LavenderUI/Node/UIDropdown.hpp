#pragma once

#include <LavenderUI/Node/UIBase.hpp>
#include <LavenderUI/Node/UIButton.hpp>
#include <LavenderUI/Node/UIPane.hpp>

namespace lav::node
{
/**
    @brief
    Class represeting an UI element which can hold options to choose from
    via collapsing or expanding the holder.
*/

class UIDropdown;
using UIDropdownPtr = std::shared_ptr<UIDropdown>;
using UIDropdownWPtr = std::weak_ptr<UIDropdown>;
using UIDropdownPtrVec = std::vector<UIDropdownPtr>;

class UIDropdown : public UIButton
{
public:
    enum class OpenDir { TOP, BOTTOM, LEFT, RIGHT };

public:
    INSERT_CONSTRUCT_COPY_MOVE_DEFS(UIDropdown, "elemVert.glsl", "elemFrag.glsl");
    INSERT_ADD_REMOVE_NOT_ALLOWED(UIDropdown);

    [[nodiscard]] auto addOption(const std::string& optName) -> UIButtonWPtr;
    [[nodiscard]] auto addSubMenu(const std::string& subMenuName) -> UIDropdownWPtr;
    auto open() -> void;
    auto close() -> void;

    auto setPreferredOpenDir(const OpenDir od) -> UIDropdown&;
    auto isOpen() const -> bool;
    auto isClosed() const -> bool;
    auto getOpenDirection() const -> OpenDir;
    auto getOptionsHolder() -> UIPaneWPtr;

private:
    auto onRender(const glm::mat4& projection) -> void override;
    auto onLayout() -> void override;
    auto onEvent(core::UIStatePtr& state) -> void override;

    auto getChildIfSelected(const uint32_t selectedId) -> UIBaseWPtr;

protected:
    UIPanePtr optionsHolder_;
    OpenDir openDir_;
};
} // namespace src::uielements
