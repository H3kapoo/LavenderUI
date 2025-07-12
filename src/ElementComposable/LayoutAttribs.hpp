#pragma once

#include "vendor/glm/glm.hpp"

namespace src::elementcomposable
{
class LayoutAttribs
{
public:
    struct BoundingBox
    {
        glm::vec2 start;
        glm::vec2 end;
    };

    enum class Type : uint8_t
    {
        HORIZONTAL, VERTICAL, GRID
    };

    enum class ScaleType : uint8_t
    {
        PX, REL, FIT, FILL
    };

    struct Scale
    {
        float val{100.0f};
        ScaleType type{ScaleType::PX};
    };

    struct ScaleXY
    {
        Scale x;
        Scale y;
    };

    enum class PositionType
    {
        COMPUTED, ABS
    };

    struct Position
    {
        float val{0.0f};
        PositionType type{PositionType::COMPUTED};
    };

    struct PositionXY
    {
        Position x;
        Position y;
    };

    struct TBLR
    {
        int32_t top{0};
        int32_t bot{};
        int32_t left{0};
        int32_t right{0};
    };

public:
    LayoutAttribs() = default;
    virtual ~LayoutAttribs() = default;

    auto isPointInside(const glm::ivec2& p) const -> bool;
    auto isPointInsideView(const glm::ivec2& p) const -> bool;
    auto computeViewBox(const LayoutAttribs& parentAttribs) -> void;
    auto getLRMargin() const -> int32_t;
    auto getTBMargin() const -> int32_t;
    auto getFullBoxPos() const -> glm::vec2;
    auto getFullBoxScale() const -> glm::vec2;
    auto getContentBoxPos() const -> glm::vec2;
    auto getContentBoxScale() const -> glm::vec2;

    auto getLayoutTransform() -> const glm::mat4&;
    auto getLayoutType() const -> Type;
    auto getLayoutMargin() const -> const TBLR&;
    auto getLayoutPadding() const -> const TBLR&;
    auto getLayoutBorder() const -> const TBLR&;
    auto getLayoutBorderRadius() const -> const TBLR&;
    auto getLayoutShadow() const -> const TBLR&;
    auto getLayoutWrap() const -> bool;
    auto getLayoutPos() const -> const PositionXY&;
    auto getLayoutScale() const -> const ScaleXY&;
    auto getLayoutComputedPos() const -> const glm::vec2&;
    auto getLayoutComputedScale() const -> const glm::vec2&;
    auto getLayoutViewPos() const -> const glm::vec2&;
    auto getLayoutViewScale() const -> const glm::vec2&;
    auto getLayoutIndex() const -> uint32_t;
    auto getLayoutAngle() const -> float;

    auto setLayoutType(Type value) -> LayoutAttribs&;
    auto setLayoutMargin(const TBLR& value) -> LayoutAttribs&;
    auto setLayoutPadding(const TBLR& value) -> LayoutAttribs&;
    auto setLayoutBorder(const TBLR& value) -> LayoutAttribs&;
    auto setLayoutBorderRadius(const TBLR& value) -> LayoutAttribs&;
    auto setLayoutShadow(const TBLR& value) -> LayoutAttribs&;
    auto setLayoutWrap(const bool value) -> LayoutAttribs&;
    auto setLayoutPos(const PositionXY& value) -> LayoutAttribs&;
    auto setLayoutScale(const ScaleXY& value) -> LayoutAttribs&;
    auto setLayoutComputedPos(const glm::vec2& value) -> LayoutAttribs&;
    auto setLayoutComputedScale(const glm::vec2& value) -> LayoutAttribs&;
    auto setLayoutViewPos(const glm::vec2& value) -> LayoutAttribs&;
    auto setLayoutViewScale(const glm::vec2& value) -> LayoutAttribs&;
    auto setLayoutIndex(uint32_t value) -> LayoutAttribs&;
    auto setLayoutEnableCustomIndex(const bool val) -> LayoutAttribs&;
    auto setLayoutAngle(float value) -> LayoutAttribs&;

// for now
public:
    glm::vec2 tempPosOffset{0, 0};

protected:
    Type layoutType_{Type::HORIZONTAL};
    TBLR margin{0}; // top, bottom, left, right
    TBLR padding{0};
    TBLR border{0};
    TBLR borderRadius{0}; // top-left, top-right, bottom-right, bottom-left
    TBLR shadow{0}; // no ideea of it's use so far
    bool wrap{false};

    /* User supplied position details. This is NOT the actual render start position since it includes margins
        as well. This is the start position of the whole object. */
    PositionXY userPos_;

    /* User supplied scale details. This is NOT the actual renderable scale, it's the size of the element's
        content area + padding + border + margins.
    */
    ScaleXY userScale_{{100.0f}, {100.0f}};

    /* The computed position where the mesh rendering starts. This is NOT the actual position of the element
        as it doesn't include margins.
    */
    glm::vec2 computedPos_{0.0f, 0.0f};

    /* The computed scale that dictates the scale of the rendered area. This is NOT the actual scale of the
        element as it doesn't include margins.
    */
    glm::vec2 computedScale_{100.0f, 100.0f};

    /* Viewable pos and scale used to determine how much of this element is visible from the parent's
        perspective. Basically the parent-child intersection data. It doesn't include margins.
    */
    glm::vec2 viewPos_{0}, viewScale_{0};
    uint32_t index_{1};
    float angle_{30.0f};
    bool isCustomIndex_{false};

private:
    glm::mat4 transform_{glm::mat4{1}};
};

// Layout::Scale operator"" _fill(unsigned long long val);
// Layout::Scale operator"" _fit(unsigned long long);
LayoutAttribs::Scale operator"" _rel(long double value);
LayoutAttribs::Scale operator"" _px(unsigned long long value);
// Layout::GridDistrib operator"" _gpx(unsigned long long value);
// Layout::GridDistrib operator"" _fr(unsigned long long value);

// inline Layout::Scale operator*(Layout::Scale lhs, float rhs)
// {
//     lhs.value *= rhs;
//     lhs.value = lhs.type == Layout::ScaleType::PX ? (int32_t)lhs.value : lhs.value;
//     return lhs;
// }
} // namespace src::elementcomposable
