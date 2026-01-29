#pragma once

#include <LavenderUI/Core/ViewParser/Rules/IRule.hpp>

namespace lav::core
{
class SliderRule : public IRule
{
public:
    auto construct(const RuleMap& ruleMap, const XmlNode& xmlNode,
        node::UIBasePtr parent, const bool shouldAddToParent) const -> node::UIBasePtr override;
    auto parseAndApply(node::UIBasePtr object,
        const XmlAttribVec& attribs) const -> void override;

private:
    utils::Logger log_{"PaneRule"};
};
} // namespace lav::core