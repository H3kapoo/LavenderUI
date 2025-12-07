#include "UIDropdown.hpp"

#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Core/Binders/WindowBinder.hpp"
#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LayoutHandler/Calculators/DropdownCalculator.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Node/Helpers/UIState.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::node
{
uint32_t UIDropdown::dropdownIndexOffset = 2;

UIDropdown::UIDropdown(UIBaseInitData&& initData) : UIBase(std::move(initData))
{
    layoutBase_.setScale({100_px, 36_px});
    optionsHolder_->setColor(utils::hexToVec4("#ffffffff"));
    optionsHolder_->getBaseLayoutData()
        .setPos({0, 0})
        .setScale({1_fit})
        // .setScale({100_px, 150_px})
        .setBorder({1})
        .setType(core::LayoutBase::Type::VERTICAL);

    label_->getBaseLayoutData().setScale({1.0_rel});
    label_->setColor(utils::hexToVec4("#ffffff00"));
    UIBase::add(label_);
}

auto UIDropdown::render(const glm::mat4& projection) -> void
{
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", layoutBase_.getTransform());
    shader_.uploadVec4f("uColor", getColor());
    shader_.uploadVec2f("uResolution", layoutBase_.getComputedScale());
    shader_.uploadVec4f("uBorderSize", layoutBase_.getBorder());
    shader_.uploadVec4f("uBorderRadii", layoutBase_.getBorderRadius());
    shader_.uploadVec4f("uBorderColor", getBorderColor());
    shader_.uploadInt("uUseTexture", 0);
    core::GPUBinder::get().renderBoundQuad();
}

auto UIDropdown::layout() -> void
{
    const auto& calculator = core::DropdownCalculator::get();
    calculator.calculateScaleForGenericElement(this);
    calculator.calculatePositionForDropdownElement(this);
}

auto UIDropdown::event(node::UIStatePtr& state) -> void
{
    const auto eId = state->currentEventId;
    if (eId == core::MouseLeftClickEvt::eventId)
    {
        setColor(onClickColor_);

        core::MouseLeftClickEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseLeftClickEvt>(e);
    }
    else if (eId == core::MouseLeftReleaseEvt::eventId)
    {
        setColor(originalColor_);
        // TODO: Before adding, check if there are elements inside it, otherwise it's pointless
        isOpen() ? closeDropdown() : UIBase::add(optionsHolder_);

        core::MouseLeftReleaseEvt e;
        eventsMgr_.emitEvent<core::MouseLeftReleaseEvt>(e);
    }
    else if (eId == core::MouseEnterEvt::eventId)
    {
        originalColor_ = getColor();
        setColor(onEnterColor_);

        core::MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseEnterEvt>(e);
    }
    else if (eId == core::MouseExitEvt::eventId)
    {
        if (state->clickedId != id_) { setColor(originalColor_); }

        core::MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseExitEvt>(e);
    }

    /* Deal with click release anywhere else except THIS Dropdown object. */
    if (isOpen()
        && state->hoveredId != id_
        && eId == core::MouseButtonEvt::eventId
        && state->mouseAction == lav::RELEASE
        && !isSelectedMyDropdownChildRecursive(state->selectedId))
    {
        closeDropdown();
        // if (auto node = isSelectedMyButtonChildRecursive(state->selectedId); node.lock())
        // {
        //     log_.warn("oe aiuc");
            // return EventDoneActions{.doSomething = state->selectedId, .node = optionsHolder_};
        // }
        // keep in mind that THIS element has been removed along with all its subchildren
        // sent them a "youve been removed during X event" event
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

auto UIDropdown::setText(const std::string& text) -> UIDropdown& { label_->setText(text); return *this; }

auto UIDropdown::isOpen() const -> bool { return elements_.size() == 2; }

auto UIDropdown::isClosed() const -> bool { return !isOpen(); }

auto UIDropdown::getOpenDirection() const -> OpenDir { return openDir_; }

auto UIDropdown::getOptionsHolder() -> UIPaneWPtr { return optionsHolder_; }

auto UIDropdown::getText() -> std::string { return label_->getText(); }

} // namespace src::uielements
