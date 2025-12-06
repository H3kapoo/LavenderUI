#include "ParseHelpers.hpp"

#include <regex>

#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Utils/Misc.hpp"

namespace lav::core
{
auto ParseHelper::get() -> ParseHelper&
{
    static ParseHelper instance;
    return instance;
}

auto ParseHelper::toScale(const std::string& value) const -> LayoutBase::ScaleXY
{
    static uint32_t EXPECTED_ARG_COUNT{2};
    static std::regex del{","};
    std::sregex_token_iterator end;

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
        utils::Logger log(__func__);
        log.error("Not enough args for '{}' . Expected '{}' Got '{}'", __func__, EXPECTED_ARG_COUNT, currentArgIdx);
    }
    return returnScale;
}

auto ParseHelper::toNumber(const std::string& value) const -> float
{
    // TODO: Check for parse errors ofc
    return std::stof(value);
}

auto ParseHelper::toVec2D(const std::string& value) const -> glm::ivec2
{
    static uint32_t EXPECTED_ARG_COUNT{2};
    static std::regex del{","};
    std::sregex_token_iterator end;

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

auto ParseHelper::toOrientation(const std::string& value) const -> LayoutBase::Type
{
    if (value == "Horizontal") { return LayoutBase::Type::HORIZONTAL; }
    if (value == "Vertical") { return LayoutBase::Type::VERTICAL; }
    if (value == "Grid") { return LayoutBase::Type::GRID; }

    log_.error("Unknown layout type fed in: '{}'", value);
    return LayoutBase::Type::HORIZONTAL;
}

auto ParseHelper::toAlign(const std::string& value) const -> LayoutBase::Align
{
    if (value == "Center") { return LayoutBase::Align::CENTER; }

    log_.error("Unkown align: '{}'", value);
    return LayoutBase::Align::TOP_LEFT; 
}

auto ParseHelper::toBorder(const std::string& value) const -> LayoutBase::TBLR
{
    static uint32_t EXPECTED_ARG_COUNT{4};
    static std::regex del{","};
    std::sregex_token_iterator end;

    std::sregex_token_iterator it(value.begin(), value.end(), del, -1);

    uint32_t currentArgIdx{0};
    LayoutBase::TBLR returnVal{0, 0, 0, 0};
    auto recast = reinterpret_cast<int32_t*>(&returnVal);
    while (it != end)
    {
        recast[currentArgIdx] = std::stoi(*it);
        ++it;
        ++currentArgIdx;
    }

    if (currentArgIdx != EXPECTED_ARG_COUNT)
    {
        log_.error("Not enough args for '{}' . Expected '{}' Got '{}'", __func__, EXPECTED_ARG_COUNT, currentArgIdx);
    }

    return returnVal;
}

auto ParseHelper::toColor(const std::string& value) const -> glm::vec4
{
    return utils::hexToVec4(value);
}

} // namespace lav::core