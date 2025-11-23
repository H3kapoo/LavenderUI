#pragma once

#include <functional>

#include "src/Node/UIBase.hpp"
#include "vendor/xml/HkXml.hpp"

namespace lav::core
{
using RuleSignature = std::function<node::UIBasePtr(const hk::XMLDecoder::AttrPairVec& attribs)>;

class IRule
{
public:
    virtual ~IRule() = default;
    virtual auto getRule() const -> RuleSignature = 0;
};
} // namespace lav::core