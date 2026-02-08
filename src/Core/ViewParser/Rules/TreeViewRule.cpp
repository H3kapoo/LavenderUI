#include <LavenderUI/Core/ViewParser/Rules/TreeViewRule.hpp>

#include <LavenderUI/Core/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/Core/ViewParser/ParseHelpers.hpp>
#include <LavenderUI/Node/UITreeView.hpp>

namespace lav::core
{
auto TreeViewRule::construct(const RuleMap&, const XmlNode& xmlNode,
    node::UIBasePtr, const bool) const -> node::UIBasePtr
{
    node::UITreeViewPtr label = utils::make<node::UITreeView>();
    parseAndApply(label, xmlNode->attributes);
    return label;
}

auto TreeViewRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) const -> void
{
    const auto& ph = ParseHelper::get();
    node::UITreeViewPtr tv = utils::as<node::UITreeView>(object);
    for (const auto&[key, value] : attribs)
    {
        if (key == "scale")
        {
            tv->getBaseLayoutData().setScale(ph.toScale(value));
        }
        else if (key == "color")
        {
            tv->setColor(ph.toColor(value));
        }
        else if (key == "vid")
        {
            tv->setViewId(value);
        }
    }
}
} // namespace lav::core