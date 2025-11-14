#include "LavParser.hpp"

#include <cstdlib>
#include <fstream>
#include <regex>

#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIImage.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Node/UIPane.hpp"
#include "src/Node/UISlider.hpp"
#include "src/Node/UIWindow.hpp"
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
    setContructRule("App", [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        std::string title;
        glm::ivec2 size;
        for (const auto&[key, value] : attribs)
        {
            if (key == TITLE) { title = value; }
            else if (key == LAUNCH_SCALE) { size = parseVec2D(value); }
        }

        node::UIWindowPtr obj = utils::make<node::UIWindow>(title, size);

        for (const auto&[key, value] : attribs)
        {
            if (key == ORIENTATION_SHORT) { obj->getBaseLayoutData().setType(parseOrientation(value)); }
            else if (key == ORIENTATION) { obj->getBaseLayoutData().setType(parseOrientation(value)); }
        }

        return obj;
    });

    setContructRule("Img", [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        node::UIImagePtr obj = utils::make<node::UIImage>();
        for (const auto&[key, value] : attribs)
        {
            if (key == SCALE)
            {
                obj->getBaseLayoutData().setScale(parseScale(value));
            }
            else if (key == SRC) { obj->setImage(value); }
        }
        return obj;
    });

    setContructRule("Button", [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        node::UIButtonPtr obj = utils::make<node::UIButton>();
        for (const auto&[key, value] : attribs)
        {
            if (key == SCALE)
            {
                obj->getBaseLayoutData().setScale(parseScale(value));
            }
            else if (key == TEXT) { obj->setText(value); }
        }
        return obj;
    });

    setContructRule("Label", [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        node::UILabelPtr obj = utils::make<node::UILabel>();
        for (const auto&[key, value] : attribs)
        {
            if (key == SCALE)
            {
                obj->getBaseLayoutData().setScale(parseScale(value));
            }
            else if (key == TEXT) { obj->setText(value); }
        }
        return obj;
    });

    setContructRule("Slider", [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        node::UISliderPtr obj = utils::make<node::UISlider>();
        for (const auto&[key, value] : attribs)
        {
            if (key == SCALE) { obj->getBaseLayoutData().setScale(parseScale(value)); }
            else if (key == ORIENTATION_SHORT) { obj->getBaseLayoutData().setType(parseOrientation(value)); }
            else if (key == ORIENTATION) { obj->getBaseLayoutData().setType(parseOrientation(value)); }
            else if (key == SLIDER_DEFAULT) { obj->setScrollValue(parseNumber(value)); }
            else if (key == SLIDER_FROM) { obj->setScrollFrom(parseNumber(value)); }
            else if (key == SLIDER_TO) { obj->setScrollTo(parseNumber(value)); }
        }
        return obj;
    });

    setContructRule("Pane", [this](const hk::XMLDecoder::AttrPairVec& attribs) -> node::UIBasePtr
    {
        node::UIPanePtr obj = utils::make<node::UIPane>();
        for (const auto&[key, value] : attribs)
        {
            if (key == SCALE) { obj->getBaseLayoutData().setScale(parseScale(value)); }
            else if (key == ORIENTATION_SHORT) { obj->getBaseLayoutData().setType(parseOrientation(value)); }
            else if (key == ORIENTATION) { obj->getBaseLayoutData().setType(parseOrientation(value)); }
        }
        return obj;
    });
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

auto LavParser::parseScale(const std::string& value) const -> LayoutBase::ScaleXY
{
    static uint32_t EXPECTED_ARG_COUNT{2};
    static std::regex del{","};
    static std::sregex_token_iterator end;

    std::sregex_token_iterator regIt(value.begin(), value.end(), del, -1);

    uint32_t currentArgIdx{0};
    LayoutBase::ScaleXY returnScale{0, 0};
    while (regIt != end)
    {
        std::string stripped{*regIt};
        std::erase_if(stripped, ::isspace);
        auto recast = reinterpret_cast<LayoutBase::Scale*>(&returnScale);
        if (stripped == "Fill")
        {
            recast[currentArgIdx] = LayoutBase::Scale{1, LayoutBase::ScaleType::FILL};
        }
        else if (stripped == "Fit")
        {
            recast[currentArgIdx] = LayoutBase::Scale{1, LayoutBase::ScaleType::FIT};
        }
        else if (auto pxIt = stripped.find("px"); pxIt != std::string::npos)
        {
            float val = std::stoi(stripped.substr(0, pxIt));
            recast[currentArgIdx] = LayoutBase::Scale{val, LayoutBase::ScaleType::PX};
        }
        else if (auto relIt = stripped.find("%"); relIt != std::string::npos)
        {
            float val = std::stof(stripped.substr(0, relIt)) / 100.0f;
            recast[currentArgIdx] = LayoutBase::Scale{val, LayoutBase::ScaleType::REL};
        }
        else
        {
            log_.error("Invalid token: '{}'", stripped);
        }
        ++regIt;
        ++currentArgIdx;
    }

    if (currentArgIdx != EXPECTED_ARG_COUNT)
    {
        log_.error("Not enough args for '{}' . Expected '{}' Got '{}'", __func__, EXPECTED_ARG_COUNT, currentArgIdx);
    }
    return returnScale;
}

auto LavParser::parseNumber(const std::string& value) const -> float
{
    // TODO: Check for parse errors ofc
    return std::stof(value);
}

auto LavParser::parseVec2D(const std::string& value) const -> glm::ivec2
{
    static uint32_t EXPECTED_ARG_COUNT{2};
    static std::regex del{","};
    static std::sregex_token_iterator end;

    std::sregex_token_iterator it(value.begin(), value.end(), del, -1);

    uint32_t currentArgIdx{0};
    glm::ivec2 returnVec{0, 0};
    while (it != end)
    {
        returnVec[currentArgIdx] = std::stoi(*it);
        ++it;
        ++currentArgIdx;
    }

    if (currentArgIdx != EXPECTED_ARG_COUNT)
    {
        log_.error("Not enough args for '{}' . Expected '{}' Got '{}'", __func__, EXPECTED_ARG_COUNT, currentArgIdx);
    }

    return returnVec;
}

auto LavParser::parseOrientation(const std::string& value) const -> LayoutBase::Type
{
    if (value == "Horizontal") { return LayoutBase::Type::HORIZONTAL; }
    if (value == "Vertical") { return LayoutBase::Type::VERTICAL; }
    if (value == "Grid") { return LayoutBase::Type::GRID; }

    log_.error("Unknown layout type fed in: '{}'", value);
    return LayoutBase::Type::HORIZONTAL;
}

auto LavParser::setContructRule(const std::string& tag, const RuleSignature& rule) -> void
{
    constructRuleMap_[tag] = rule;
}

} // namespace lav::core