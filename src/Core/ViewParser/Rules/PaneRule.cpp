#include <LavenderUI/Core/ViewParser/Rules/PaneRule.hpp>

#include <LavenderUI/Core/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/Core/ViewParser/ParseHelpers.hpp>
#include <LavenderUI/Node/UIBase.hpp>
#include <LavenderUI/Node/UIPane.hpp>

namespace lav::core
{
auto PaneRule::construct(const RuleMap& ruleMap, const XmlNode& xmlNode,
    node::UIBasePtr parent, const bool shouldAddToParent) const -> node::UIBasePtr
{
    node::UIPanePtr pane;
    if (!shouldAddToParent)
    {
        pane = utils::make<node::UIPane>();
        parseAndApply(pane, xmlNode->attributes);
    }

    for (const auto& childXmlNode : xmlNode->children)
    {

        const auto& nName = childXmlNode->nodeName;
        if (!ruleMap.contains(nName))
        {
            log_.error("No rule to parse '{}' found!", nName);
            continue;
        }

        if (shouldAddToParent)
        {
            auto newUINode = ruleMap.at(nName)->construct(ruleMap, childXmlNode, parent, true);
            utils::as<node::UIPane>(parent)->add(newUINode);
        }
        else
        {
            auto newUINode = ruleMap.at(nName)->construct(ruleMap, childXmlNode, pane, false);
            pane->add(newUINode);
        }
    }

    return pane;
}

auto PaneRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) const -> void
{
    const auto& ph = ParseHelper::get();
    node::UIPanePtr pane = utils::as<node::UIPane>(object);
    for (const auto&[key, value] : attribs)
    {
        if (key == "scale")
        {
            pane->getBaseLayoutData().setScale(ph.toScale(value));
        }
        else if (key == "ori" || key == "orientation")
        {
            pane->getBaseLayoutData().setType(ph.toOrientation(value));
        }
        else if (key == "border")
        {
            pane->getBaseLayoutData().setBorder(ph.toBorder(value));
        }
        else if (key == "align")
        {
            pane->getBaseLayoutData().setAlign(ph.toAlign(value));
        }
        else if (key == "color")
        {
            pane->setColor(ph.toColor(value));
        }
        else if (key == "bcolor")
        {
            pane->setBorderColor(ph.toColor(value));
        }
        else if (key == "scroll" && value == "true")
        {
            pane->setScrollEnabled(true, true);
        }
    }
}
} // namespace lav::core