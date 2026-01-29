#include <LavenderUI/Core/LayoutHandler/Calculators/DropdownCalculator.hpp>

namespace lav::core
{
auto DropdownCalculator::get() -> DropdownCalculator&
{
    static DropdownCalculator instance;
    return instance;
}

DropdownCalculator::DropdownCalculator()
    : log_(__func__)
{}

auto DropdownCalculator::calculatePositionForDropdownElement(node::UIDropdown* dropdown) const -> void
{
    static const uint32_t EXPECTED_ELEMENT_SIZE = 2;
    static const uint32_t OPTIONS_HOLDER_INDEX = 1;

    /* This will calculate the positioning of the UILabel inside which is always present. */
    calculatePositionForGenericElement(dropdown);

    /* If we don't have 2 elements it is safe to say we don't have the options holder added. */
    if (dropdown->getElements().size() < EXPECTED_ELEMENT_SIZE) { return; }

    const auto& dLayout = dropdown->getBaseLayoutData();
    const bool isNestedDropdown = isNestedInsideAnotherDropdown(dropdown);

    /*
        If this dropdown is nested, refer to the optionHolder that we are a child of for
        dimensions as these are a more accurate representation of where to open the nested dropdown.
        Otherwise refer to bounds of the dropdown itself.
    */
    const auto& optionsHolder = dropdown->getElements().at(OPTIONS_HOLDER_INDEX);
    const auto& optionsHolderLayout = optionsHolder->getBaseLayoutData();
    const auto openDir = dropdown->getOpenDirection();
    const glm::ivec2 ddFullBoxPos = dLayout.getFullBoxPos();
    const glm::ivec2 ddFullBoxScale = dLayout.getFullBoxScale();

    glm::ivec2 parentHolderFullBoxPos{0, 0};
    glm::ivec2 parentHolderFullBoxScale{0, 0};
    if (isNestedDropdown)
    {
        const auto parentOptionsHolder = dropdown->getParent().lock();
        const auto& parentOptionsHolderLayout = parentOptionsHolder->getBaseLayoutData();
        parentHolderFullBoxPos = parentOptionsHolderLayout.getFullBoxPos();
        parentHolderFullBoxScale = parentOptionsHolderLayout.getFullBoxScale();
    }

    LayoutBase::PositionXY finalPos{{0, LayoutBase::PositionType::ABS}, {0, LayoutBase::PositionType::ABS}};
    switch (openDir)
    {
        case node::UIDropdown::OpenDir::TOP:
        {
            finalPos.x.val = ddFullBoxPos.x;
            finalPos.y.val = ddFullBoxPos.y - optionsHolderLayout.getFullBoxScale().y;
            break;
        }
        case node::UIDropdown::OpenDir::BOTTOM:
        {
            finalPos.x.val = ddFullBoxPos.x;
            finalPos.y.val = ddFullBoxPos.y + ddFullBoxScale.y;
            break;
        }
        case node::UIDropdown::OpenDir::LEFT:
        {
            finalPos.x.val = isNestedDropdown
                ? parentHolderFullBoxPos.x - parentHolderFullBoxScale.x
                : ddFullBoxPos.x + ddFullBoxScale.x;
            finalPos.y.val = ddFullBoxPos.y;
            break;
        }
        case node::UIDropdown::OpenDir::RIGHT:
        {
            finalPos.x.val = isNestedDropdown
                ? parentHolderFullBoxPos.x + parentHolderFullBoxScale.x
                : ddFullBoxPos.x + ddFullBoxScale.x;
            finalPos.y.val = ddFullBoxPos.y;
            break;
        }
    }

    optionsHolder->getBaseLayoutData().setPos(finalPos);
    optionsHolder->getBaseLayoutData().setComputedPos({finalPos.x.val, finalPos.y.val});
}

auto DropdownCalculator::isNestedInsideAnotherDropdown(node::UIDropdown* dropdown) const -> bool
{
    const auto dGrandParent = dropdown->getGrandParent().lock();
    return dGrandParent && dGrandParent->getTypeId() == node::UIDropdown::typeId;
}

} // namespace lav::core