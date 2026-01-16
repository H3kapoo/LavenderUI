#pragma once

#include "include/LavenderUI/Core/ViewParser/Rules/IRule.hpp"

namespace lav::core
{
class LabelRule : public IRule
{
public:
    auto construct(const RuleMap& ruleMap, const XmlNode& xmlNode,
        node::UIBasePtr parent, const bool shouldAddToParent) const -> node::UIBasePtr override;
    auto parseAndApply(node::UIBasePtr object,
        const hk::XMLDecoder::AttrPairVec& attribs) const -> void override;
};
} // namespace lav::core