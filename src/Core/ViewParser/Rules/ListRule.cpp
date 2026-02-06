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
    node::UIListPtr obj = utils::as<node::UIList>(object);
    for (const auto&[key, value] : attribs)
    {
        if (key == "scale")
        {
            obj->getBaseLayoutData().setScale(ph.toScale(value));
        }
        else if (key == "color")
        {
            obj->setColor(ph.toColor(value));
        }
    }
}
} // namespace lav::core