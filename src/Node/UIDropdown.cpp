#include "LavenderUI/Core/EventHandler/CoreEvents/FocusLost.hpp"
#include <LavenderUI/Node/UIDropdown.hpp>

#include <LavenderUI/Core/EventHandler/CoreEvents/MouseButton.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseLeftRelease.hpp>
#include <LavenderUI/Core/LayoutHandler/Calculators/DropdownCalculator.hpp>
#include <LavenderUI/Core/LayoutHandler/LayoutBase.hpp>
#include <LavenderUI/Core/State/UIState.hpp>
#include <LavenderUI/Node/UIBase.hpp>
#include <LavenderUI/Node/UIButton.hpp>
#include <LavenderUI/Utils/Misc.hpp>

namespace lav::node
{
UIDropdown::UIDropdown(UIBaseInitData&& initData)
    : UIButton(std::move(initData))
    , optionsHolder_(utils::make<UIPane>())
    , openDir_(OpenDir::BOTTOM)
{
    setColor(utils::hexToVec4("#ffffff00"));
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
        // TODO: openDropdown fun needs to be added for completeness
        isOpen() ? close() : open();
    }

    /* Handle event exactly as my base type. */
    UIButton::onEvent(state);

    if (eId == core::MouseButtonEvt::eventId && isOpen() && state->hoveredId != getId())
    {
        /*
            Pressing click anywhere else except on this dropdown + it's children will close
            the dropdown if open.
        */
        if (state->mouseAction == lav::Action::PRESS)
        {
            if (const auto el = getChildIfSelected(state->selectedId).lock(); !el)
            {
                close();
            }
        }
        /*
            Releasing click anywhere else except on this dropdown + button it's children will close
            the dropdown if open. If release was on a UIDropdown, dropdown will stay open.
        */
        else if (state->mouseAction == lav::Action::RELEASE)
        {
            if (const auto el = getChildIfSelected(state->selectedId).lock();
                el && el->getTypeId() != UIDropdown::typeId)
            {
                close();
            }
        }
    }
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

auto UIDropdown::open() -> void
{
    if (isOpen()) { return; }

    UIBase::add(optionsHolder_);
}

auto UIDropdown::close() -> void
{
    if (!isOpen()) { return; }

    optionsHolder_->resetElementsToDefault();
    auto& holderEls = optionsHolder_->getElements();
    for (auto& el : holderEls)
    {
        if (el->getTypeId() == UIDropdown::typeId)
        {
            utils::as<UIDropdown>(el)->close();
        }
    }

    UIBase::remove(optionsHolder_);
}

auto UIDropdown::getChildIfSelected(const uint32_t selectedId) -> UIBaseWPtr
{
    auto& holderEls = optionsHolder_->getElements();
    for (auto& el : holderEls)
    {
        if (el->getTypeId() == UIDropdown::typeId || el->getTypeId() == UIButton::typeId)
        {
            if (el->getId() == selectedId) { return el; }
        }

        if (el->getTypeId() == UIDropdown::typeId)
        {
            if (auto ret = utils::as<UIDropdown>(el)->getChildIfSelected(selectedId).lock())
            {
                return ret;
            }
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
