#include <LavenderUI/ViewParser/Rules/LineEditRule.hpp>

#include <LavenderUI/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/ViewParser/ParseHelpers.hpp>
#include <LavenderUI/Node/UILineEdit.hpp>

namespace lav::core
{
auto LineEditRule::construct(const RuleMap&, const XmlNode& xmlNode,
    node::UIBasePtr, const bool) const -> node::UIBasePtr
{
    node::UILineEditPtr le = utils::make<node::UILineEdit>();
    parseAndApply(le, xmlNode->attributes);
    return le;
}

auto LineEditRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) const -> void
{
    const auto& ph = ParseHelper::get();
    node::UILineEditPtr le = utils::as<node::UILineEdit>(object);
    for (const auto&[key, value] : attribs)
    {
        if (key == "scale")
        {
            le->getBaseLayoutData().setScale(ph.toScale(value));
        }
        else if (key == "color")
        {
            le->setColor(ph.toColor(value));
        }
        else if (key == "text")
        {
            le->setText(value);
        }
        else if (key == "vid")
        {
            le->setViewId(value);
        }
        else if (key == "numeric" && value == "true")
        {
            le->setNumbericOnly(true);
        }
    }
}
} // namespace lav::core