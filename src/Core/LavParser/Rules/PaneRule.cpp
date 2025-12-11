#include "PaneRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Core/LavParser/ParseHelpers.hpp"
#include "src/Node/UIPane.hpp"

namespace lav::core
{
auto PaneRule::getRule() const -> IRule::RuleData
{
    return {getConstructRule(), getAdditionRule()};
}
auto PaneRule::getConstructRule() const -> ConstructRule
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        const auto& ph = ParseHelper::get();
        node::UIPanePtr obj = utils::make<node::UIPane>();
        for (const auto&[key, value] : attribs)
        {
            if (key == "scale")
            {
                obj->getBaseLayoutData().setScale(ph.toScale(value));
            }
            else if (key == "ori" || key == "orientation")
            {
                obj->getBaseLayoutData().setType(ph.toOrientation(value));
            }
            else if (key == "border")
            {
                obj->getBaseLayoutData().setBorder(ph.toBorder(value));
            }
            else if (key == "align")
            {
                obj->getBaseLayoutData().setAlign(ph.toAlign(value));
            }
            else if (key == "color")
            {
                obj->setColor(ph.toColor(value));
            }
            else if (key == "bcolor")
            {
                obj->setBorderColor(ph.toColor(value));
            }
            else if (key == "scroll" && value == "true")
            {
                obj->setScrollEnabled(true, true);
            }
        }
        return obj;
    };
}

auto PaneRule::getAdditionRule() const -> AddRule
{
    return [this](node::UIBasePtr parent, node::UIBasePtr child) -> void
    {
        parent->add(child);
    };
}
} // namespace lav::core