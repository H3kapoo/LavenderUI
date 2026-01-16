#include "include/LavenderUI/Core/ViewParser/Rules/ButtonRule.hpp"

#include "include/LavenderUI/Core/ViewParser/Rules/IRule.hpp"
#include "include/LavenderUI/Core/ViewParser/ParseHelpers.hpp"
#include "include/LavenderUI/Node/UIBase.hpp"
#include "include/LavenderUI/Node/UIButton.hpp"
#include "include/LavenderUI/Utils/Logger.hpp"

namespace lav::core
{
auto ButtonRule::construct(const RuleMap&, const XmlNode& xmlNode,
    node::UIBasePtr, const bool) const -> node::UIBasePtr
{
    node::UIButtonPtr btn = utils::make<node::UIButton>();
    parseAndApply(btn, xmlNode->attributes);
    return btn;
}

auto ButtonRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) const -> void
{
    const auto& ph = ParseHelper::get();
    node::UIButtonPtr btn = utils::as<node::UIButton>(object);
    for (const auto&[key, value] : attribs)
    {
        if (key == "scale")
        {
            btn->getBaseLayoutData().setScale(ph.toScale(value));
        }
        else if (key == "border")
        {
            btn->getBaseLayoutData().setBorder(ph.toBorder(value));
        }
        else if (key == "text")
        {
            btn->setText(value);
        }
    }
}
} // namespace lav::core