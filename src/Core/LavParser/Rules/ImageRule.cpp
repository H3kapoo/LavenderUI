#include "ImageRule.hpp"

#include "src/Core/LavParser/ParseHelpers.hpp"
#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Node/UIImage.hpp"

namespace lav::core
{
auto ImageRule::getRule() const -> IRule::RuleData
{
    return {getConstructRule(), getAdditionRule()};
}

auto ImageRule::getConstructRule() const -> ConstructRule
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        const auto& ph = ParseHelper::get();
        node::UIImagePtr obj = utils::make<node::UIImage>();
        for (const auto&[key, value] : attribs)
        {
            if (key == "scale")
            {
                obj->getBaseLayoutData().setScale(ph.toScale(value));
            }
            else if (key == "src")
            {
                obj->setImage(value);
            }
        }
        // obj->getBaseLayoutData().setMargin(10);
        return obj;
    };
}

auto ImageRule::getAdditionRule() const -> AddRule
{
    return [this](node::UIBasePtr parent, node::UIBasePtr child) -> void
    {
        (void)parent;
        (void)child;
        // parent->add(child);
    };
}
} // namespace lav::core