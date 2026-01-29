#pragma once

#include <string>

#include <glm/glm.hpp>
#include <LavenderUI/Core/LayoutHandler/LayoutBase.hpp>
#include <LavenderUI/Utils/Logger.hpp>

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
    auto toRelVector(const std::string& value) const -> std::vector<float>;

private:
    ParseHelper();
    ~ParseHelper() = default;
    ParseHelper(const ParseHelper&) = delete;
    ParseHelper& operator=(const ParseHelper&) = delete;
    ParseHelper(ParseHelper&&) noexcept = delete;
    ParseHelper& operator=(ParseHelper&&) noexcept = delete;

private:
    utils::Logger log_;
};
} // namespace lav::core