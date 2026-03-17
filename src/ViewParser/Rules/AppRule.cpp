#include <LavenderUI/ViewParser/Rules/AppRule.hpp>

#include <LavenderUI/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/ViewParser/ParseHelpers.hpp>
#include <LavenderUI/Node/UIBase.hpp>
#include <LavenderUI/Node/UIWindow.hpp>

namespace lav::core
{
auto AppRule::construct(const RuleMap& ruleMap, const XmlNode& xmlNode,
    node::UIBasePtr, const bool) const -> node::UIBasePtr
{
    node::UIWindowPtr win = utils::make<node::UIWindow>("Test", glm::ivec2{1280, 720});
    parseAndApply(win, xmlNode->attributes);

    for (const auto& childXmlNode : xmlNode->children)
    {
        const auto& nName = childXmlNode->nodeName;
        if (!ruleMap.contains(nName))
        {
            log_.error("No rule to parse '{}' found!", nName);
            continue;
        }

        auto newUINode = ruleMap.at(nName)->construct(ruleMap, childXmlNode, win, false);
        win->add(newUINode);
    }

    return win;
}

auto AppRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) const -> void
{
    const auto& ph = ParseHelper::get();
    node::UIWindowPtr win = utils::as<node::UIWindow>(object);

    for (const auto&[key, value] : attribs)
    {
        if (key == "title")
        {
            win->setTitle(value);
        }
        else if (key == "launchSize")
        {
            if (value == "+")
            {
                win->setFullScreen();
            }
            else
            {
                win->setSize(ph.toVec2D(value));
            }
        }
    }

    for (const auto&[key, value] : attribs)
    {
        if (key == "ori" || key == "orientation")
        {
            win->getBaseLayoutData().setType(ph.toOrientation(value));
        }
        else if (key == "align")
        {
            win->getBaseLayoutData().setAlign(ph.toAlign(value));
        }
        else if (key == "vid")
        {
            win->setViewId(value);
        }
        else if (key == "launchPos")
        {
            if (value == "center" || value == "Center")
            {
                win->setPosition(win->getWindowMonitorSize() / 2 - win->getSize() / 2);
            }
            else
            {
                win->setPosition(ph.toVec2D(value));
            }
        }
    }
}
} // namespace lav::core