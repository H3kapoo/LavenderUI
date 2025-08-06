#pragma once

#include <sstream>

#include "vendor/glm/glm.hpp"

namespace src::elementcomposable
{
/**
    @brief Base class for any generic layout related options/information.
*/
class LayoutBase
{
public:
    struct BoundingBox
    {
        glm::vec2 start;
        glm::vec2 end;
    };

    /** @brief Represents the display type of the element. */
    enum class Type : uint8_t
    {
        HORIZONTAL, VERTICAL, GRID
    };

    /** @brief Defines how elements will be aligned inside the parent. */
    enum Align : uint8_t
    {
        // TOP, LEFT, BOTTOM, RIGHT,
        TOP_LEFT,
        TOP_CENTER,
        TOP_RIGHT,
        CENTER_LEFT,
        CENTER,
        CENTER_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT
    };

    /** @brief Defines how elements will be spaced from each other inside the parent. */
    enum Spacing : uint8_t { TIGHT, EVEN_NO_GAP, EVEN_GAP };

    /** @brief Represents the scale type applied to the element. */
    enum class ScaleType : uint8_t
    {
        PX, REL, FIT, FILL
    };

    /** @brief Represents the scale type and value of the element. */
    struct Scale
    {
        Scale(int32_t v) : val{(float)v} {}
        Scale(float v) : val{v} {}
        Scale(float v, ScaleType t) : val{v}, type{t} {}

        float val{100.0f};
        ScaleType type{ScaleType::PX};
    };

    /** @brief Represents the Scale on each axis. */
    struct ScaleXY
    {
        Scale x;
        Scale y;
    };

    /** @brief Represents the position type applied to the element. */
    enum class PositionType : uint8_t
    {
        COMPUTED, ABS
    };

    /** @brief Represents the position type and value of the element. */
    struct Position
    {
        float val{0.0f};
        PositionType type{PositionType::COMPUTED};
    };

    /** @brief Represents the Position on each axis. */
    struct PositionXY
    {
        Position x;
        Position y;
    };

    /** @brief Represents a generic structure containing values for the 4 general directions. */
    struct TBLR
    {
        TBLR(int32_t vt, int32_t vb, int32_t vl, int32_t vr) : top{vt}, bot{vb}, left{vl}, right{vr} {}
        TBLR(int32_t vtb, int32_t vlr) : top{vtb}, bot{vtb}, left{vlr}, right{vlr} {}
        TBLR(int32_t v) : top{v}, bot{v}, left{v}, right{v} {}
        operator glm::vec4() const { return glm::vec4{top, bot, left, right}; }

        int32_t top{0};
        int32_t bot{};
        int32_t left{0};
        int32_t right{0};
    };

public:
    LayoutBase() = default;
    virtual ~LayoutBase() = default;

    auto isPointInside(const glm::ivec2& p) const -> bool;
    auto isPointInsideView(const glm::ivec2& p) const -> bool;
    auto computeViewBox(const LayoutBase& parentAttribs) -> void;
    auto getLRMargin() const -> int32_t;
    auto getTBMargin() const -> int32_t;
    auto getFullBoxPos() const -> glm::vec2;
    auto getFullBoxScale() const -> glm::vec2;
    auto getContentBoxPos() const -> glm::vec2;
    auto getContentBoxScale() const -> glm::vec2;
    auto getTransform() -> const glm::mat4&;
    auto getType() const -> Type;
    auto getMargin() const -> const TBLR&;
    auto getPadding() const -> const TBLR&;
    auto getBorder() const -> const TBLR&;
    auto getBorderRadius() const -> const TBLR&;
    auto getShadow() const -> const TBLR&;
    auto getSelfAlign() const -> const Align&;
    auto getAlign() const -> const Align&;
    auto getSpacing() const -> const Spacing&;
    auto getMinScale() const -> const glm::ivec2&;
    auto getMaxScale() const -> const glm::ivec2&;
    auto getWrap() const -> bool;
    auto getPos() const -> const PositionXY&;
    auto getScale() const -> const ScaleXY&;
    auto getComputedPos() const -> const glm::vec2&;
    auto getComputedScale() const -> const glm::vec2&;
    auto getViewPos() const -> const glm::ivec2&;
    auto getViewScale() const -> const glm::ivec2&;
    auto getIndex() const -> uint32_t;
    auto getAngle() const -> float;
    auto isCustomIndex() const -> bool;

