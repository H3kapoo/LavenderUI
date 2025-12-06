#pragma once

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Node/UIDropdown.hpp"

namespace lav::core
{
class DropdownRule : public IRule
{
public:
    auto getRule() const -> IRule::RuleData override;

private:
    auto getConstructRule() const -> ConstructRule;
    auto getAdditionRule() const -> AddRule;

    auto toOpenDir(const std::string& value) const -> node::UIDropdown::OpenDir;

};
} // namespace lav::core