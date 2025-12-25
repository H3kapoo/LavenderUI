#include "AppRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Core/LavParser/ParseHelpers.hpp"
#include "src/Node/UIWindow.hpp"

namespace lav::core
{
auto AppRule::construct(node::UIBasePtr parent,
    const hk::XMLDecoder::NodeSPtr& xmlNode) -> node::UIBasePtr
{
    node::UIWindowPtr win = utils::make<node::UIWindow>("Test", glm::ivec2{1280, 720});
    parseAndApply(win, xmlNode->attributes);

    for (const auto& childXmlNode : xmlNode->children)
    {
        auto child = IRule::ruleMap_[childXmlNode->nodeName]->construct(win, childXmlNode);
        win->add(child);
    }

    return win;
}

auto AppRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) -> void
{
    const auto& ph = ParseHelper::get();
    node::UIWindowPtr win = utils::as<node::UIWindow>(object);

    std::string title;
    glm::ivec2 size;
    for (const auto&[key, value] : attribs)
    {
        if (key == "title")
        {
            title = value;
        }
        else if (key == "launchScale")
        {
            size = ph.toVec2D(value);
        }
    }
    // win.setTitle..
    // win.setSize...

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
    }
}
} // namespace lav::core