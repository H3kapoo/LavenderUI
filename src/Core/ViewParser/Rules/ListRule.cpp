#include <LavenderUI/Core/ViewParser/Rules/ListRule.hpp>

#include <LavenderUI/Core/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/Core/ViewParser/ParseHelpers.hpp>
#include <LavenderUI/Node/UIList.hpp>

namespace lav::core
{
auto ListRule::construct(const RuleMap&, const XmlNode& xmlNode,
    node::UIBasePtr, const bool) const -> node::UIBasePtr
{
    node::UIListPtr label = utils::make<node::UIList>();
    parseAndApply(label, xmlNode->attributes);
    return label;
}

auto ListRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) const -> void
{
    const auto& ph = ParseHelper::get();
    node::UIListPtr list = utils::as<node::UIList>(object);
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