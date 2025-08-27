
#include "src/App.hpp"
#include "src/ElementComposable/IEvent.hpp"
#include "src/ElementComposable/LayoutBase.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIPane.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UISplitPane.hpp"
#include "src/UIElements/UIWindow.hpp"
#include "src/UIElements/UITreeView.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"
#include "src/WindowManagement/Input.hpp"

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

    UISplitPanePtr sp = utils::make<UISplitPane>();

    const float frac = 1.0f / 3;
    sp->setColor(utils::hexToVec4("#dbdbdbff"))
        .setBorderColor(utils::hexToVec4("#ffffffff"));
    sp->setScale({1_fill})
        .setType(LayoutBase::Type::VERTICAL)
        ;
    sp->createPanes({frac, frac, frac});

    sp->getPaneIdx(0).lock()->setMaxScale({400, 2000});
    sp->getPaneIdx(1).lock()->setMaxScale({4000, 2000});
    sp->getPaneIdx(2).lock()->setMaxScale({7000, 2000});
    sp->getPaneIdx(0).lock()->setMinScale({20, 20});
    sp->getPaneIdx(1).lock()->setMinScale({20, 20});
    sp->getPaneIdx(2).lock()->setMinScale({20, 20});

    UISplitPanePtr sp2 = utils::make<UISplitPane>();
    sp->getPaneIdx(0).lock()->add(sp2);
    sp2->createPanes({frac, frac, frac});
    sp2->setScale({1_fill});
    sp2->getPaneIdx(0).lock()->setMaxScale({400, 2000});
    sp2->getPaneIdx(1).lock()->setMaxScale({4000, 2000});
    sp2->getPaneIdx(2).lock()->setMaxScale({7000, 2000});
    sp2->getPaneIdx(0).lock()->setMinScale({20, 20});
    sp2->getPaneIdx(1).lock()->setMinScale({20, 20});
    sp2->getPaneIdx(2).lock()->setMinScale({20, 20});

    window.lock()->add(sp);
    log.debug("{}", window.lock());

    /* Blocks */
    app.run();
    return 0;
}