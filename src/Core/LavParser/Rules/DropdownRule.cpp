#include "DropdownRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Core/LavParser/ParseHelpers.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIDropdown.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::core
{
auto DropdownRule::construct(node::UIBasePtr parent,
    const hk::XMLDecoder::NodeSPtr& xmlNode) -> node::UIBasePtr
{
    node::UIDropdownPtr dd = utils::make<node::UIDropdown>();
    parseAndApply(dd, xmlNode->attributes);

    utils::Logger log{"DDRULE"};
    for (const auto& childXmlNode : xmlNode->children)
    {
        if (parent->getTypeId() == node::UIDropdown::typeId)
        {
            node::UIDropdownPtr castParent = utils::as<node::UIDropdown>(parent);
            if (childXmlNode->nodeName == "Dropdown")
            {
                log.error("bla");
                auto sm = castParent->addSubMenu("").lock();
                IRule::ruleMap_[childXmlNode->nodeName]->parseAndApply(sm, childXmlNode->attributes);
                IRule::ruleMap_[childXmlNode->nodeName]->construct(sm, childXmlNode);
            }
            else if (childXmlNode->nodeName == "Button")
            {
                log.error("ra");
                auto btn = castParent->addOption("");
                IRule::ruleMap_[childXmlNode->nodeName]->parseAndApply(btn.lock(), childXmlNode->attributes);
                IRule::ruleMap_[childXmlNode->nodeName]->construct(castParent, childXmlNode);
            }
        }
        else // parent is not Dropdown
        {
            if (childXmlNode->nodeName == "Dropdown")
            {
                log.error("nod");
                auto sm = dd->addSubMenu("").lock();
                IRule::ruleMap_[childXmlNode->nodeName]->parseAndApply(sm, childXmlNode->attributes);
                IRule::ruleMap_[childXmlNode->nodeName]->construct(sm, childXmlNode);
            }
            else if (childXmlNode->nodeName == "Button")
            {
                auto btn = dd->addOption("");
                IRule::ruleMap_[childXmlNode->nodeName]->parseAndApply(btn.lock(), childXmlNode->attributes);
                IRule::ruleMap_[childXmlNode->nodeName]->construct(dd, childXmlNode);
            }
        }
    }

    // Unused if parent is already a Dropdown
    return dd;
    // return parent->getTypeId() != node::UIDropdown::typeId ? dd : nullptr;
}

auto DropdownRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) -> void
{
    const auto& ph = ParseHelper::get();
    node::UIDropdownPtr obj = utils::as<node::UIDropdown>(object);
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