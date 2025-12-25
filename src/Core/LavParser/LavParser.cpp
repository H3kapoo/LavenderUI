#include "LavParser.hpp"

#include <cstdlib>
#include <fstream>

#include "src/Core/LavParser/Rules/AppRule.hpp"
#include "src/Core/LavParser/Rules/ButtonRule.hpp"
#include "src/Core/LavParser/Rules/DropdownRule.hpp"
#include "src/Core/LavParser/Rules/IRule.hpp"
// #include "src/Core/LavParser/Rules/LabelRule.hpp"
// #include "src/Core/LavParser/Rules/SliderRule.hpp"
// #include "src/Core/LavParser/Rules/AppRule.hpp"
// #include "src/Core/LavParser/Rules/PaneRule.hpp"
// #include "src/Core/LavParser/Rules/ImageRule.hpp"
// #include "src/Core/LavParser/Rules/SplitPaneRule.hpp"
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
    // setRule("App", AppRule().getRule());
    // setRule("Pane", PaneRule().getRule());
    // setRule("Img", ImageRule().getRule());
    // setRule("Button", ButtonRule());
    // // setContructRule("Label", LabelRule().getRule());
    // setRule("Slider", SliderRule().getRule());
    // setRule("Dropdown", DropdownRule().getRule());
    // setRule("SplitPane", SplitPaneRule().getRule());

    IRule::ruleMap_["Button"] = std::make_unique<ButtonRule>();
    IRule::ruleMap_["App"] = std::make_unique<AppRule>();
    IRule::ruleMap_["Dropdown"] = std::make_unique<DropdownRule>();
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

    uiViewRoot = IRule::ruleMap_["App"]->construct(nullptr, xmlNode);
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

// auto LavParser::tryConstructTreeFromXmlNode(hk::XMLDecoder::NodeSPtr xmlNode) -> node::UIBasePtr
// {
//     node::UIBasePtr uiParentNode = tryConstructUINode(xmlNode);
//     if (!uiParentNode)
//     {
//         log_.error("Could not construct UI node from xml data!");
//         return nullptr;
//     }

//     for (const auto& childXmlNode : xmlNode->children)
//     {
//         const auto& childUiNode = tryConstructTreeFromXmlNode(childXmlNode);
//         ruleMap_[xmlNode->nodeName].addRule(uiParentNode, childUiNode);
//     }

//     return uiParentNode;
// }

// auto LavParser::tryConstructUINode(hk::XMLDecoder::NodeSPtr xmlNode) -> node::UIBasePtr
// {
//     if (ruleMap_.contains(xmlNode->nodeName))
//     {
//         // log_.debug("Constructing '{}'...", xmlNode->nodeName);
//         return ruleMap_[xmlNode->nodeName].constructRule(xmlNode->attributes);
//     }
//     else
//     {
//         log_.error("Unknown node: '{}'", xmlNode->nodeName);
//     }

//     return nullptr;
// }

// auto LavParser::setRule(const std::string& tag, const IRule::RuleData& ruleData) -> void
// {
//     ruleMap_[tag] = ruleData;
// }
} // namespace lav::core