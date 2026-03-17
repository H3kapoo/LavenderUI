#include <LavenderUI/ViewParser/Rules/ListViewRule.hpp>

#include <LavenderUI/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/ViewParser/ParseHelpers.hpp>
#include <LavenderUI/Node/UIListView.hpp>

namespace lav::core
{
auto ListViewRule::construct(const RuleMap&, const XmlNode& xmlNode,
    node::UIBasePtr, const bool) const -> node::UIBasePtr
{
    node::UIListViewPtr list = utils::make<node::UIListView>();
    parseAndApply(list, xmlNode->attributes);
    return list;
}

auto ListViewRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) const -> void
{
    const auto& ph = ParseHelper::get();
    node::UIListViewPtr list = utils::as<node::UIListView>(object);
    for (const auto&[key, value] : attribs)
    {
        if (key == "scale")
        {
            list->getBaseLayoutData().setScale(ph.toScale(value));
        }
        else if (key == "color")
        {
            list->setColor(ph.toColor(value));
        }
        else if (key == "vid")
        {
            list->setViewId(value);
        }
    }
}
} // namespace lav::core