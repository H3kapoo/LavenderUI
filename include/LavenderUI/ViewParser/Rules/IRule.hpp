#pragma once

#include <xml/HkXml.hpp>
#include <LavenderUI/Node/UIBase.hpp>

namespace lav::core
{
class IRule;
using IRuleUPtr = std::unique_ptr<IRule>;
using RuleMap = std::unordered_map<std::string, IRuleUPtr>;
using XmlNode = hk::XMLDecoder::NodeSPtr;
using XmlNodeVec = hk::XMLDecoder::NodeVec;
using XmlAttribVec = hk::XMLDecoder::AttrPairVec;
class IRule
{
public:
    virtual ~IRule() = default;
    virtual auto construct(const RuleMap& ruleMap, const XmlNode& xmlNode,
    node::UIBasePtr parent, const bool shouldAddToParent) const -> node::UIBasePtr = 0;
    virtual auto parseAndApply(node::UIBasePtr object, const XmlAttribVec& attribs) const -> void = 0;
};
} // namespace lav::core