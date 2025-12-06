#pragma once

#include <string>

#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Utils/Logger.hpp"
#include "vendor/glm/glm.hpp"

namespace lav::core
{
class ParseHelper
{
public:
    static auto get() -> ParseHelper&;

    auto toScale(const std::string& value) const -> LayoutBase::ScaleXY;
    auto toNumber(const std::string& value) const -> float;
    auto toVec2D(const std::string& value) const -> glm::ivec2;
    auto toOrientation(const std::string& value) const -> LayoutBase::Type;
    auto toAlign(const std::string& value) const -> LayoutBase::Align;
    auto toBorder(const std::string& value) const -> LayoutBase::TBLR;
    auto toColor(const std::string& value) const -> glm::vec4;

private:
    ParseHelper() = default;
    ~ParseHelper() = default;
    ParseHelper(const ParseHelper&) = delete;
    ParseHelper& operator=(const ParseHelper&) = delete;
    ParseHelper(ParseHelper&&) noexcept = delete;
    ParseHelper& operator=(ParseHelper&&) noexcept = delete;

private:
    utils::Logger log_{"ParseHelper"};
};
} // namespace lav::core