#include "LavParser.hpp"

#include <cstdlib>
#include <fstream>

#include "src/Core/LavParser/Rules/ButtonRule.hpp"
#include "src/Core/LavParser/Rules/LabelRule.hpp"
#include "src/Core/LavParser/Rules/SliderRule.hpp"
#include "src/Core/LavParser/Rules/AppRule.hpp"
#include "src/Core/LavParser/Rules/PaneRule.hpp"
#include "src/Core/LavParser/Rules/ImageRule.hpp"
#include "src/Node/UIBase.hpp"
#include "vendor/xml/HkXml.hpp"


namespace lav::core
{
static constexpr std::string TITLE = "title";
static constexpr std::string SCALE = "scale";
static constexpr std::string SRC = "src";
static constexpr std::string TEXT = "text";
static constexpr std::string ORIENTATION = "orientation";
static constexpr std::string ORIENTATION_SHORT = "ori";
static constexpr std::string LAUNCH_SCALE = "launchScale";
static constexpr std::string SLIDER_DEFAULT = "default";
static constexpr std::string SLIDER_TO = "to";
static constexpr std::string SLIDER_FROM = "from";

auto LavParser::get() -> LavParser&
{
    static LavParser instance;
    return instance;
}

LavParser::LavParser()
{
    /* Initialize parser rules for each tag name. */
    setContructRule("App", AppRule().getRule());
    setContructRule("Pane", PaneRule().getRule());
    setContructRule("Img", ImageRule().getRule());
    setContructRule("Button", ButtonRule().getRule());
    setContructRule("Label", LabelRule().getRule());
    setContructRule("Slider", SliderRule().getRule());
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
    if (!res.second.empty())
    {
        log_.error("There was some error parsing XML: {}", res.second);
        return {};
    }

    // Assume just one element is possible as root
    hk::XMLDecoder::NodeSPtr xmlNode = res.first[0];
    node::UIBasePtr uiViewRoot;

    uiViewRoot = parseXmlTagData(xmlNode);
    if (!uiViewRoot)
    {
        log_.error("No root could be established. WHoops!");
        return {};
    }

    /* Transfer the elements after being attached to a "mock window" */
    /* This shall be enhanced later as we could load views that dont have a window as a root. It could
    be a "patch", a separate component to slap into some place. */
    // node::UIBasePtrVec elements{uiViewRoot->getElements().begin(), uiViewRoot->getElements().end()};
    // uiViewRoot->remove([](const auto&){ return true; });
    // return elements;
    log_.info("File has been parsed: '{}' !", path.string());
    return {uiViewRoot};
}

auto LavParser::parseXmlTagData(hk::XMLDecoder::NodeSPtr xmlNode) -> node::UIBasePtr
{
    const node::UIBasePtr uiParsedNode = parseSingleXmlTagData(xmlNode);
    if (!uiParsedNode)
    {
        log_.error("Could not parse node from xml data");
        return nullptr;
    }
    for (const auto& n : xmlNode->children)
    {
        uiParsedNode->add(parseXmlTagData(n));
    }

    return uiParsedNode;
}

auto LavParser::parseSingleXmlTagData(hk::XMLDecoder::NodeSPtr xmlNode) -> node::UIBasePtr
{
    if (constructRuleMap_.contains(xmlNode->nodeName))
    {
        log_.debug("Constructing '{}'...", xmlNode->nodeName);
        return constructRuleMap_[xmlNode->nodeName](xmlNode->attributes);
    }
    else
    {
        log_.error("Unknown node: '{}'", xmlNode->nodeName);
    }

    return nullptr;
}

auto LavParser::setContructRule(const std::string& tag, const RuleSignature& rule) -> void
{
    constructRuleMap_[tag] = rule;
}

} // namespace lav::core