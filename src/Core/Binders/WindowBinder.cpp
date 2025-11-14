#include "WindowBinder.hpp"
#include "vendor/glfw/include/GLFW/glfw3.h"

namespace lav::core
{
auto WindowBinder::get() -> WindowBinder&
{
    static WindowBinder instance;
    return instance;
}

auto WindowBinder::init() -> bool
{
    glfwSetErrorCallback(
        [](int32_t error, const char* message)
        {
            utils::Logger internalLog("WindowBinder");
            internalLog.error("{}: {}", error, message);
        });
    
    if (!glfwInit())
    {
        log_.error("Couldn't initialize GLFW.");
        return false;
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_VISIBLE, false);

    initWindowHandle_ = glfwCreateWindow(100, 100, "dummy", NULL, NULL);

    glfwWindowHint(GLFW_VISIBLE, true);
    glfwMakeContextCurrent(initWindowHandle_);
    enableVSync(true);

#ifdef __linux__
    initDisplay_ = glfwGetX11Display();
    initContext_ = glXGetCurrentContext();
#endif

    cursors_[Cursor::ARROW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    cursors_[Cursor::IBEAM] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    cursors_[Cursor::CROSSHAIR] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    cursors_[Cursor::HAND] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    cursors_[Cursor::HRESIZE] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    cursors_[Cursor::VRESIZE] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    cursors_[Cursor::ALLRESIZE] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
    cursors_[Cursor::NOT_ALLOWED] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);

    log_.debug("GL Version {}", (const char*)glGetString(GL_VERSION));
    log_.debug("GLSL Version {}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    return true;
}

auto WindowBinder::terminate() -> void
{
    for (auto&[key, cursor] : cursors_) { destroyCursor(cursor); }

    glfwTerminate();
    log_.debug("Terminated.");
}

auto WindowBinder::createWindow(const std::string& title, const glm::ivec2 size) -> WindowHandle
{
    WindowHandle windowHandle = glfwCreateWindow(size.x, size.y, title.c_str(), NULL, NULL);
    if (!windowHandle)
    {
        log_.error("Could not create window!");
        return nullptr;
    }

    makeContextCurrent(windowHandle);
    maskEvents(windowHandle);
    enableVSync(true);

    log_.info("Window '{}/{{{}, {}}}' has been created!", title, size.x, size.y);
    return windowHandle;
}

auto WindowBinder::makeContextCurrent(WindowHandle handle) -> void
{
#ifdef __linux__
    glXMakeCurrent(initDisplay_, glfwGetX11Window(handle), initContext_);
#else
    glfwMakeContextCurrent(windowHandle_);
#endif
}

auto WindowBinder::enableVSync(const bool enable) -> void
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
    else
    {
        log_.error("Not found {}", __func__);
    }

#else
    glfwSwapInterval(enable);
#endif
}

auto WindowBinder::maskEvents(WindowHandle handle) -> void
{
   /* On X11 systems when we have ONE context shared among MANY windows, the WM will spam out PropertyNotify
       events endlessly. This messes with glfwWaitEvents() blocking nature and behaves just like a
       glfwPollEvents() instead which is not what we want from a GUI app.
       The way around this is to tell the WM to not generate PropertyNotify events anymore by masking the
       attribute associated with that event.
       Not sure if the behavior is similar on Windows/MacOS. */

#ifdef __linux__
    XWindowAttributes attributes;

    XGetWindowAttributes(initDisplay_, glfwGetX11Window(handle), &attributes);

    long current_mask = attributes.your_event_mask;
    long new_mask = current_mask & ~PropertyChangeMask;

    XSelectInput(initDisplay_, glfwGetX11Window(handle), new_mask);
#endif
}

auto WindowBinder::setCursor(WindowHandle handle, WindowCursor cursor) -> void
{
    glfwSetCursor(handle, cursor);
}

auto WindowBinder::setStandardCursor(WindowHandle handle, lav::Cursor cursor) -> void
{
    setCursor(handle, cursors_[cursor]);
}

auto WindowBinder::destroyCursor(WindowCursor cursor) -> void
{
    glfwDestroyCursor(cursor);
}

auto WindowBinder::swapBuffers(WindowHandle handle) -> void
{
    if (!glfwGetCurrentContext())
    {
        utils::Logger("WINDOW").error("No context is bound!");
        return;
    }
#ifdef __linux__
    glXSwapBuffers(initDisplay_, glfwGetX11Window(handle));
#else
    glfwSwapBuffers(handle);
#endif
}

auto WindowBinder::shouldWindowClose(WindowHandle handle) -> bool
{
    return glfwWindowShouldClose(handle);
}

auto WindowBinder::close(WindowHandle handle) -> void
{
    glfwSetWindowShouldClose(handle, true);
}

auto WindowBinder::setTitle(WindowHandle handle, const std::string& title) -> void
{
    glfwSetWindowTitle(handle, title.c_str());
}

auto WindowBinder::setPollWaitForEvents(const bool wait) -> void { pollingMethodIsWait_ = wait; }

auto WindowBinder::pollEvents() -> void
{
    pollingMethodIsWait_ ? glfwWaitEvents() : glfwPollEvents();
}

auto WindowBinder::destroyWindow(WindowHandle handle) -> void
{
    glfwDestroyWindow(handle);
}

auto WindowBinder::getTime() -> double
{
    return glfwGetTime();
}

auto WindowBinder::setUserPointer(WindowHandle handle, void* data) -> void
{
    glfwSetWindowUserPointer(handle, data);
}

auto WindowBinder::setInputCallbacks(WindowHandle handle, const InputCallbacks& cbs) -> void
{
    setUserPointer(handle, (void*)&cbs);

    glfwSetKeyCallback(handle,
        [](WindowHandle returnHandle, int32_t key, int32_t scanCode, int32_t action, int32_t mods)
        {
            const InputCallbacks* cbsData = static_cast<InputCallbacks*>(
                glfwGetWindowUserPointer(returnHandle));
            cbsData->keyCallback(key, scanCode, action, mods);
        });

    glfwSetCharCallback(handle,
        [](WindowHandle returnHandle, uint32_t codepoint)
        {
            const InputCallbacks* cbsData = static_cast<InputCallbacks*>(
                glfwGetWindowUserPointer(returnHandle));
            cbsData->characterCallback(codepoint);
        });

    glfwSetWindowSizeCallback(handle,
        [](WindowHandle returnHandle, int32_t x, int32_t y)
        {
            const InputCallbacks* cbsData = static_cast<InputCallbacks*>(
                glfwGetWindowUserPointer(returnHandle));
            cbsData->windowSizeCallback(x, y);
        });
    
    glfwSetCursorPosCallback(handle,
        [](WindowHandle returnHandle, double x, double y)
        {
            const InputCallbacks* cbsData = static_cast<InputCallbacks*>(
                glfwGetWindowUserPointer(returnHandle));
            cbsData->mouseMoveCallback(x, y);
        });
    
    glfwSetMouseButtonCallback(handle,
        [](WindowHandle returnHandle, int32_t btn, int32_t action, int32_t)
        {
            const InputCallbacks* cbsData = static_cast<InputCallbacks*>(
                glfwGetWindowUserPointer(returnHandle));
            cbsData->mouseBtnCallback(btn, action);
        });

    glfwSetScrollCallback(handle,
        [](WindowHandle returnHandle, double xOffset, double yOffset)
        {
            const InputCallbacks* cbsData = static_cast<InputCallbacks*>(
                glfwGetWindowUserPointer(returnHandle));
            cbsData->mouseScrollCallback(xOffset, yOffset);
        });

    glfwSetCursorEnterCallback(handle,
        [](WindowHandle returnHandle, int32_t entered)
        {
            const InputCallbacks* cbsData = static_cast<InputCallbacks*>(
                glfwGetWindowUserPointer(returnHandle));
            cbsData->windowMouseEntered(entered);
        });

    glfwSetDropCallback(handle,
        [](WindowHandle returnHandle, int32_t count, const char** paths)
        {
            const InputCallbacks* cbsData = static_cast<InputCallbacks*>(
                glfwGetWindowUserPointer(returnHandle));
            cbsData->windowFileDrop(count, paths);
        });
}

} // namespace lav::core