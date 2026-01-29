#include <LavenderUI/Core/ViewParser/Rules/ImageRule.hpp>

#include <LavenderUI/Core/ViewParser/ParseHelpers.hpp>
#include <LavenderUI/Core/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/Node/UIImage.hpp>

namespace lav::core
{
auto ImageRule::construct(const RuleMap&, const XmlNode& xmlNode,
    node::UIBasePtr, const bool) const -> node::UIBasePtr
{
    node::UIImagePtr img = utils::make<node::UIImage>();
    parseAndApply(img, xmlNode->attributes);
    return img;
}

auto ImageRule::parseAndApply(node::UIBasePtr object,
    const hk::XMLDecoder::AttrPairVec& attribs) const -> void
{
    const auto& ph = ParseHelper::get();
    node::UIImagePtr obj = utils::as<node::UIImage>(object);
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
}
} // namespace lav::core