#include "ImageRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Node/UIImage.hpp"

namespace lav::core
{
auto ImageRule::getRule() const -> RuleSignature
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        node::UIImagePtr obj = utils::make<node::UIImage>();
        for (const auto&[key, value] : attribs)
        {
            if (key == "scale")
            {
                obj->getBaseLayoutData().setScale(parseHelper_.toScale(value));
            }
            else if (key == "src")
            {
                obj->setImage(value);
            }
        }
        return obj;
    };
}
} // namespace lav::core