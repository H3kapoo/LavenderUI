#pragma once

#include <memory>

#include "vendor/glm/glm.hpp"

namespace src::framestate
{
static constexpr uint32_t NOTHING = 0;

struct FrameState
{
    uint32_t clickedId{NOTHING};
    uint32_t selectedId{NOTHING};
    uint32_t hoveredId{NOTHING};
    uint32_t prevHoveredId{NOTHING};
    uint32_t hoveredZIndex{NOTHING};
    uint8_t mouseAction{NOTHING};
    uint8_t mouseButton{NOTHING};
    glm::ivec2 mousePos{0, 0};
    glm::ivec2 scrollOffset{0, 0};
    bool isDragging{false};
    uint32_t currentEventId{0};
};
using FrameStatePtr = std::shared_ptr<FrameState>;
} // namespace src::framestate
