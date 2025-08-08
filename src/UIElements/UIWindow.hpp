#pragma once

#include "src/ElementComposable/IEvent.hpp"
#include "src/State/UIWindowState.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/WindowManagement/NativeWindow.hpp"

namespace src::uielements
{
/**
    @brief Top level GUI container wrapping an OS window. Manages dispatch of layout calculations,
        rendering, layout and events handling for all children GUI elements.

    @note Each UIWindow has it's own global UIWindowState handle.
*/
class UIWindow : public UIBase
{
public:
    UIWindow(const std::string& title, const glm::ivec2& size);
    virtual ~UIWindow() = default;
    UIWindow(const UIWindow&) = delete;
    UIWindow(UIWindow&&) = delete;
    auto operator=(UIWindow&&) -> UIWindow& = delete;
    auto operator=(const UIWindow&) -> UIWindow& = delete;

    /**
        @brief Run the window's logic one time.

        @note This is not the same as the App::run() method as UIWindows do not have their
            standalone game loop.
        @note This is public in order to give the user the freedom to make his own loop.

        @return True if window shall close. False otherwise.
    */
    auto run() -> bool;

    auto setTitle(std::string title, const bool updateInteralText = true) -> void;
    auto getDeltaTime() -> double;
    auto getTitle() -> std::string;
    auto getWindow() -> windowmanagement::NativeWindow&;
    auto isMainWindow() -> bool;

    /* Mandatory typeinfo */
    INSERT_TYPEINFO(UIWindow);
    BASE_ALLOW_ADD_REMOVE;

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(state::UIWindowStatePtr& state) -> void override;
    auto spawnEvent(const elementcomposable::IEvent& event) -> void;
    auto updateProjection() -> void;
    auto windowResizeHook(const uint32_t newX, const uint32_t newY) -> void;
    auto windowMouseEnterHook(const bool entered) -> void;
    auto keyHook(const uint32_t key, const uint32_t scancode, const uint32_t action,
        const uint32_t mods) -> void;
    auto mouseMoveHook(const int32_t newX, const int32_t newY) -> void;
    auto mouseButtonHook(const uint32_t btn, const uint32_t action) -> void;
    auto mouseScrollHook(const uint32_t xOffset, const uint32_t yOffset) -> void;

private:
    windowmanagement::NativeWindow window_;
    state::UIWindowStatePtr windowState_{utils::make<state::UIWindowState>()};
    bool forcedQuit_{false};
    bool isMainWindow_{false};
    glm::mat4 projection_;

    static bool isFirstWindow_;
    static int32_t MAX_LAYERS;
};
using UIWindowPtr = std::shared_ptr<UIWindow>;
using UIWindowWPtr = std::weak_ptr<UIWindow>;
} // namespace src::uielements