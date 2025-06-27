#pragma once

#include "src/UIElements/UIBase.hpp"
#include "src/WindowManagement/NativeWindow.hpp"

namespace src::uielements
{
class UIFrame : public UIBaseCPRT<UIFrame>
{
public:
    UIFrame(const std::string& title, const glm::ivec2& size);
    UIFrame(const UIFrame&) = delete;
    UIFrame(UIFrame&&) = delete;
    UIFrame& operator=(const UIFrame&) = delete;
    UIFrame& operator=(UIFrame&&) = delete;

    auto run() -> bool;

    auto getWindow() -> windowmanagement::NativeWindow&;
    auto isMainFrame() -> bool;

private:
    auto render(const glm::mat4& projection) -> void override;
    auto layout() -> void override;
    auto event(framestate::FrameStatePtr& state, const elementevents::IEvent& e) -> void override;

    auto updateProjection() -> void;

private:
    windowmanagement::NativeWindow window_;
    framestate::FrameStatePtr frameState_;
    bool forcedQuit_;
    bool isMainFrame_;
    glm::mat4 projection_;

private:
    static bool isFirstFrame_;
};
using UIFramePtr = std::shared_ptr<UIFrame>;
using UIFrameWPtr = std::weak_ptr<UIFrame>;
} // namespace src::uielements