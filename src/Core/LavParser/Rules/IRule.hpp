#pragma once

#include <functional>

#include "src/Node/UIBase.hpp"
#include "vendor/xml/HkXml.hpp"

namespace lav::core
{
class IRule
{
public:
    virtual ~IRule() = default;
    virtual auto construct(node::UIBasePtr parent,
        const hk::XMLDecoder::NodeSPtr& xmlNode) -> node::UIBasePtr = 0;
    virtual auto parseAndApply(node::UIBasePtr object,
        const hk::XMLDecoder::AttrPairVec& attribs) -> void = 0;

public:
    static inline std::unordered_map<std::string, std::unique_ptr<IRule>> ruleMap_ = {};
};
} // namespace lav::core