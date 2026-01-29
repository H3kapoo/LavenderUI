#include <LavenderUI/Core/TextHandler/CaretHelper.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <LavenderUI/Core/Binders/WindowBinder.hpp>
#include <LavenderUI/Core/ResourceHandler/ShaderLoader.hpp>
#include <LavenderUI/Utils/Misc.hpp>

namespace lav::core
{
auto CaretHelper::get() -> CaretHelper&
{
    static CaretHelper instance;
    return instance;
}

CaretHelper::CaretHelper()
    : caretShader_(ShaderLoader::get().load("assets/shaders/elemVert.glsl", "assets/shaders/elemFrag.glsl" ))
    , caretColor_(utils::hexToVec4("#dd4b4bff"))
    , caretScale_(1, 20)
    , caretPos_(0, 0)
    , lastBlinkTime_(0)
    , lastKeepAliveTimeSec_(0)
    , blinkIntervalMs_(500)
    , renderCaret_(false)
    , started_(false)
    , blinkThread_()
{}

CaretHelper::~CaretHelper()
{
    started_ = false;
    cv_.notify_one();
    blinkThread_.reset();
}

auto CaretHelper::render(const glm::mat4& projection) -> void
{
    if (!started_ || !renderCaret_) { return; }

    caretShader_.bind();
    caretShader_.uploadMat4("uMatrixProjection", projection);
    caretShader_.uploadMat4("uMatrixTransform", getCaretTransform());
    caretShader_.uploadVec4f("uColor", caretColor_);
    caretShader_.uploadVec2f("uResolution", caretScale_);
    // TODO: shader shall be created just for caret as to not use a lot of features
    // dedicated to a normal UI element, like bellow.
    caretShader_.uploadVec4f("uBorderSize", glm::ivec4{0, 0, 0, 0});
    caretShader_.uploadVec4f("uBorderRadii", glm::ivec4{0, 0, 0, 0});
    caretShader_.uploadVec4f("uBorderColor", glm::ivec4{0, 0, 0, 0});
    core::GPUBinder::get().enable(core::GPUBinder::Function::DEPTH, false);
    core::GPUBinder::get().renderBoundQuad();
    core::GPUBinder::get().enable(core::GPUBinder::Function::DEPTH, true);
}

auto CaretHelper::start() -> void
{
    if (started_) { return; }

    started_ = true;

    blinkThread_ = std::make_unique<std::jthread>([this](){ startBlinkLogic(); });
}

auto CaretHelper::stop() -> void
{
    started_ = false;
    renderCaret_ = false;
    lastBlinkTime_ = 0;
    lastKeepAliveTimeSec_ = 0;
    cv_.notify_one();
    blinkThread_.reset();
}

auto CaretHelper::requestKeepAlive() -> void
{
    lastKeepAliveTimeSec_ = core::WindowBinder::get().getTime();
}

auto CaretHelper::startBlinkLogic() -> void
{
    // TODO: Ideally this shall be a job in a threadpool queue, but it's fine for now
    // TODO: Since we use jthread, use stop_token for cooperative termination
    using namespace std::literals::chrono_literals;
    while (started_)
    {
        /*
            Blink is served as MS but getTime() returns time in seconds since app start.
            We need to convert the MS in S to do the comparisons.
        */
        const double seconds = blinkIntervalMs_.count() / 1000.0f;

        renderCaret_ = !renderCaret_;

        const double now = core::WindowBinder::get().getTime();

        /* Always keep caret active if the user recently typed. */
        if (now - lastKeepAliveTimeSec_ < seconds) { renderCaret_ = true; }

        core::WindowBinder::get().requestEmptyEvent();

        /* Sleep for intervalMs OR until we are notified by stop() to pack and fuck off. */
        // TODO: Dont create a lock each while loop
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait_for(lock, blinkIntervalMs_,
            [this]() -> bool { return !started_; });
    }
}

auto CaretHelper::getCaretTransform() -> const glm::mat4
{
    glm::mat4 model{glm::mat4(1.0f)};
    model = glm::translate(model, glm::vec3(caretPos_.x, caretPos_.y, 1));
    model = glm::scale(model, glm::vec3(caretScale_, 1));
    return model;
}

auto CaretHelper::setCaretColor(const glm::vec4 color) -> void
{
    caretColor_ = color;
}

auto CaretHelper::setCaretScale(const glm::ivec2 scale) -> void
{
    caretScale_ = scale;
}

auto CaretHelper::setCaretPos(const glm::ivec2 pos) -> void
{
    caretPos_ = pos;
}

auto CaretHelper::setBlinkTime(const std::chrono::milliseconds& ms) -> void
{
    blinkIntervalMs_ = ms;
}
} // namespace lav::core
