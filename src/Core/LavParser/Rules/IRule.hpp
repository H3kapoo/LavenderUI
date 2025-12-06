#pragma once

#include <functional>

#include "src/Node/UIBase.hpp"
#include "vendor/xml/HkXml.hpp"

namespace lav::core
{
using ConstructRule = std::function<node::UIBasePtr(const hk::XMLDecoder::AttrPairVec& attribs)>;
using AddRule = std::function<void(node::UIBasePtr parent, node::UIBasePtr child)>;

class IRule
{
public:
    struct RuleData
    {
        ConstructRule constructRule;
        AddRule addRule;
    };

public:
    virtual ~IRule() = default;
    virtual auto getRule() const -> RuleData = 0;
};
} // namespace lav::core