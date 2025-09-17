#include "UIDropdown.hpp"

#include "src/ElementComposable/IEvent.hpp"
#include "src/ElementComposable/LayoutBase.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/LayoutCalculator/BasicCalculator.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/Utils/Misc.hpp"
#include "src/WindowManagement/Input.hpp"

namespace src::uielements
{
using namespace elementcomposable;
using namespace windowmanagement;

UIDropdown::UIDropdown() : UIBase(getTypeInfo())
{
    setScale({100_px, 36_px});
    optionsHolder_ = utils::make<UIPane>();
    optionsHolder_->setScale({1_fit})
        .setBorder({4})
        .setType(LayoutBase::Type::VERTICAL);
}

auto UIDropdown::render(const glm::mat4& projection) -> void
{
    mesh_.bind();
    shader_.bind();
    shader_.uploadMat4("uMatrixProjection", projection);
    shader_.uploadMat4("uMatrixTransform", getTransform());
    shader_.uploadVec4f("uColor", getColor());
    shader_.uploadVec2f("uResolution", getComputedScale());
    shader_.uploadVec4f("uBorderSize", getBorder());
    shader_.uploadVec4f("uBorderRadii", getBorderRadius());
    shader_.uploadVec4f("uBorderColor", getBorderColor());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    /* Draw the text */
    const auto& textShader_ = textAttribs_.getShader();
    const auto& textBuffer = textAttribs_.getBuffer();
    textShader_.bind();
    textShader_.uploadVec4f("uColor", utils::hexToVec4("#141414ff"));
    textShader_.uploadTexture2DArray("uTextureArray", GL_TEXTURE0, textAttribs_.getFont()->textureId);
    textShader_.uploadMat4("uMatrixProjection", projection);
    textShader_.uploadMat4v("uModelMatrices", textBuffer.model);
    textShader_.uploadIntv("uCharIndices", textBuffer.glyphCode);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, textAttribs_.getText().size());

    renderNext(projection);
}

auto UIDropdown::layout() -> void
{
    using namespace layoutcalculator;

    using namespace layoutcalculator;
    const auto& calculator = BasicCalculator::get();

    calculator.calculateScaleForGenericElement(this);
    calculator.calculatePositionForDropdownElement(this);

    const glm::vec2 p = getComputedPos() + getComputedScale() / 2.0f
        - textAttribs_.computeMaxSize() / 2.0f;
    textAttribs_.setPosition(p);

    layoutNext();
}

auto UIDropdown::event(state::UIWindowStatePtr& state) -> void
{
    /* Let the base do the generic stuff */
    UIBase::event(state);

    const auto eId = state->currentEventId;

    if (eId == MouseButtonEvt::eventId
        && (state->hoveredId == id_ || state->clickedId == id_ || state->prevHoveredId == id_))
    {
        MouseButtonEvt e{state->mouseButton, state->mouseAction};
        /* We can safely ignore bubbling down the tree as we found the clicked element. */
        return emitEvent<MouseButtonEvt>(e);
    }
    else if (eId == MouseLeftClickEvt::eventId && state->clickedId == id_)
    {
        setColor(onClickColor_);

        /* We can safely ignore bubbling down the tree as we found the clicked element. */
        MouseLeftClickEvt e{state->mousePos.x, state->mousePos.y};
        return emitEvent<MouseLeftClickEvt>(e);
    }
    else if (eId == MouseLeftReleaseEvt::eventId && state->selectedId == id_)
    {
        setColor(originalColor_);
        isOpen() ? closeDropdown() : add(optionsHolder_);

        /* We can safely ignore bubbling down the tree as we found the clicked element. */
        MouseLeftReleaseEvt e;
        return emitEvent<MouseLeftReleaseEvt>(e);
    }
    else if (eId == MouseEnterEvt::eventId && state->hoveredId == id_)
    {
        setColor(onEnterColor_);

        MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        return emitEvent<MouseEnterEvt>(e);
    }
    else if (eId == MouseExitEvt::eventId && state->prevHoveredId == id_)
    {
        if (state->clickedId != id_) { setColor(originalColor_); }

        MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        /* We can safely ignore bubbling down the tree as we found the entered element. */
        return emitEvent<MouseExitEvt>(e);
    }

    /* Handle child elements events first as they have a chance to get removed immediately after this. */
    eventNext(state);

    /* AFTER handling the rest of the mouse cases & child elements events, check if the mouse was released
        so that this dropdown is not the hovered element anymore (maybe one of our chain elements is).
        If so, we selected another element and, if the dropdown is open, we need to close it.
    */
    if (eId == MouseLeftReleaseEvt::eventId && isOpen()
        && state->hoveredId != id_ && !isSelectedChainDropdown(state->selectedId))
    {
        closeDropdown();
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

auto UIDropdown::closeDropdown() -> bool
{
    auto& holderEls = optionsHolder_->getElements();
    for (auto& el : holderEls)
    {
        if (el->getTypeId() == UIDropdown::typeId)
        {
            utils::as<UIDropdown>(el)->closeDropdown();
        }
    }

    return remove(optionsHolder_);
}

auto UIDropdown::isSelectedChainDropdown(const uint32_t selectedId) -> bool
{
    auto& holderEls = optionsHolder_->getElements();
    for (auto& el : holderEls)
    {
        if (el->getTypeId() == UIDropdown::typeId)
        {
            if (el->getId() == selectedId) { return true; }
            return utils::as<UIDropdown>(el)->isSelectedChainDropdown(selectedId);
        }
    }

    return false;
}

auto UIDropdown::setPreferredOpenDir(const OpenDir od) -> void { openDir_ = od; }

auto UIDropdown::setFont(const std::filesystem::path& fontPath) -> void{}

auto UIDropdown::setText(const std::string& text) -> void { textAttribs_.setText(text); }

auto UIDropdown::isOpen() const -> bool { return elements_.size() == 1; }

auto UIDropdown::isClosed() const -> bool { return !isOpen(); }

auto UIDropdown::getOpenDirection() const -> OpenDir { return openDir_; }

} // namespace src::uielements
