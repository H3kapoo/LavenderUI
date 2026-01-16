#pragma once

#include "include/LavenderUI/Core/ViewParser/Rules/IRule.hpp"
#include "include/LavenderUI/Node/UIDropdown.hpp"

namespace lav::core
{
class SplitPaneRule : public IRule
{
public:
    auto construct(const RuleMap& ruleMap, const XmlNode& xmlNode,
        node::UIBasePtr parent, const bool shouldAddToParent) const -> node::UIBasePtr override;
    auto parseAndApply(node::UIBasePtr object,
        const XmlAttribVec& attribs) const -> void override;

private:
    utils::Logger log_{"SplitPaneRule"};
};
} // namespace lav::core