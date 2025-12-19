#pragma once

#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Node/UIDropdown.hpp"

namespace lav::core
{
class SplitPaneRule : public IRule
{
public:
    auto getRule() const -> IRule::RuleData override;

private:
    auto getConstructRule() const -> ConstructRule;
    auto getAdditionRule() const -> AddRule;
};
} // namespace lav::core