    auto setType(Type value) -> LayoutBase&;
    auto setMargin(const TBLR& value) -> LayoutBase&;
    auto setPadding(const TBLR& value) -> LayoutBase&;
    auto setBorder(const TBLR& value) -> LayoutBase&;
    auto setBorderRadius(const TBLR& value) -> LayoutBase&;
    auto setShadow(const TBLR& value) -> LayoutBase&;
    auto setSelfAlign(const Align value) -> LayoutBase&;
    auto setAlign(const Align value) -> LayoutBase&;
    auto setSpacing(const Spacing value) -> LayoutBase&;
    auto setMinScale(const glm::ivec2 value) -> LayoutBase&;
    auto setMaxScale(const glm::ivec2 value) -> LayoutBase&;
    auto setWrap(const bool value) -> LayoutBase&;
    auto setPos(const PositionXY& value) -> LayoutBase&;
    auto setScale(const ScaleXY& value) -> LayoutBase&;
    auto setComputedPos(const glm::vec2& value) -> LayoutBase&;
    auto setComputedScale(const glm::vec2& value) -> LayoutBase&;
    auto setViewPos(const glm::vec2& value) -> LayoutBase&;
    auto setViewScale(const glm::vec2& value) -> LayoutBase&;
    auto setIndex(uint32_t value) -> LayoutBase&;
    auto setEnableCustomIndex(const bool val) -> LayoutBase&;
    auto setAngle(float value) -> LayoutBase&;

    friend auto operator-(const glm::vec2 lhs, const TBLR rhs) -> glm::vec2;

// for now
public:
    glm::vec2 tempPosOffset{0, 0};

protected:
    Type layoutType_{Type::HORIZONTAL};
    TBLR margin{0};
    TBLR padding{0};
    TBLR border{0};
    TBLR borderRadius{0};
    TBLR shadow{0};
    Align selfAlign_{Align::TOP_LEFT};
    Align align_{Align::TOP_LEFT};
    Spacing spacing_{Spacing::TIGHT};
    glm::ivec2 minScale{10, 10};
    glm::ivec2 maxScale{10'000, 10'000};
    bool wrap{false};

    /** @brief User supplied position details. This is NOT the actual render start position since it
        includes margins as well. This is the start position of the whole object. */
    PositionXY userPos_;

    /** @brief User supplied scale details. This is NOT the actual rendered scale but the size of
        the element's content area + padding + border + margins.
    */
    ScaleXY userScale_{{100.0f}, {100.0f}};

    /** @brief The computed position where the mesh rendering starts. This is NOT the actual position of
        the element as it doesn't include margins.
    */
    glm::vec2 computedPos_{0.0f, 0.0f};

    /** @brief The computed scale that dictates the scale of the rendered area. This is NOT the actual
        scale of the element as it doesn't include margins.
    */
    glm::vec2 computedScale_{100.0f, 100.0f};

    /** @brief Viewable pos and scale used to determine how much of this element is visible from the parent's
        perspective. Basically the parent-child intersection data. It doesn't include margins.
    */
    glm::ivec2 viewPos_{0}, viewScale_{0};
    uint32_t index_{1};
    float angle_{30.0f};
    bool isCustomIndex_{false};

private:
    glm::mat4 transform_{glm::mat4{1}};
};

LayoutBase::Scale operator"" _fill(unsigned long long);
LayoutBase::Scale operator"" _fit(unsigned long long);
LayoutBase::Scale operator"" _rel(long double value);
LayoutBase::Scale operator"" _px(unsigned long long value);
// Layout::GridDistrib operator"" _gpx(unsigned long long value);
// Layout::GridDistrib operator"" _fr(unsigned long long value);

auto operator-(const glm::vec2 lhs, const glm::ivec2 rhs) -> glm::vec2;
auto operator/(const glm::vec2 lhs, const int32_t rhs) -> glm::vec2;

} // namespace src::elementcomposable

