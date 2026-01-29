#include <LavenderUI/Core/ViewParser/Rules/LabelRule.hpp>

#include <LavenderUI/Core/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/Core/ViewParser/ParseHelpers.hpp>
#include <LavenderUI/Node/UILabel.hpp>

namespace lav::core
{
auto LabelRule::construct(const RuleMap&, const XmlNode& xmlNode,
    node::UIBasePtr, const bool) const -> node::UIBasePtr
{
    node::UILabelPtr label = utils::make<node::UILabel>();
    parseAndApply(label, xmlNode->attributes);
    return label;
}

auto LabelRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) const -> void
{
    const auto& ph = ParseHelper::get();
    node::UILabelPtr obj = utils::as<node::UILabel>(object);
    for (const auto&[key, value] : attribs)
    {
        if (key == "scale")
        {
            obj->getBaseLayoutData().setScale(ph.toScale(value));
        }
        else if (key == "text")
        {
            obj->setText(value);
        }
    }
}
} // namespace lav::core