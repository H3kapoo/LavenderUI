#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "src/Core/LavParser/ParseHelpers.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Utils/Logger.hpp"
#include "Rules/IRule.hpp"
#include "vendor/xml/HkXml.hpp"

namespace lav::core
{
class LavParser
{

public:
    static auto get() -> LavParser&;
    auto parseFromFile(const std::filesystem::path& path) -> node::UIBasePtrVec;
    auto setRule(const std::string& tag, const IRule::RuleData& ruleData) -> void;

private:
    LavParser();
    LavParser(const LavParser&) = delete;
    LavParser(LavParser&&) = delete;
    auto operator=(const LavParser&) -> LavParser& = delete;
    auto operator=(LavParser&&) -> LavParser& = delete;

    auto tryConstructTreeFromXmlNode(hk::XMLDecoder::NodeSPtr xmlNode) -> node::UIBasePtr;
    auto tryConstructUINode(hk::XMLDecoder::NodeSPtr xmlNode) -> node::UIBasePtr;

private:
    utils::Logger log_{"LavParser"};
    std::unordered_map<std::string, IRule::RuleData> ruleMap_;
};

} // namespace lav::core