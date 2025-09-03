#pragma once

#include <memory>

#include "src/WindowManagement/Input.hpp"
#include "vendor/glm/glm.hpp"

namespace src::state
{
using namespace windowmanagement;

static constexpr uint32_t NOTHING = 0;

/**
    @brief Structure defining each needed state per window.
    
    @note Can be used to exchange data between UI elements if need be.
    @note It's best to keep this class as opaque as possible with as little deps as possible at
        least for now.
*/
struct UIWindowState
{
    uint32_t clickedId{NOTHING};
    uint32_t selectedId{NOTHING};
    uint32_t hoveredId{NOTHING};
    uint32_t prevHoveredId{NOTHING};
    uint32_t hoveredZIndex{NOTHING};
    uint32_t closestScroll{NOTHING};
    Input::Action mouseAction{NOTHING};
    Input::Mouse mouseButton{NOTHING};
    glm::ivec2 mousePos{0, 0};
    glm::ivec2 mouseDiff{0, 0};
    glm::ivec2 scrollOffset{0, 0};
    glm::ivec2 windowSize{0, 0};
    glm::ivec2 windowSizeDelta{0, 0};
    bool isDragging{false};
    std::optional<Input::Cursor> currentCursorType{Input::Cursor::ARROW};
    std::optional<Input::Cursor> wantedCursorType{std::nullopt};
    uint32_t currentEventId{0};
};
using UIWindowStatePtr = std::shared_ptr<UIWindowState>;
} // namespace src::state
