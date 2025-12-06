#include "ButtonRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Core/LavParser/ParseHelpers.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Utils/Logger.hpp"

namespace lav::core
{
auto ButtonRule::getRule() const -> IRule::RuleData
{
    return {getConstructRule(), getAdditionRule()};
}

auto ButtonRule::getConstructRule() const -> ConstructRule
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        const auto& ph = ParseHelper::get();
        node::UIButtonPtr obj = utils::make<node::UIButton>();
        for (const auto&[key, value] : attribs)
        {
            if (key == "scale")
            {
                obj->getBaseLayoutData().setScale(ph.toScale(value));
            }
            else if (key == "border")
            {
                obj->getBaseLayoutData().setBorder(ph.toBorder(value));
            }
            else if (key == "text")
            {
                obj->setText(value);
            }
        }
        return obj;
    };
}

auto ButtonRule::getAdditionRule() const -> AddRule
{
    return [this](node::UIBasePtr parent, node::UIBasePtr child) -> void
    {
        // parent->add(elements);
    };
}
} // namespace lav::core