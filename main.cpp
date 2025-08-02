
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
    // sp->setColor(utils::hexToVec4("#290303ff"));
    // sp->setScale({1.0_rel, 1.0_rel});
    // // window.lock()->tempPosOffset = {30, 40};

    // window.lock()->add(sp);
    // // p->setScale({0.5_rel, 400_px});

    UIPanePtr p = utils::make<UIPane>();
    p->setScrollEnabled(true, true);
    p->setBorderColor(utils::hexToVec4("#b14cb1ff"));
    p->setScale({1.0_rel, 1.0_rel})
        // .setType(LayoutBase::Type::VERTICAL)
        .setAlign(LayoutBase::Align::CENTER)
        .setSpacing(LayoutBase::Spacing::EVEN_GAP)
        // .setWrap(true)
        .setPadding({4, 4, 4, 4})
        .setBorder({4, 4, 4, 4})
        ;

    p->setColor(utils::hexToVec4("#635a5aff"));

    UIPanePtr p2 = utils::make<UIPane>();
    // p2->setScale({0.5_rel, 0.5_rel});
    p2->setScale({1_fit, 1_fit});
    p2->setPadding({4, 4, 4, 4});
    p2->setBorder({4, 4, 4, 4});
    p2->setColor(utils::hexToVec4("#f52f2fff"));
    p->add(p2);

    // for (int32_t i = 0; i < 5; i++)
    for (int32_t i = 0; i < 3; i++)
    {
        UIButtonPtr b = utils::make<UIButton>();
        // b->setScale({1_fit, 1_fit});
        b->setScale({200_px, 100_px});
        b->setMargin({2, 2, 2, 2});
        b->setColor(utils::hexToVec4("#ffffff96"));
        // b->setText("ai");
        
        if (i == 1 || i == 3)
        {
            b->setScale({200_px, 400_px});
        }
        p2->add(b);
    }

    window.lock()->add(p);
    // app.setWaitEvents(false);

    /* Blocks */
    app.run();
    return 0;
}