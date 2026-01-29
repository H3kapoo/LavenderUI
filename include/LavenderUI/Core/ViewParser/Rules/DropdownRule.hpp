#pragma once

#include <LavenderUI/Core/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/Node/UIDropdown.hpp>

namespace lav::core
{
class DropdownRule : public IRule
{
public:
    auto construct(const RuleMap& ruleMap, const XmlNode& xmlNode,
        node::UIBasePtr parent, const bool shouldAddToParent) const -> node::UIBasePtr override;
    auto parseAndApply(node::UIBasePtr object,
        const XmlAttribVec& attribs) const -> void override;

private:
    auto toOpenDir(const std::string& value) const -> node::UIDropdown::OpenDir;

private:
    utils::Logger log_{"DropdownRule"};
};
} // namespace lav::core