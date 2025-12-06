#include "DropdownRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Core/LavParser/ParseHelpers.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::core
{
auto DropdownRule::getRule() const -> IRule::RuleData
{
    return {getConstructRule(), getAdditionRule()};
}

auto DropdownRule::getConstructRule() const -> ConstructRule
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        const auto& ph = ParseHelper::get();
        node::UIDropdownPtr obj = utils::make<node::UIDropdown>();
        for (const auto&[key, value] : attribs)
        {
            if (key == "scale")
            {
                obj->getBaseLayoutData().setScale(ph.toScale(value));
            }
            else if (key == "border")
            {
                obj->getBaseLayoutData().setBorder(ph.toBorder(value));
            }
            else if (key == "text")
            {
                obj->setText(value);
            }
            else if (key == "dir")
            {
                obj->setPreferredOpenDir(toOpenDir(value));
            }
            else if (key == "color")
            {
                obj->setColor(ph.toColor(value));
            }
            else if (key == "bcolor")
            {
                obj->setBorderColor(ph.toColor(value));
            }
        }
        return obj;
    };
}

auto DropdownRule::getAdditionRule() const -> AddRule
{
    return [this](node::UIBasePtr parent, node::UIBasePtr child) -> void
    {
        auto castDropdown = utils::as<node::UIDropdown>(parent);
        if (child->getTypeId() == node::UIButton::typeId)
        {
            auto castButton = utils::as<node::UIButton>(child);
            castDropdown->addOption(std::move(castButton));
        }
        else if (child->getTypeId() == node::UIDropdown::typeId)
        {
            auto castSubmenu = utils::as<node::UIDropdown>(child);
            castDropdown->addSubMenu(std::move(castSubmenu));
        }
    };
}

auto DropdownRule::toOpenDir(const std::string& value) const -> node::UIDropdown::OpenDir
{
    if (value == "B" || value == "Bottom") { return node::UIDropdown::OpenDir::BOTTOM; }
    if (value == "T" || value == "Top") { return node::UIDropdown::OpenDir::TOP; }
    if (value == "L" || value == "Left") { return node::UIDropdown::OpenDir::LEFT; }
    if (value == "R" || value == "Right") { return node::UIDropdown::OpenDir::RIGHT; }

    utils::Logger log_{"DropdownRule"};
    log_.error("Invalid open direction: '{}'", value);
    return node::UIDropdown::OpenDir::BOTTOM;
}
} // namespace lav::core