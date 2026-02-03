#include <LavenderUI/Node/UIWindow.hpp>

#include <algorithm>
#include <optional>

#include <glm/ext/matrix_clip_space.hpp>
#include <LavenderUI/App.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/Character.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/FocusGain.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/FocusLost.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/Keyboard.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseButton.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseDrag.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseEnter.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseExit.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseLeftClick.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseLeftRelease.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseMove.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/MouseScroll.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/WindowFocus.hpp>
#include <LavenderUI/Core/EventHandler/CoreEvents/WindowResize.hpp>
#include <LavenderUI/Core/Binders/GPUBinder.hpp>
#include <LavenderUI/Core/Binders/WindowBinder.hpp>
#include <LavenderUI/Core/LayoutHandler/BaseCalculator.hpp>
#include <LavenderUI/Core/State/UIState.hpp>
#include <LavenderUI/Node/UIBase.hpp>
#include <LavenderUI/Node/UIDropdown.hpp>
#include <LavenderUI/Node/UIPane.hpp>
#include <LavenderUI/Node/UISlider.hpp>
#include <LavenderUI/Utils/Misc.hpp>
#include <LavenderUI/Node/InternalUse/UIScroll.hpp>

namespace lav::node
{
/* Static definitions */
int32_t UIWindow::MAX_LAYERS = 1000;
bool UIWindow::isFirstWindow_ = true;

UIWindow::UIWindow(const std::string& title, const glm::ivec2& size)
    : UIBase({"UIWindow", "elemVert.glsl", "elemFrag.glsl"})
    , window_(core::WindowBinder::get().createWindow(title, size))
    , title_(title)
    , uiState_(utils::make<core::UIState>())
    , startTime_(0)
    , deltaTime_(0)
    , shouldManuallyQuit_(false)
    , isMainWindow_(isFirstWindow_)
{
    initializeDefaultCursor();
    updateWindowSizeAndProjection(size);
    setupInputCallbacks();

    // core::GPUBinder::get().enable(core::GPUBinder::Function::SCISSORS, false);
    // core::GPUBinder::get().enable(core::GPUBinder::Function::DEPTH, false);
}

UIWindow::~UIWindow()
{
    core::WindowBinder::get().destroyWindow(window_);
    log_.debug("Window destroyed");
}

auto UIWindow::run() -> bool
{
    startTime_ = core::WindowBinder::get().getTime();

    /* General flow: Events, Layout, Render. Other. */
    resolvePendingRawEvents();

    /* Setup GPU backend and current window. */
    const auto& size = uiState_->windowSize;
    core::WindowBinder::get().makeContextCurrent(window_);
    core::GPUBinder::get().setViewportArea({0, 0, size.x, size.y});
    core::GPUBinder::get().setScissorsArea({0, 0, size.x, size.y});
    core::GPUBinder::get().clearColor(getColor());
    core::GPUBinder::get().clearAllBufferBits();

    /* Layout. */
    const bool shouldWiggleMouseInPlace = resolveLayoutTask();

    /*
        After layout is calculated it may happen that elements got removed/added and the previously
        calculated hovered element became invalid.
        In such cases, wiggle the mouse in place so that new events can propagate to the correct elements.
        In theory the bellow solver shouldn't trigger any element to add/remove elements again, but if they do,
        they shall calculate the layout again for themselves alone.
    */
    if (shouldWiggleMouseInPlace && uiState_->hoveredId != core::NOTHING)
    {
        // TODO: Maybe it can be improved even more to skip any redundant wiggles.
        mouseMoveSolver(uiState_->mousePos.x, uiState_->mousePos.y);
    }

    /* Render. */
    resolveRenderTask();

    /* Change current mouse cursor icon if needed. */
    if (uiState_->wantedCursorType.has_value())
    {
        core::WindowBinder::get().setStandardCursor(window_, uiState_->wantedCursorType.value());
        uiState_->currentCursorType = uiState_->wantedCursorType;
        uiState_->wantedCursorType.reset();
    }

    /* Buffer swapping and timings measurement. */
    core::WindowBinder::get().swapBuffers(window_);

    const double nowTime = core::WindowBinder::get().getTime();
    deltaTime_ = nowTime - startTime_;

    return core::WindowBinder::get().shouldWindowClose(window_) || shouldManuallyQuit_;
}

auto UIWindow::resolvePendingRawEvents() -> void
{
    /*
        Call the solvers for all pending events. This will trigger specific subevents for
        each impacted element (MouseRelease/MouseExit/etc). Don't forget to clear at the end.
    */
    for (const auto& rawEventCallback : pendingRawEventCallbacks_)
    {
        rawEventCallback();
    }
    clearAllUniquePendingRawEvents();
}

auto UIWindow::resolveLayoutTask() -> bool
{
    bool anyTreeChanged{false};
    processingQueue_.push(shared_from_this());
    while (!processingQueue_.empty())
    {
        UIBasePtr element = processingQueue_.front();
        processingQueue_.pop();

        setupStaticViewBoundsForElement(element);
        if (shouldLayoutBeComputedForElement(element))
        {
            element->onLayout();
        }
        calculateDynamicViewBoundsForChildElements(element);

        if (element->getAndConsumeTreeChangeIfAny()) { anyTreeChanged = true; }

        for (const auto& childEl : element->getElements()) { processingQueue_.push(childEl); }
    }

    return anyTreeChanged;
}

auto UIWindow::resolveRenderTask() -> void
{
    processingQueue_.push(shared_from_this());
    while (!processingQueue_.empty())
    {
        UIBasePtr element = processingQueue_.front();
        processingQueue_.pop();

        if (shouldElementBeRendered(element))
        {
            setupScissorAreaForElement(element, projection_);
            element->onRender(projection_);
        }

        for (const auto& childEl : element->getElements()) { processingQueue_.push(childEl); }
    }
}

auto UIWindow::quit() -> void { shouldManuallyQuit_ = true; }

auto UIWindow::initializeDefaultCursor() -> void
{
    if (!isFirstWindow_) { return; }

    core::WindowBinder::get().setStandardCursor(window_, lav::Cursor::ARROW);

    isFirstWindow_ = false;
}

auto UIWindow::setupInputCallbacks() -> void
{
    /*
        Each callback triggered by user interacting with the window will combe here and be
        inserted an a unique map of events alongside it's specific event solver.

        The map of unique events is needed due to the fact that during event polling the windowing API
        can receive, for example, multiple Mouse_Moved events and we don't want to process all of them.
        We acknowledge them but we only care about the latest event of that type.
    */
    cbs_ = {
        .keyCallback =
            [this](const uint32_t key, const uint32_t sc, const uint32_t action, const uint32_t mods)
            {
                insertUniquePendingRawEvent(core::KeyboardEvt{}, [this, key, sc, action, mods]()
                {
                    keyButtonSolver(key, sc, action, mods);
                });
            },
        .characterCallback = 
            [this](const uint32_t codepoint)
            {
                insertUniquePendingRawEvent(core::CharacterEvt{}, [this, codepoint]()
                {
                    characterSolver(codepoint);
                });
            },
        .mouseMoveCallback = 
            [this](const int32_t x, const int32_t y)
            {
                insertUniquePendingRawEvent(core::MouseMoveEvt{}, [this, x, y]()
                {
                    mouseMoveSolver(x, y);
                });
            },
        .mouseBtnCallback = 
            [this](const uint8_t btn, const uint8_t action)
            {
                insertUniquePendingRawEvent(core::MouseButtonEvt{}, [this, btn, action]()
                {
                    mouseButtonSolver(btn, action);
                });
            },
        .mouseScrollCallback = 
            [this](const int8_t xOffset, const int8_t yOffset)
            {
                insertUniquePendingRawEvent(core::MouseScrollEvt{}, [this, xOffset, yOffset]()
                {
                    mouseScrollSolver(xOffset, yOffset);
                });
            },
        .windowSizeCallback = 
            [this](const uint32_t newX, const uint32_t newY)
            {
                insertUniquePendingRawEvent(core::WindowResizeEvt{}, [this, newX, newY]()
                {
                    windowResizeSolver(newX, newY);
                });
            },
        .windowMouseEntered = 
            [this](const bool entered)
            {
                insertUniquePendingRawEvent(core::WindowResizeEvt{}, [this, entered]()
                {
                    windowMouseEnterSolver(entered);
                });
            },
        .windowFileDrop =
            [this](const int32_t count, const char** paths)
            {
                (void)count;
                (void)paths;
                for (int32_t i = 0; i < count; ++i) {}
            },
        .windowFocus =
            [this](const bool focused)
            {
                insertUniquePendingRawEvent(core::WindowFocusEvt{}, [this, focused]()
                {
                    windowFocusSolver(focused);
                });
            }
    };

    core::WindowBinder::get().setInputCallbacks(window_, cbs_);
}

auto UIWindow::updateWindowSizeAndProjection(const glm::ivec2 newSize) -> void
{
    uiState_->windowSizeDelta = newSize - uiState_->windowSize;
    uiState_->windowSize = newSize;

    /* Camera is looking into -Z by default. Here, higher Z means closer to the camera. */
    projection_ = glm::ortho(0.0f, (float)newSize.x, (float)newSize.y, 0.0f, -(float)MAX_LAYERS, 0.0f);
}

auto UIWindow::insertUniquePendingRawEvent(const core::IEvent& e, const RawEventCallback& cb) -> void
{
    if (pendingRawEventIds_.contains(e.getEventId())) { return; }

    pendingRawEventIds_.insert(e.getEventId());
    pendingRawEventCallbacks_.push_back(cb);
}

auto UIWindow::clearAllUniquePendingRawEvents() -> void
{
    pendingRawEventCallbacks_.clear();
    pendingRawEventIds_.clear();
}

auto UIWindow::emitEventTo(const core::IEvent& evt, const std::optional<uint32_t> nodeId) -> void
{
    uiState_->currentEventId = evt.getEventId();

    // TODO: Storing weak_ptrs to the nodes would be much more effici`ent
    // and we could use the processing queue just to find the hovered node
    processingQueue_.push(shared_from_this());
    while (!processingQueue_.empty())
    {
        UIBasePtr node = processingQueue_.front();
        processingQueue_.pop();

        if (node->isIgnoringEvents()) { continue; }

        if (!nodeId || nodeId.value() == node->getId())
        {
            node->onEvent(uiState_);
        }
        // Did some changes in test branch

        for (const auto& childNode : node->getElements()) { processingQueue_.push(childNode); }
    }

    uiState_->prevEventId = uiState_->currentEventId;
}

auto UIWindow::scanForHoveredNode() -> void
{
    /* No need to recheck the hovered item if we are currently holding onto something down. */
    if (uiState_->clickedId != core::NOTHING) { return; }

    /*
        TODO: Propagate functions work at nodeId level and each time we propagate something we need to
        go thru the tree and find the node, it's very inefficient.
        We could minimize the overhead by processing all the "queued" events in one pass.
        Maybe we could send the events in the run() processingQueue loop to have only one master loop over the
        entire tree, but not sure how that will affect the elements, we might process the event and that event
        changes the UI but the change is not reflected until the next loop. 
        However there's nothing stopping us from signaling the window a new loop pass needs to be done from events.
    */
    uint32_t maxZIndexSoFar{0};

    processingQueue_.push(shared_from_this());
    while (!processingQueue_.empty())
    {
        UIBasePtr node = processingQueue_.front();
        processingQueue_.pop();

        if (node->isIgnoringEvents()) { continue; }

        /* 
            Determine in the scan pass who's the hovered element. We need to ensure that the user's input will
            go to the highest index element.
        */
        if (node->layoutBase_.getZIndex() > maxZIndexSoFar
            && node->layoutBase_.isPointInsideView(uiState_->mousePos))
        {
            uiState_->hoveredId = node->getId();
            uiState_->hoveredTypeId = node->getTypeId();
            maxZIndexSoFar = node->layoutBase_.getZIndex();
        }

        for (const auto& childNode : node->getElements()) { processingQueue_.push(childNode); }
    }
}

auto UIWindow::mouseMoveSolver(const int32_t newX, const int32_t newY) -> void
{
    const glm::ivec2 newMouse = utils::clamp({newX, newY}, {0, 0}, uiState_->windowSize);
    uint32_t prevHoveredId = uiState_->hoveredId;
    uiState_->hoveredId = core::NOTHING;
    uiState_->mouseDiff = newMouse - uiState_->mousePos;
    uiState_->mousePos = newMouse;

    scanForHoveredNode();
    uint32_t currHoveredId = uiState_->hoveredId;

    /* Entered the window for the first time */
    if (prevHoveredId == core::NOTHING)
    {
        emitEventTo(core::MouseEnterEvt{}, currHoveredId);
    }
    /* Spawn exit for previous and enter for the current id */
    else if (prevHoveredId != currHoveredId)
    {
        uiState_->prevHoveredId = prevHoveredId;
        emitEventTo(core::MouseEnterEvt{}, currHoveredId);
        emitEventTo(core::MouseExitEvt{}, prevHoveredId);
    }

    /* Handle dragging on the clicked id */
    if (uiState_->clickedId != core::NOTHING
        && uiState_->mouseAction == Action::PRESS
        && uiState_->mouseButton == Mouse::LEFT)
    {
        uiState_->isDragging = true;
        emitEventTo(core::MouseDragEvt{}, uiState_->clickedId);
    }

    /* Spawn event itself */
    emitEventTo(core::MouseMoveEvt{}, std::nullopt);
}

auto UIWindow::mouseButtonSolver(const uint32_t btn, const uint32_t action) -> void
{
    uiState_->mouseButton = static_cast<lav::Mouse>(btn);
    uiState_->mouseAction = static_cast<lav::Action>(action);

    if (static_cast<lav::Mouse>(btn) == Mouse::LEFT && static_cast<lav::Action>(action) == Action::PRESS)
    {
        const uint32_t prevSelectedId = uiState_->selectedId;

        uiState_->clickedId = uiState_->hoveredId;
        uiState_->selectedId = uiState_->hoveredId;

        emitEventTo(core::MouseLeftClickEvt{}, uiState_->clickedId);

        if (prevSelectedId != uiState_->selectedId)
        {
            emitEventTo(core::FocusLostEvt{}, prevSelectedId);
            emitEventTo(core::FocusGainEvt{}, uiState_->selectedId);
        }
    }
    else if (static_cast<lav::Mouse>(btn) == Mouse::LEFT && static_cast<lav::Action>(action) == Action::RELEASE)
    {
        uiState_->isDragging = false;
        uiState_->clickedId = core::NOTHING;
        emitEventTo(core::MouseLeftReleaseEvt{}, uiState_->selectedId);
    }

    /* Spawn event itself */
    emitEventTo(core::MouseButtonEvt{}, std::nullopt);
}

auto UIWindow::mouseScrollSolver(const uint32_t xOffset, const uint32_t yOffset) -> void
{
    uiState_->scrollOffset = {xOffset, yOffset};

    /* Simulate mouse wiggling in place as underlying hoveredId might change. */
    mouseMoveSolver(uiState_->mousePos.x, uiState_->mousePos.y);

    emitEventTo(core::MouseScrollEvt{}, uiState_->hoveredId);
    if (uiState_->hoveredId != uiState_->closestScrollId
        && uiState_->hoveredTypeId != node::UISlider::typeId)
    {
        emitEventTo(core::MouseScrollEvt{}, uiState_->closestScrollId);
    }
}

auto UIWindow::windowResizeSolver(const uint32_t x, const uint32_t y) -> void
{
    /* Note: use framebuffer size to set viewport in case DPI is not a default
       one aka we have some artificial scaling. */
    updateWindowSizeAndProjection(glm::ivec2{x, y});
    emitEventTo(core::WindowResizeEvt{}, std::nullopt);
}

auto UIWindow::windowMouseEnterSolver(const bool entered) -> void
{
    if (entered)
    {
        mouseMoveSolver(uiState_->mousePos.x, uiState_->mousePos.y);
    }
    else
    {
        emitEventTo(core::MouseExitEvt{}, uiState_->hoveredId);
        uiState_->hoveredId = core::NOTHING;
        uiState_->prevHoveredId = core::NOTHING;
    }
}

auto UIWindow::windowFocusSolver(const bool focused) -> void
{
    /* If focus is lost, the selectedId shall be notified of loss of focus aswell. */
    if (!focused && uiState_->selectedId != core::NOTHING)
    {
        emitEventTo(core::FocusLostEvt{}, uiState_->selectedId);
    }
}

auto UIWindow::keyButtonSolver(const uint32_t key, const uint32_t, const uint32_t action,
    const uint32_t) -> void
{
    const lav::Key castKey = static_cast<lav::Key>(key);
    const lav::Action castAction = static_cast<lav::Action>(action);

    //TODO: REPEAT Shall have a rate limiter
    if (castAction == Action::RELEASE || castAction == Action::REPEAT) { return; }

    if (castKey == Key::ESC)
    {
        core::WindowBinder::get().close(window_);
    }
    // TODO: This is now broken and it makes the code crash.
    // Most probably its something to do with adding while iterating in ::App
    else if (castKey == Key::C)
    {
        // App::get().createWindow("new_frame" + std::to_string(id_), {200, 300});
        App::get().createWindow("new_frame", {200, 300});
    }
    else if (castKey == Key::P)
    {
        // log_.debug("\n{}", shared_from_this());
    }

    uiState_->keyStates[key] = static_cast<lav::Action>(action);
    uiState_->keyRecent = castKey;
    if (uiState_->selectedId != core::NOTHING)
    {
        emitEventTo(core::KeyboardEvt{}, uiState_->selectedId);
    }
}

auto UIWindow::characterSolver(const uint32_t codepoint) -> void
{
    uiState_->codepointRecent = codepoint;
    if (uiState_->selectedId != core::NOTHING)
    {
        emitEventTo(core::CharacterEvt{}, uiState_->selectedId);
    }
}

auto UIWindow::onRender(const glm::mat4& projection) -> void { (void)projection; }

auto UIWindow::onLayout() -> void
{
    layoutBase_.setComputedScale(uiState_->windowSize);

    const auto& calculator = core::BaseCalculator::get();
    calculator.calculateScaleForGenericElement(this);
    calculator.calculatePositionForGenericElement(this);

    const auto overflow = calculator.calculateElementOverflow(this, {0, 0});
    calculator.calculateAlignmentForElements(this, overflow);
}

auto UIWindow::onEvent(core::UIStatePtr& state) -> void
{
    const auto eId = state->currentEventId;
    if (eId == core::MouseEnterEvt::eventId)
    {
        core::MouseEnterEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseEnterEvt>(e);
    }
    else if (eId == core::MouseExitEvt::eventId)
    {
        core::MouseExitEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseExitEvt>(e);
    }
    else if (eId == core::MouseLeftClickEvt::eventId)
    {
        core::MouseLeftClickEvt e{state->mousePos.x, state->mousePos.y};
        eventsMgr_.emitEvent<core::MouseLeftClickEvt>(e);
    }
    else if (eId == core::MouseLeftReleaseEvt::eventId)
    {
        core::MouseLeftReleaseEvt e;
        eventsMgr_.emitEvent<core::MouseLeftReleaseEvt>(e);
    }
}

auto UIWindow::shouldElementBeRendered(const UIBasePtr& element) -> bool
{
    /*
        Only the UIWIndow should be rendered at any time. Other element's render ability depends
        on if they are at least visible to the user.
    */
    if (element->getTypeId() == UIWindow::typeId) { return true; }

    if (!element || !element->isParented()) { return false; }

    const auto& eLayout = element->getBaseLayoutData();
    const auto& viewScale = eLayout.getViewScale();

    return viewScale.x > 0 && viewScale.y > 0;
}

auto UIWindow::shouldLayoutBeComputedForElement(const UIBasePtr& element) -> bool
{
    /*
        Only the UIWIndow should be computed at any time. Other element's compute ability depends
        on if they are at least visible to the user.
    */
    if (element->getTypeId() == UIWindow::typeId) { return true; }

    if (!element || !element->isParented()) { return false; }

    const auto& eLayout = element->getBaseLayoutData();
    const auto& viewScale = eLayout.getViewScale();
    return viewScale.x > 0 && viewScale.y > 0;
}

auto UIWindow::setupScissorAreaForElement(const UIBasePtr& element, const glm::mat4& projection) -> void
{
    const auto& eLayout = element->getBaseLayoutData();
    const auto& viewPos = eLayout.getViewPos();
    const auto& viewScale = eLayout.getViewScale();
    core::GPUBinder::get().setScissorsArea(
        {
            viewPos.x,
            std::round((-2.0f / projection[1][1])) - viewPos.y - viewScale.y,
            viewScale.x,
            viewScale.y
        });
}

auto UIWindow::setupStaticViewBoundsForElement(const UIBasePtr& element) -> void
{
    /* If element is UIWindow scissor area is the whole element area. */
    if (element->getTypeId() == UIWindow::typeId)
    {
        auto& nLayout = element->getBaseLayoutData();
        nLayout.setViewPos({0, 0});
        nLayout.setViewScale(uiState_->windowSize);
    }
}

auto UIWindow::calculateDynamicViewBoundsForChildElements(const UIBasePtr& element) -> void
{
    /*
        After calculating the parent element, go through all it's child elements and calculate their view
        scale and position based on how viewable the parent element is. We don't want child elements to oveflow
        the parent's bounds.
        Child elements ZIndex will also be setup here based on UI type and parent's ZIndex.
    */
    std::ranges::for_each(element->getElements(),
        [&element, this](const auto& it)
        {
            auto& itLayout = it->getBaseLayoutData();
            const auto& nodeLayout = element->getBaseLayoutData();
            itLayout.computeViewBox(element->getBaseLayoutData());

            /* Index is used for layer rendering order with DEPTH_TEST enabled. */
            if (!itLayout.isCustomIndex())
            {
                itLayout.setZIndex(nodeLayout.getZIndex() + 1);
            }

            /*
                UIPanes that hold the options of a UIDropdown need to be some offset higher so as to not
                be occluded by the UILabels of the options.
                If element is UIPane but parent is UIDropdown scissor area is the whole UIPane area.
            */
            if (element->getTypeId() == UIDropdown::typeId && it->getTypeId() == UIPane::typeId)
            {
                itLayout.setZIndex(nodeLayout.getZIndex() + UIDropdown::dropdownIndexOffset);
                itLayout.setViewPos(itLayout.getComputedPos());
                itLayout.setViewScale(itLayout.getComputedScale());
            }

            /* UIScroll elements of a pane will have a higher custom ZIndex */
            if (it->getTypeId() == UIScroll::typeId)
            {
                itLayout.setZIndex(UIScroll::scrollIndexOffset - nodeLayout.getZIndex());
            }

            /* Depth is used mostly for printing. */
            it->depth_ = element->depth_ + 1;
        });
}

auto UIWindow::setTitle(std::string title, const bool onlyForShow) -> void
{
    (void)onlyForShow;
    // title_ = std::move(title);
    core::WindowBinder::get().setTitle(window_, title);
}

auto UIWindow::getTitle() -> std::string { return title_; }

auto UIWindow::getWindow() -> core::WindowHandle { return window_; }

auto UIWindow::getDeltaTime() -> double { return deltaTime_; }

auto UIWindow::isMainWindow() -> bool { return isMainWindow_; }
} // namespace lav::node