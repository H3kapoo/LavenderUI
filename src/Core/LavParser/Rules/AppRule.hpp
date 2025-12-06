#pragma once

#include "src/Core/LavParser/Rules/IRule.hpp"

namespace lav::core
{
class AppRule : public IRule
{
public:
    auto getRule() const -> IRule::RuleData override;

private:
    auto getConstructRule() const -> ConstructRule;
    auto getAdditionRule() const -> AddRule;
};
} // namespace lav::core