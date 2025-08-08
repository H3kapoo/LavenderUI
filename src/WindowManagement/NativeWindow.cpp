#include "NativeWindow.hpp"

#include "src/Utils/Misc.hpp"
#include "vendor/glfw/include/GLFW/glfw3.h"

namespace src::windowmanagement
{
GLFWwindow* NativeWindow::initWindowHandle_ = nullptr;
bool NativeWindow::isWaitingEvents_ = true;

#ifdef __linux__
Display* NativeWindow::initDisplay_ = nullptr;
GLXContext NativeWindow::initContext_ = {};
#endif

NativeWindow::NativeWindow(const std::string& title, const glm::ivec2 size)
    : windowId_(utils::genId())
    , log_("Window({})/{}", title, windowId_)
    , title_(title)
    , startTime_(glfwGetTime())
    , delta_(0.0f)
{
    windowHandle_ = glfwCreateWindow(size.x, size.y, title_.c_str(), NULL, NULL);
    if (!windowHandle_)
    {
        log_.error("Could not create window!");
        return;
    }

    makeContextCurrent();
    enableVsync(true);
    maskEvents();

    input_.bindWindow(windowHandle_);

    log_.info("Window with id {} has been created!", windowId_);
}

NativeWindow::~NativeWindow()
{
    log_.debug("Window {} destroyed.", windowId_);
    glfwDestroyWindow(windowHandle_);
}

auto NativeWindow::init() -> bool
{
    glfwSetErrorCallback(
        [](int32_t error, const char* message)
        {
            utils::Logger internalLog("GLFWErrorCallback");
            internalLog.error("{}: {}", error, message);
        });
    
    if (!glfwInit())
    {
        utils::Logger internalLog("WINDOW");
        internalLog.error("Couldn't initialize GLFW.");
        return false;
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_VISIBLE, false);

    initWindowHandle_ = glfwCreateWindow(100, 100, "dummy", NULL, NULL);

    glfwWindowHint(GLFW_VISIBLE, true);
    glfwMakeContextCurrent(initWindowHandle_);

#ifdef __linux__
    initDisplay_ = glfwGetX11Display();
    initContext_ = glXGetCurrentContext();
#endif

    if (glewInit() != GLEW_OK)
    {
        utils::Logger internalLog("WINDOW");
        internalLog.error("Couldn't initialize GLEW.");
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    glDebugMessageCallback(
        [](uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length,
            const char* message, const void* userParam)
        {
            (void) source;
            (void) id;
            (void) length;
            (void) userParam;

            utils::Logger internalLog("WINDOW");
            internalLog.error("Type {} Severity {} Message {}", type, severity, message);
        }, nullptr );

    utils::Logger internalLog("WINDOW");
    internalLog.debug("GL Version {}", (const char*)glGetString(GL_VERSION));
    internalLog.debug("GLSL Version {}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    enableDepthTest();
    enableScissorsTest();
    enableAlphaBlending();

    return true;
}

auto NativeWindow::close() const -> void
{
    glfwSetWindowShouldClose(windowHandle_, true);
}

auto NativeWindow::makeContextCurrent() -> void
{
#ifdef __linux__
    glXMakeCurrent(initDisplay_, glfwGetX11Window(windowHandle_), initContext_);
#else
    glfwMakeContextCurrent(windowHandle_);
#endif
}

auto NativeWindow::swapBuffers() -> void
{
    if (!glfwGetCurrentContext())
    {
        utils::Logger("WINDOW").error("No context is bound!");
        return;
    }
#ifdef __linux__
    glXSwapBuffers(initDisplay_, glfwGetX11Window(windowHandle_));
#else
    glfwSwapBuffers(windowHandle_);
#endif

    const double nowTime = glfwGetTime();
    delta_ = nowTime - startTime_;
    startTime_ = nowTime;
}

auto NativeWindow::setTitle(std::string title, const bool updateInteralText) -> void
{
    if (updateInteralText) { title_ = std::move(title); }
    glfwSetWindowTitle(windowHandle_, title.c_str());
}

auto NativeWindow::shouldWindowClose() const -> bool
{
    return glfwWindowShouldClose(windowHandle_);
}

auto NativeWindow::getGlfwHandle() const -> GLFWwindow* { return windowHandle_; }

auto NativeWindow::getTitle() const -> std::string { return title_; }

auto NativeWindow::getSize() const -> glm::ivec2
{
    glm::ivec2 size;
    glfwGetWindowSize(windowHandle_, &size.x, &size.y);
    return size;
}

auto NativeWindow::getId() const -> uint64_t { return windowId_; }

auto NativeWindow::getDeltaTime() -> double { return delta_; }

auto NativeWindow::getInput() -> Input& { return input_; }

auto NativeWindow::terminate() -> void
{
    glfwTerminate();
    utils::Logger internalLog("WINDOW");
    internalLog.debug("Terminated.");
}

auto NativeWindow::enableVsync(const bool enable) -> void
{
#ifdef __linux__
    /* Unfortunately due to drivers or my limited knowledge we can only have the main window obey
    the vSync rule. As soon as there are 2 or more windows it looks like it doesn't want to apply anymore. */
    typedef int (*PFNGLXSWAPINTERVALMESAPROC)(unsigned int);
    PFNGLXSWAPINTERVALMESAPROC glXSwapIntervalMESA = reinterpret_cast<PFNGLXSWAPINTERVALMESAPROC>(
        glXGetProcAddressARB(reinterpret_cast<const GLubyte*>("glXSwapIntervalMESA")));
    if (glXSwapIntervalMESA)
    {
        glXSwapIntervalMESA(enable);
    }

#else
    glfwSwapInterval(enable);
#endif
}

auto NativeWindow::setWaitEvents(const bool waitEvents) -> void
{
    isWaitingEvents_ = waitEvents;
}

auto NativeWindow::pollEvents() -> void
{
    isWaitingEvents_ ? glfwWaitEvents() : glfwPollEvents();
}

auto NativeWindow::setClipboardData(const std::string& data) -> void
{
    glfwSetClipboardString(nullptr, data.c_str());
}

auto NativeWindow::enableDepthTest(const bool enable) -> void
{
    enable ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

auto NativeWindow::enableScissorsTest(const bool enable) -> void
{
    enable ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
}

auto NativeWindow::enableAlphaBlending(const bool enable) -> void
{
    if (enable)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        return;
    }

    glDisable(GL_BLEND);
}

auto NativeWindow::updateScissors(const glm::vec4& area) -> void
{
    glScissor(area.x, area.y, area.z, area.w);
}

auto NativeWindow::clearColor(const glm::vec4& color) -> void
{
    glClearColor(color.r, color.g, color.b, color.a);
}

auto NativeWindow::clearBuffers(uint32_t bufferBits) -> void
{
    glClear(bufferBits);
}

auto NativeWindow::clearAllBuffers() -> void
{
    clearBuffers(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

auto NativeWindow::updateViewport(const glm::ivec2 size) -> void
{
    glViewport(0, 0, size.x, size.y);
}

auto NativeWindow::getInitGlfwHandle() -> GLFWwindow* { return initWindowHandle_; }

auto NativeWindow::getTime() -> double { return glfwGetTime(); }

auto NativeWindow::getClipboardData() -> std::string { return glfwGetClipboardString(initWindowHandle_); }

auto NativeWindow::maskEvents() -> void
{
   /* On X11 systems when we have ONE context shared among MANY windows, the WM will spam out PropertyNotify
       events endlessly. This messes with glfwWaitEvents() blocking nature and behaves just like a
       glfwPollEvents() instead which is not what we want from a GUI app.
       The way around this is to tell the WM to not generate PropertyNotify events anymore by masking the
       attribute associated with that event.
       Not sure if the behavior is similar on Windows/MacOS. */

#ifdef __linux__
    XWindowAttributes attributes;

    XGetWindowAttributes(initDisplay_, glfwGetX11Window(windowHandle_), &attributes);

    long current_mask = attributes.your_event_mask;
    long new_mask = current_mask & ~PropertyChangeMask;

    XSelectInput(initDisplay_, glfwGetX11Window(windowHandle_), new_mask);
#endif
}
} // namespace src::windowmanagement