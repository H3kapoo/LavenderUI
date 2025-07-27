#pragma once

#include <memory>

#include "vendor/glm/glm.hpp"

namespace src::state
{
static constexpr uint32_t NOTHING = 0;

/**
    @brief:
        Structure defining each needed state per window.
    
    @notes:
    (1) Can be used to exchange data between UI elements if need be.
*/
struct UIWindowState
{
    uint32_t clickedId{NOTHING};
    uint32_t selectedId{NOTHING};
    uint32_t hoveredId{NOTHING};
    uint32_t prevHoveredId{NOTHING};
    uint32_t hoveredZIndex{NOTHING};
    uint32_t closestScroll{NOTHING};
    uint8_t mouseAction{NOTHING};
    uint8_t mouseButton{NOTHING};
    glm::ivec2 mousePos{0, 0};
    glm::ivec2 scrollOffset{0, 0};
    bool isDragging{false};
    uint32_t currentEventId{0};
};
using UIWindowStatePtr = std::shared_ptr<UIWindowState>;
} // namespace src::state
