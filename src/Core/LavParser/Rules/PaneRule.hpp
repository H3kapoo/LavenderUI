#pragma once

#include "src/Core/LavParser/ParseHelpers.hpp"
#include "src/Core/LavParser/Rules/IRule.hpp"

namespace lav::core
{
class PaneRule : public IRule
{
public:
    auto getRule() const -> RuleSignature;

private:
    ParseHelper parseHelper_;
};
} // namespace lav::core