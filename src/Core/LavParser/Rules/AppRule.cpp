#include "AppRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Core/LavParser/ParseHelpers.hpp"
#include "src/Node/UIWindow.hpp"

namespace lav::core
{
auto AppRule::getRule() const -> IRule::RuleData
{
    return { getConstructRule(), getAdditionRule() };
}

auto AppRule::getConstructRule() const -> ConstructRule
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        const auto& ph = ParseHelper::get();

        std::string title;
        glm::ivec2 size;
        for (const auto&[key, value] : attribs)
        {
            if (key == "title")
            {
                title = value;
            }
            else if (key == "launchScale")
            {
                size = ph.toVec2D(value);
            }
        }

        node::UIWindowPtr obj = utils::make<node::UIWindow>(title, size);
        for (const auto&[key, value] : attribs)
        {
            if (key == "ori" || key == "orientation")
            {
                obj->getBaseLayoutData().setType(ph.toOrientation(value));
            }
            else if (key == "align")
            {
                obj->getBaseLayoutData().setAlign(ph.toAlign(value));
            }
        }

        return obj;
    };
}

auto AppRule::getAdditionRule() const -> AddRule
{
    return [this](node::UIBasePtr parent, node::UIBasePtr child) -> void
    {
        parent->add(child);
    };
}
} // namespace lav::core