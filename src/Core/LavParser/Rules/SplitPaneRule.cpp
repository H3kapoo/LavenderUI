#include "SplitPaneRule.hpp"

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Core/LavParser/ParseHelpers.hpp"
#include "src/Node/UIPane.hpp"
#include "src/Node/UISplitPane.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::core
{
auto SplitPaneRule::getRule() const -> IRule::RuleData
{
    return {getConstructRule(), getAdditionRule()};
}

auto SplitPaneRule::getConstructRule() const -> ConstructRule
{
    return [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UISplitPanePtr
    {
        const auto& ph = ParseHelper::get();
        node::UISplitPanePtr obj = utils::make<node::UISplitPane>();
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
            else if (key == "color")
            {
                obj->setColor(ph.toColor(value));
            }
            else if (key == "bcolor")
            {
                obj->setBorderColor(ph.toColor(value));
            }
            else if (key == "ori" || key == "orientation")
            {
                obj->getBaseLayoutData().setType(ph.toOrientation(value));
            }
            else if (key == "splits")
            {
                obj->setSplitDistribution(ph.toRelVector(value));
            }
        }
        return obj;
    };
}

auto SplitPaneRule::getAdditionRule() const -> AddRule
{
    return [this](node::UIBasePtr parent, node::UIBasePtr child) -> void
    {
        auto castSplitPane = utils::as<node::UISplitPane>(parent);
        if (child->getTypeId() == node::UIPane::typeId)
        {
            auto castPane = utils::as<node::UIPane>(child);
            castSplitPane->createPane(std::move(castPane));
        }
        else if (child->getTypeId() == node::UISplitPane::typeId)
        {
            auto castSubsplit = utils::as<node::UISplitPane>(child);
            castSplitPane->createSubsplit(std::move(castSubsplit));
        }
    };
}
} // namespace lav::core