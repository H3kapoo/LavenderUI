#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <xml/HkXml.hpp>
#include <LavenderUI/Node/UIBase.hpp>
#include <LavenderUI/Utils/Logger.hpp>
#include <LavenderUI/ViewParser/Rules/IRule.hpp>

namespace lav::core
{
class LavParser
{

public:
    static auto get() -> LavParser&;
    auto parseFromFile(const std::filesystem::path& path) -> node::UIBasePtrVec;
    auto setRule(const std::string& tag, IRuleUPtr rule) -> void;

private:
    LavParser();
    LavParser(const LavParser&) = delete;
    LavParser(LavParser&&) = delete;
    auto operator=(const LavParser&) -> LavParser& = delete;
    auto operator=(LavParser&&) -> LavParser& = delete;

    auto tryConstructTreeFromXmlNode(hk::XMLDecoder::NodeSPtr xmlNode) -> node::UIBasePtr;
    auto tryConstructUINode(hk::XMLDecoder::NodeSPtr xmlNode) -> node::UIBasePtr;

private:
    utils::Logger log_;
    std::unordered_map<std::string, std::unique_ptr<IRule>> ruleMap_;
};

} // namespace lav::core