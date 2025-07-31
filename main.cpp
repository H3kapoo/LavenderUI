
#include "src/App.hpp"
#include "src/ElementComposable/IEvent.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIPane.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UISplitPane.hpp"
#include "src/UIElements/UIWindow.hpp"
#include "src/UIElements/UITreeView.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"
#include <memory>

using namespace src::windowmanagement;
using namespace src::uielements;
using namespace src::elementcomposable;
using namespace src;

int main()
{
    utils::Logger log("Main");

    log.debug("version {}", __cplusplus);

    App& app = App::get();

    if (!app.init()) { return 1; }
    app.enableTitleWithFPS();

    // uielements::UIFrameWPtr win = app.createFrame("myWindow", glm::ivec2{1280, 720});
    UIWindowWPtr window = app.createWindow("myWindow", {1280, 720});
    // uielements::UIFrameWPtr frame2 = app.createFrame("myWindow 2", glm::ivec2{680, 720});

    // UISplitPanePtr sp = utils::make<UISplitPane>();

    // const float frac = 1.0f / 5;
    // // sp->createPanes({frac, frac});
    // sp->createPanes({frac, frac, frac, frac, frac});
    // sp->setColor(utils::hexToVec4("#3a3a3aff"));
    // sp->setScale({1.0_rel, 1.0_rel});
    // // window.lock()->tempPosOffset = {30, 40};

    // window.lock()->add(sp);
    // // p->setScale({0.5_rel, 400_px});

    UIPanePtr p = utils::make<UIPane>();
    p->setScrollEnabled(true, true);
    p->setScale({1.0_rel, 1.0_rel});
        // .setBorder({4, 4, 4, 4});

    p->setColor(utils::hexToVec4("#635a5aff"));

    for (int32_t i = 0; i < 10; i++)
    {
        UIButtonPtr b = utils::make<UIButton>();
        // b->setMargin({3, 2, 5, 4});
        // b->setMargin({4, 4, 4, 4});
        b->setColor(utils::randomRGB());
        p->add(b);
    }

    window.lock()->add(p);
    // app.setWaitEvents(false);

    /* Blocks */
    app.run();
    return 0;
}