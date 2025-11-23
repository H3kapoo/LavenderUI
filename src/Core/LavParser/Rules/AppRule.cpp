#include "AppRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Node/UIWindow.hpp"

namespace lav::core
{
auto AppRule::getRule() const -> RuleSignature
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
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
                size = parseHelper_.toVec2D(value);
            }
        }

        node::UIWindowPtr obj = utils::make<node::UIWindow>(title, size);
        for (const auto&[key, value] : attribs)
        {
            if (key == "ori" || key == "orientation")
            {
                obj->getBaseLayoutData().setType(parseHelper_.toOrientation(value));
            }
        }

        return obj;
    };
}
} // namespace lav::core