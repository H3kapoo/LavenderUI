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

public:
    LayoutAttribs() = default;
    ~LayoutAttribs() = default;
    // LayoutAttribs(const LayoutAttribs&) = delete;
    // LayoutAttribs(LayoutAttribs&&) = delete;
    // LayoutAttribs& operator=(const LayoutAttribs&) = delete;
    // LayoutAttribs& operator=(LayoutAttribs&&) = delete;

    auto isPointInside(const glm::ivec2& p) const -> bool;
    auto isPointInsideView(const glm::ivec2& p) const -> bool;
    auto computeViewBox(const LayoutAttribs& parentAttribs) -> void;
    // contains size of the drawable as sent to the shader: includes border, padding, content are
    auto getDrawableBox() -> BoundingBox;
    // auto getFullBox() -> BoundingBox;
    auto getTransform() -> const glm::mat4&;

public:
    Type type{Type::HORIZONTAL};
    glm::ivec4 margin{0};
    glm::ivec4 padding{0};
    glm::ivec4 border{0};
    glm::ivec4 borderRadius{0};
    glm::ivec4 shadow{0}; // no ideea of it's use so far
    glm::vec2 tempPosOffset{0, 0};

    /* User supplied position details. This is NOT the actual render start position since it includes margins
        as well. This is the start position of the whole object. */
    PositionXY pos;

    /* User supplied scale details. This is NOT the actual renderable scale, it's the size of the element's
        content area + padding + border + margins.
    */
    ScaleXY scale{{100.0f}, {100.0f}};

    /* The computed position where the mesh rendering starts. This is NOT the actual position of the element
        as it doesn't include margins.
    */
    glm::vec2 cPos{0};

    /* The computed scale that dictates the scale of the rendered area. This is NOT the actual scale of the
        element as it doesn't include margins.
    */
    glm::vec2 cScale{100.0f, 100.0f};

    /* Viewable pos and scale used to determine how much of this element is visible from the parent's
        perspective. Basically the parent-child intersection data. It doesn't include margins.
    */
    glm::vec2 viewPos{0}, viewScale{0};
    uint32_t index{1};
    float angle{30.0f};
    bool isCustomLevel{false};

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
