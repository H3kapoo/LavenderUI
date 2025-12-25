#pragma once

#include "src/Core/LavParser/Rules/IRule.hpp"

namespace lav::core
{
class AppRule : public IRule
{
public:
    auto construct(node::UIBasePtr parent,
        const hk::XMLDecoder::NodeSPtr& xmlNode) -> node::UIBasePtr override;
    auto parseAndApply(node::UIBasePtr object,
        const hk::XMLDecoder::AttrPairVec& attribs) -> void override;
};
} // namespace lav::core