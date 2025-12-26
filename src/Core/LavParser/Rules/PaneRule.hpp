#pragma once

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Utils/Logger.hpp"

namespace lav::core
{
class PaneRule : public IRule
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