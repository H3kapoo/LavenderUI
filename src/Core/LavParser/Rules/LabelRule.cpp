#include "LabelRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Node/UILabel.hpp"

namespace lav::core
{
auto LabelRule::getRule() const -> RuleSignature
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        node::UILabelPtr obj = utils::make<node::UILabel>();
        for (const auto&[key, value] : attribs)
        {
            if (key == "scale")
            {
                obj->getBaseLayoutData().setScale(parseHelper_.toScale(value));
            }
            else if (key == "text")
            {
                obj->setText(value);
            }
        }
        return obj;
    };
}
} // namespace lav::core