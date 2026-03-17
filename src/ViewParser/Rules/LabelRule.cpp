#include <LavenderUI/ViewParser/Rules/LabelRule.hpp>

#include <LavenderUI/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/ViewParser/ParseHelpers.hpp>
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
    node::UILabelPtr label = utils::as<node::UILabel>(object);
    for (const auto&[key, value] : attribs)
    {
        if (key == "scale")
        {
            label->getBaseLayoutData().setScale(ph.toScale(value));
        }
        else if (key == "text")
        {
            label->setText(value);
        }
        else if (key == "vid")
        {
            label->setViewId(value);
        }
    }
}
} // namespace lav::core