#include <print>
#include <type_traits>

#include "src/App.hpp"
#include "src/ElementEvents/IEvent.hpp"
#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/MeshLoader.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIFrame.hpp"
#include "src/UIElements/UIImage.hpp"
#include "src/Utils/Logger.hpp"

#include "src/Utils/Misc.hpp"
#include "vendor/freetype/include/freetype/freetype.h"

using namespace src::windowmanagement;
using namespace src::uielements;
using namespace src::elementevents;
using namespace src;

int main()
{
    utils::Logger log("Main");

    log.debug("version {}", __cplusplus);

    App& app = App::get();

    if (!app.init()) { return 1; }

    // uielements::UIFrameWPtr win = app.createFrame("myWindow", glm::ivec2{1280, 720});
    UIFrameWPtr frame = app.createFrame("myWindow", {1280, 720});
    // uielements::UIFrameWPtr frame2 = app.createFrame("myWindow 2", glm::ivec2{680, 720});

    // UIButton::type();
    // UIButton::typeId;
    UIBasePtr a = utils::make<UIButton>();
    UIButtonPtr b = utils::make<UIButton>();
    UIButtonPtr c = utils::make<UIButton>();
    UIButtonPtr d = utils::make<UIButton>();

    // uielements::UIBasePtrVec x = {a, b, c, d, e, f, g};

    // a->render({});
    frame.lock()->add({a});
    a->getLayout().pos = {150, 100};
    a->getLayout().scale = {300, 300};
    a->getVisual().color = {0, 0, 0, 1};
    b->getLayout().pos = {390, 140};

    a->getEvents().listen<MouseExitEvt>([&log, &a](const MouseExitEvt& e)
    {
        a->getVisual().color = {0, 0, 0, 1};
        log.warn("exit called {}", e.getType());
    });

    a->getEvents().listen<MouseEnterEvt>([&log, &a](const MouseEnterEvt& e)
    {
        log.warn("enter called {}", e.getType());
        a->getVisual().color = {1, 0, 1, 1};
    });

    a->getEvents().listen<MouseButtonEvt>([&log, &a](const MouseButtonEvt& e)
    {
        log.warn("click called {} {}", e.btn, e.action);
    });

    a->add(b);
    b->add(d);


    // b->getEvents().listen<MouseExitEvt>([&log, &b](const MouseExitEvt& e)
    // {
    //     b->getVisual().color = {0, 0, 0, 1};
    //     log.error("exit called {}", e.getType());
    // });

    // b->getEvents().listen<MouseEnterEvt>([&log, &b](const MouseEnterEvt& e)
    // {
    //     log.error("enter called {}", e.getType());
    //     b->getVisual().color = {1, 0, 1, 1};
    // });

    // b->getEvents().listen<MouseButtonEvt>([&log, &b](const MouseButtonEvt& e)
    // {
    //     log.error("click called {} {}", e.btn, e.action);
    // });
    std::println("{}", frame.lock());
    // std::println("{}", a->getTypeId());
    // std::println("{}", frame.lock()->getTypeId());
    // std::println("{}", b->getTypeId());
    // a->add(uielements::UIBasePtrVec{});
    // a->add(x);
    // a->add(std::move(x));
    // a->add({a, b});
    // FT_Library ftLib;
    // if (FT_Init_FreeType(&ftLib))
    // {
    //     std::cout << "testing not good\n";
    // }
    // FT_Done_FreeType(ftLib);
    // log.warn("hey there");

    // App::setVsync(false);
    // app.setWaitEvents(false);

    /* Blocks */
    app.run();
    return 0;
}