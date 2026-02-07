#include <LavenderUI/Core/ViewParser/Rules/DropdownRule.hpp>

#include <LavenderUI/Core/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/Core/ViewParser/ParseHelpers.hpp>
#include <LavenderUI/Node/UIBase.hpp>
#include <LavenderUI/Node/UIDropdown.hpp>
#include <LavenderUI/Utils/Logger.hpp>
#include <LavenderUI/Utils/Misc.hpp>

namespace lav::core
{
auto DropdownRule::construct(const RuleMap& ruleMap, const XmlNode& xmlNode,
        node::UIBasePtr parent, const bool shouldAddToParent) const -> node::UIBasePtr
{
    node::UIDropdownPtr newDropdown{nullptr};
    if (!shouldAddToParent)
    {
        newDropdown = utils::make<node::UIDropdown>();
        parseAndApply(newDropdown, xmlNode->attributes);
    }

    if (!ruleMap.contains("Button"))
    {
        log_.error("No rule for '{}' found! This is mandatory for SplitPane!", "Button");
        return newDropdown;
    }

    node::UIDropdownPtr castParent{nullptr};
    for (const auto& childXmlNode : xmlNode->children)
    {
        if (shouldAddToParent && childXmlNode->nodeName == "Dropdown")
        {
            castParent = utils::as<node::UIDropdown>(parent);
            auto sm = castParent->addSubMenu("").lock();
            ruleMap.at(childXmlNode->nodeName)->parseAndApply(sm, childXmlNode->attributes);
            ruleMap.at(childXmlNode->nodeName)->construct(ruleMap, childXmlNode, sm, true);
        }
        else if (shouldAddToParent && childXmlNode->nodeName == "Button")
        {
            castParent = utils::as<node::UIDropdown>(parent);
            auto btn = castParent->addOption("");
            ruleMap.at(childXmlNode->nodeName)->parseAndApply(btn.lock(), childXmlNode->attributes);
            ruleMap.at(childXmlNode->nodeName)->construct(ruleMap, childXmlNode, nullptr, false);
        }
        else if (!shouldAddToParent && childXmlNode->nodeName == "Dropdown")
        {
            auto sm = newDropdown->addSubMenu("").lock();
            ruleMap.at(childXmlNode->nodeName)->parseAndApply(sm, childXmlNode->attributes);
            ruleMap.at(childXmlNode->nodeName)->construct(ruleMap, childXmlNode, sm, true);
        }
        else if (!shouldAddToParent && childXmlNode->nodeName == "Button")
        {
            auto btn = newDropdown->addOption("");
            ruleMap.at(childXmlNode->nodeName)->parseAndApply(btn.lock(), childXmlNode->attributes);
            ruleMap.at(childXmlNode->nodeName)->construct(ruleMap, childXmlNode, nullptr, false);
        }
    }

    return newDropdown;
}

auto DropdownRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) const -> void
{
    const auto& ph = ParseHelper::get();
    node::UIDropdownPtr dd = utils::as<node::UIDropdown>(object);
    for (const auto&[key, value] : attribs)
    {
        if (key == "scale")
        {
            dd->getBaseLayoutData().setScale(ph.toScale(value));
        }
        else if (key == "border")
        {
            dd->getBaseLayoutData().setBorder(ph.toBorder(value));
        }
        else if (key == "text")
        {
            dd->setText(value);
        }
        else if (key == "dir")
        {
            dd->setPreferredOpenDir(toOpenDir(value));
        }
        else if (key == "color")
        {
            dd->setColor(ph.toColor(value));
        }
        else if (key == "bcolor")
        {
            dd->setBorderColor(ph.toColor(value));
        }
        else if (key == "vid")
        {
            dd->setViewId(value);
        }
    }
}

auto DropdownRule::toOpenDir(const std::string& value) const -> node::UIDropdown::OpenDir
{
    if (value == "B" || value == "Bottom") { return node::UIDropdown::OpenDir::BOTTOM; }
    if (value == "T" || value == "Top") { return node::UIDropdown::OpenDir::TOP; }
    if (value == "L" || value == "Left") { return node::UIDropdown::OpenDir::LEFT; }
    if (value == "R" || value == "Right") { return node::UIDropdown::OpenDir::RIGHT; }

    log_.error("Invalid open direction: '{}'", value);
    return node::UIDropdown::OpenDir::BOTTOM;
}
} // namespace lav::core