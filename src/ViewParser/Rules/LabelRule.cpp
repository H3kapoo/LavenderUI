#include "LavenderUI/Core/Config.hpp"
#include "LavenderUI/Core/TextHandler/Common.hpp"
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
        else if (key == "wrap")
        {
            if (value == "char" || value == "c")
            {
                label->setTextWrap(core::TextOptions::Wrap::CHAR);
            }
            else if (value == "word" || value == "w")
            {
                label->setTextWrap(core::TextOptions::Wrap::WORD);
            }
        }
        else if (key == "ellipsis")
        {
            const int32_t val = ph.toNumber(value);
            label->setTextEllipsis(val < 2 ? 0 : val);
        }
        else if (key == "align")
        {
            // TODO: Create custom func
            if (value == "c")
            {
                label->setTextAlign(core::TextOptions::Align::CENTER);
            }
        }
        else if (key == "font")
        {
            label->setFont(core::Config::fontsPath / value);
        }
    }
}
} // namespace lav::core