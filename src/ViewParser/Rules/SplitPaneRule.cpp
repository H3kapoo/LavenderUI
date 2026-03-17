#include <LavenderUI/ViewParser/Rules/SplitPaneRule.hpp>

#include <LavenderUI/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/ViewParser/ParseHelpers.hpp>
#include <LavenderUI/Node/UIPane.hpp>
#include <LavenderUI/Node/UISplitPane.hpp>
#include <LavenderUI/Utils/Misc.hpp>

namespace lav::core
{
auto SplitPaneRule::construct(const RuleMap& ruleMap, const XmlNode& xmlNode,
    node::UIBasePtr parent, const bool shouldAddToParent) const -> node::UIBasePtr
{
    node::UISplitPanePtr newSplitPane{nullptr};
    if (!shouldAddToParent)
    {
        newSplitPane = utils::make<node::UISplitPane>();
        parseAndApply(newSplitPane, xmlNode->attributes);
    }

    if (!ruleMap.contains("Pane"))
    {
        log_.error("No rule for '{}' found! This is mandatory for SplitPane!", "Pane");
        return newSplitPane;
    }

    node::UISplitPanePtr castParent{nullptr};
    for (const auto& childXmlNode : xmlNode->children)
    {
        if (shouldAddToParent && childXmlNode->nodeName == "SplitPane")
        {
            castParent = utils::as<node::UISplitPane>(parent);
            auto ss = castParent->createSubsplit(1.0f, {30, 10'000}).lock();
            ruleMap.at(childXmlNode->nodeName)->parseAndApply(ss, childXmlNode->attributes);
            ruleMap.at(childXmlNode->nodeName)->construct(ruleMap, childXmlNode, ss, true);
        }
        else if (shouldAddToParent && childXmlNode->nodeName == "Pane")
        {
            castParent = utils::as<node::UISplitPane>(parent);
            auto pane = castParent->createPane(1.0f, {30, 10'000}).lock();
            ruleMap.at(childXmlNode->nodeName)->parseAndApply(pane, childXmlNode->attributes);
            ruleMap.at(childXmlNode->nodeName)->construct(ruleMap, childXmlNode, pane, true);
        }
        else if (!shouldAddToParent && childXmlNode->nodeName == "SplitPane")
        {
            auto ss = newSplitPane->createSubsplit(1.0f, {30, 10'000}).lock();
            ruleMap.at(childXmlNode->nodeName)->parseAndApply(ss, childXmlNode->attributes);
            ruleMap.at(childXmlNode->nodeName)->construct(ruleMap, childXmlNode, ss, true);
        }
        else if (!shouldAddToParent && childXmlNode->nodeName == "Pane")
        {
            auto pane = newSplitPane->createPane(1.0f, {30, 10'000}).lock();
            ruleMap.at(childXmlNode->nodeName)->parseAndApply(pane, childXmlNode->attributes);
            ruleMap.at(childXmlNode->nodeName)->construct(ruleMap, childXmlNode, pane, true);
        }
    }

    return newSplitPane;
}

auto SplitPaneRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) const -> void
{
    const auto& ph = ParseHelper::get();
    node::UISplitPanePtr sp = utils::as<node::UISplitPane>(object);
    for (const auto&[key, value] : attribs)
    {
        if (key == "scale")
        {
            sp->getBaseLayoutData().setScale(ph.toScale(value));
        }
        else if (key == "border")
        {
            sp->getBaseLayoutData().setBorder(ph.toBorder(value));
        }
        else if (key == "color")
        {
            sp->setColor(ph.toColor(value));
        }
        else if (key == "bcolor")
        {
            sp->setBorderColor(ph.toColor(value));
        }
        else if (key == "ori" || key == "orientation")
        {
            sp->getBaseLayoutData().setType(ph.toOrientation(value));
        }
        else if (key == "splits")
        {
            //TBD
            // sp->setSplitDistribution(ph.toRelVector(value));
        }
        else if (key == "vid")
        {
            sp->setViewId(value);
        }
    }
}
} // namespace lav::core