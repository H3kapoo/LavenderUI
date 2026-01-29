#include <LavenderUI/Core/ViewParser/LavParser.hpp>

#include <cstdlib>
#include <fstream>

#include <xml/HkXml.hpp>
#include <LavenderUI/Core/ViewParser/Rules/AppRule.hpp>
#include <LavenderUI/Core/ViewParser/Rules/ButtonRule.hpp>
#include <LavenderUI/Core/ViewParser/Rules/DropdownRule.hpp>
#include <LavenderUI/Core/ViewParser/Rules/IRule.hpp>
#include <LavenderUI/Core/ViewParser/Rules/LabelRule.hpp>
#include <LavenderUI/Core/ViewParser/Rules/ImageRule.hpp>
#include <LavenderUI/Core/ViewParser/Rules/PaneRule.hpp>
#include <LavenderUI/Core/ViewParser/Rules/SliderRule.hpp>
#include <LavenderUI/Core/ViewParser/Rules/SplitPaneRule.hpp>
#include <LavenderUI/Node/UIBase.hpp>

namespace lav::core
{
auto LavParser::get() -> LavParser&
{
    static LavParser instance;
    return instance;
}

LavParser::LavParser()
    : log_(__func__)
{
    /* Initialize parser rules for each tag name. */
    setRule("App", std::move(std::make_unique<AppRule>()));
    setRule("Img", std::move(std::make_unique<ImageRule>()));
    setRule("Pane", std::move(std::make_unique<PaneRule>()));
    setRule("Button", std::move(std::make_unique<ButtonRule>()));
    setRule("Slider", std::move(std::make_unique<SliderRule>()));
    setRule("Dropdown", std::move(std::make_unique<DropdownRule>()));
    setRule("SplitPane", std::move(std::make_unique<SplitPaneRule>()));
    setRule("Label", std::move(std::make_unique<LabelRule>()));
    // setRule("Pane", PaneRule().getRule());
    // setRule("Img", ImageRule().getRule());
    // setRule("Button", ButtonRule());
    // // setContructRule("Label", LabelRule().getRule());
    // setRule("Slider", SliderRule().getRule());
    // setRule("Dropdown", DropdownRule().getRule());
    // setRule("SplitPane", SplitPaneRule().getRule());
}

auto LavParser::parseFromFile(const std::filesystem::path& path) -> node::UIBasePtrVec
{
    std::ifstream xmlFile(path);
    if (xmlFile.fail())
    {
        log_.error("Failed to find/open '{}'", path.string());
        return {};
    }

    hk::XMLDecoder::XmlResult res = hk::XMLDecoder().decodeFromStream(xmlFile);
    //TODO: Bug in XML decoder when we dont close the tag with children?
    if (!res.second.empty())
    {
        log_.error("There was some error parsing XML: {}", res.second);
        return {};
    }

    // Assume just one element is possible as root
    hk::XMLDecoder::NodeSPtr xmlNode = res.first[0];
    node::UIBasePtr uiViewRoot;

    if (!ruleMap_.contains("App"))
    {
        log_.error("Rule {} doesn't exist!", "App");
        return {};
    }

    uiViewRoot = ruleMap_["App"]->construct(ruleMap_, xmlNode, nullptr, false);
    if (!uiViewRoot)
    {
        log_.error("No root could be established. WHoops!");
        return {};
    }

    log_.info("File has been parsed: '{}' !", path.string());
    return {uiViewRoot};
}

auto LavParser::setRule(const std::string& tag, IRuleUPtr rule) -> void
{
    ruleMap_[tag] = std::move(rule);
}
} // namespace lav::core