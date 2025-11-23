#include "ParseHelpers.hpp"

namespace lav::core
{
auto ParseHelper::toScale(const std::string& value) const -> LayoutBase::ScaleXY
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
            utils::Logger log(__func__);
            log.error("Invalid token: '{}'", stripped);
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
        utils::Logger log(__func__);
        log.error("Not enough args for '{}' . Expected '{}' Got '{}'", __func__, EXPECTED_ARG_COUNT, currentArgIdx);
    }

    return returnVec;
}

auto ParseHelper::toOrientation(const std::string& value) const -> LayoutBase::Type
{
    if (value == "Horizontal") { return LayoutBase::Type::HORIZONTAL; }
    if (value == "Vertical") { return LayoutBase::Type::VERTICAL; }
    if (value == "Grid") { return LayoutBase::Type::GRID; }

    utils::Logger log(__func__);
    log.error("Unknown layout type fed in: '{}'", value);
    return LayoutBase::Type::HORIZONTAL;
}
} // namespace lav::core