#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Utils/Logger.hpp"
#include "vendor/xml/HkXml.hpp"

namespace lav::core
{
class LavParser
{
using RuleSignature = std::function<node::UIBasePtr(const hk::XMLDecoder::AttrPairVec& attribs)>;

public:
    static auto get() -> LavParser&;
    auto parseFromFile(const std::filesystem::path& path) -> node::UIBasePtrVec;
    auto setContructRule(const std::string& tag, const RuleSignature& rule) -> void;

private:
    LavParser();
    LavParser(const LavParser&) = delete;
    LavParser(LavParser&&) = delete;
    auto operator=(const LavParser&) -> LavParser& = delete;
    auto operator=(LavParser&&) -> LavParser& = delete;

    auto parseXmlTagData(hk::XMLDecoder::NodeSPtr xmlNode) -> node::UIBasePtr;
    auto parseSingleXmlTagData(hk::XMLDecoder::NodeSPtr xmlNode) -> node::UIBasePtr;

    auto parseScale(const std::string& value) const -> LayoutBase::ScaleXY;
    auto parseNumber(const std::string& value) const -> float;
    auto parseVec2D(const std::string& value) const -> glm::ivec2;
    auto parseOrientation(const std::string& value) const -> LayoutBase::Type;

private:
    utils::Logger log_{"LavParser"};
    std::unordered_map<std::string, RuleSignature> constructRuleMap_;
};

} // namespace lav::core