/* Custom formatters */
template<>
struct std::formatter<glm::vec2> : std::formatter<std::string_view>
{
    auto format(const glm::vec2& obj, format_context& ctx) const
    {
        std::ostringstream os;
        os << std::format("({:.03f}, {:.03f})", obj.x, obj.y);
        return std::formatter<std::string_view>::format(std::move(os).str(), ctx);
    }
};

template<>
struct std::formatter<glm::vec3> : std::formatter<std::string_view>
{
    auto format(const glm::vec3& obj, format_context& ctx) const
    {
        std::ostringstream os;
        os << std::format("({:.03f}, {:.03f}, {:.03f})", obj.x, obj.y, obj.z);
        return std::formatter<std::string_view>::format(std::move(os).str(), ctx);
    }
};

template<>
struct std::formatter<glm::vec4> : std::formatter<std::string_view>
{
    auto format(const glm::vec4& obj, format_context& ctx) const
    {
        std::ostringstream os;
        os << std::format("({:.03f}, {:.03f}, {:.03f}, {:.03f})", obj.x, obj.y, obj.z, obj.w);
        return std::formatter<std::string_view>::format(std::move(os).str(), ctx);
    }
};

template<>
struct std::formatter<src::elementcomposable::LayoutBase::TBLR> : std::formatter<std::string_view>
{
    auto format(const src::elementcomposable::LayoutBase::TBLR& obj, format_context& ctx) const
    {
        std::ostringstream os;
        os << std::format("(T:{}, B:{}, L:{}, R:{})", obj.top, obj.bot, obj.left, obj.right);
        return std::formatter<std::string_view>::format(std::move(os).str(), ctx);
    }
};

template<>
struct std::formatter<src::elementcomposable::LayoutBase::PositionType> : std::formatter<std::string_view>
{
    auto format(const src::elementcomposable::LayoutBase::PositionType& obj, format_context& ctx) const
    {
        std::ostringstream os;
        switch(obj)
        {
            case src::elementcomposable::LayoutBase::PositionType::ABS:
                os << "ABS";
                break;
            case src::elementcomposable::LayoutBase::PositionType::COMPUTED:
                os << "COMPUTED";
                break;
            default:
                os << "UNKNOWN";
        }
        return std::formatter<std::string_view>::format(std::move(os).str(), ctx);
    }
};

template<>
struct std::formatter<src::elementcomposable::LayoutBase::PositionXY> : std::formatter<std::string_view>
{
    auto format(const src::elementcomposable::LayoutBase::PositionXY& obj, format_context& ctx) const
    {
        std::ostringstream os;
        os << std::format("(({}, {}), ({}, {}))", obj.x.val, obj.x.type, obj.y.val, obj.y.type);
        return std::formatter<std::string_view>::format(std::move(os).str(), ctx);
    }
};

template<>
struct std::formatter<src::elementcomposable::LayoutBase::ScaleType> : std::formatter<std::string_view>
{
    auto format(const src::elementcomposable::LayoutBase::ScaleType& obj, format_context& ctx) const
    {
        std::ostringstream os;
        switch(obj)
        {
            case src::elementcomposable::LayoutBase::ScaleType::PX:
                os << "PX";
                break;
            case src::elementcomposable::LayoutBase::ScaleType::REL:
                os << "REL";
                break;
            case src::elementcomposable::LayoutBase::ScaleType::FIT:
                os << "FIT";
                break;
            case src::elementcomposable::LayoutBase::ScaleType::FILL:
                os << "FILL";
                break;
            default:
                os << "UNKNOWN";
        }
        return std::formatter<std::string_view>::format(std::move(os).str(), ctx);
    }
};

template<>
struct std::formatter<src::elementcomposable::LayoutBase::ScaleXY> : std::formatter<std::string_view>
{
    auto format(const src::elementcomposable::LayoutBase::ScaleXY& obj, format_context& ctx) const
    {
        std::ostringstream os;
        os << std::format("(({}, {}), ({}, {}))", obj.x.val, obj.x.type, obj.y.val, obj.y.type);
        return std::formatter<std::string_view>::format(std::move(os).str(), ctx);
    }
};

