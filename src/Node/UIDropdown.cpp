#include "include/LavenderUI/Node/UIDropdown.hpp"

#include "include/LavenderUI/Core/EventHandler/CoreEvents/MouseButton.hpp"
#include "include/LavenderUI/Core/EventHandler/CoreEvents/MouseLeftRelease.hpp"
#include "include/LavenderUI/Core/LayoutHandler/Calculators/DropdownCalculator.hpp"
#include "include/LavenderUI/Core/LayoutHandler/LayoutBase.hpp"
#include "include/LavenderUI/Core/State/UIState.hpp"
#include "include/LavenderUI/Node/UIBase.hpp"
#include "include/LavenderUI/Node/UIButton.hpp"
#include "include/LavenderUI/Utils/Misc.hpp"

namespace lav::node
{
uint32_t UIDropdown::dropdownIndexOffset = 2;

UIDropdown::UIDropdown(UIBaseInitData&& initData)
    : UIButton(std::move(initData))
    , optionsHolder_(utils::make<UIPane>())
    , openDir_(OpenDir::BOTTOM)
{
    layoutBase_.setScale({100_px, 36_px});
    optionsHolder_->setColor(utils::hexToVec4("#ffffffff"));
    optionsHolder_->getBaseLayoutData()
        .setPos({0, 0})
        .setScale({1_fit})
        .setBorder({1})
        .setType(core::LayoutBase::Type::VERTICAL);

    label_->getBaseLayoutData().setScale({1.0_rel});
    label_->setColor(utils::hexToVec4("#ffffff00"));
}

auto UIDropdown::onRender(const glm::mat4& projection) -> void
{
    /* Render me exactly as my base type. */
    UIButton::onRender(projection);
}

auto UIDropdown::onLayout() -> void
{
    const auto& calculator = core::DropdownCalculator::get();
    calculator.calculateScaleForGenericElement(this);
    calculator.calculatePositionForDropdownElement(this);
}

auto UIDropdown::onEvent(core::UIStatePtr& state) -> void
{
    const auto eId = state->currentEventId;
    if (eId == core::MouseLeftReleaseEvt::eventId)
    {
        isOpen() ? closeDropdown() : UIBase::add(optionsHolder_);
    }

    /* Handle event exactly as my base type. */
    UIButton::onEvent(state);

    /* Deal with click release anywhere else except THIS Dropdown object. */
    if (isOpen()
        && state->hoveredId != id_
        && eId == core::MouseButtonEvt::eventId
        && state->mouseAction == lav::Action::RELEASE
        && !isSelectedMyDropdownChildRecursive(state->selectedId))
    {
        closeDropdown();
    }
}

auto UIDropdown::addOption(UIButtonPtr&& opt) -> void
{
    optionsHolder_->add(std::move(opt));
}

auto UIDropdown::addSubMenu(UIDropdownPtr&& subMenu) -> void
{
    optionsHolder_->add(subMenu);
}

auto UIDropdown::addOption(const std::string& optName) -> UIButtonWPtr
{
    UIButtonPtr opt = utils::make<UIButton>();
    opt->setText(optName);
    optionsHolder_->add(opt);
    return opt;
}

auto UIDropdown::addSubMenu(const std::string& subMenuName) -> UIDropdownWPtr
{
    UIDropdownPtr subDropdownMenu = utils::make<UIDropdown>();
    subDropdownMenu->setText(subMenuName);
    optionsHolder_->add(subDropdownMenu);
    return subDropdownMenu;
}

auto UIDropdown::closeDropdown() -> bool
{
    optionsHolder_->resetElementsToDefault();
    auto& holderEls = optionsHolder_->getElements();
    for (auto& el : holderEls)
    {
        if (el->getTypeId() == UIDropdown::typeId)
        {
            utils::as<UIDropdown>(el)->closeDropdown();
        }
    }

    return UIBase::remove(optionsHolder_);
}

auto UIDropdown::isSelectedMyDropdownChildRecursive(const uint32_t selectedId) -> bool
{
    auto& holderEls = optionsHolder_->getElements();
    for (auto& el : holderEls)
    {
        if (el->getTypeId() == UIDropdown::typeId)
        {
            if (el->getId() == selectedId) { return true; }
            return utils::as<UIDropdown>(el)->isSelectedMyDropdownChildRecursive(selectedId);
        }
    }

    return false;
}

auto UIDropdown::isSelectedMyButtonChildRecursive(const uint32_t selectedId) -> UIBaseWPtr
{
    auto& holderEls = optionsHolder_->getElements();
    for (auto& el : holderEls)
    {
        if (el->getTypeId() == UIButton::typeId)
        {
            if (el->getId() == selectedId) { return el; }
        }
        else
        {
            return utils::as<UIDropdown>(el)->isSelectedMyButtonChildRecursive(selectedId);
        }
    }

    return {};
}

auto UIDropdown::setPreferredOpenDir(const OpenDir od) -> UIDropdown& { openDir_ = od; return *this; }

auto UIDropdown::isOpen() const -> bool { return optionsHolder_->isParented(); }

auto UIDropdown::isClosed() const -> bool { return !isOpen(); }

auto UIDropdown::getOpenDirection() const -> OpenDir { return openDir_; }

auto UIDropdown::getOptionsHolder() -> UIPaneWPtr { return optionsHolder_; }
} // namespace src::uielements
