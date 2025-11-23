#pragma once

#include <string>
#include <regex>

#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Utils/Logger.hpp"
#include "vendor/glm/glm.hpp"

namespace lav::core
{
class ParseHelper
{
public:
    auto toScale(const std::string& value) const -> LayoutBase::ScaleXY;
    auto toNumber(const std::string& value) const -> float;
    auto toVec2D(const std::string& value) const -> glm::ivec2;
    auto toOrientation(const std::string& value) const -> LayoutBase::Type;
};
} // namespace lav::core