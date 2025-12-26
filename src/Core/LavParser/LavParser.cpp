#include "LavParser.hpp"

#include <cstdlib>
#include <fstream>

#include "src/Core/LavParser/Rules/AppRule.hpp"
#include "src/Core/LavParser/Rules/ButtonRule.hpp"
#include "src/Core/LavParser/Rules/DropdownRule.hpp"
#include "src/Core/LavParser/Rules/IRule.hpp"
#include "src/Core/LavParser/Rules/LabelRule.hpp"
#include "src/Core/LavParser/Rules/ImageRule.hpp"
#include "src/Core/LavParser/Rules/PaneRule.hpp"
#include "src/Core/LavParser/Rules/SliderRule.hpp"
#include "src/Core/LavParser/Rules/SplitPaneRule.hpp"
#include "src/Node/UIBase.hpp"
#include "vendor/xml/HkXml.hpp"


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