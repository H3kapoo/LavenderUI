
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
    

    UIPanePtr p = utils::make<UIPane>();
    p->setColor(utils::hexToVec4("#5c5c5cff"));
    p->setAlign(LayoutBase::CENTER);
    // p->setWrap(true);
    p->setScrollEnabled(true, true);
    p->setScale({1_fill, 1_fill});
    window.lock()->add(p);

    for (int32_t i = 0; i < 5; ++i)
    {
        UIButtonPtr b = utils::make<UIButton>();
        b->setText(std::format("ceva {}", i));
        b->setBorderRadius(4).setMargin({4}).setBorder({1});
        b->setColor(utils::hexToVec4("#c73e3eff"));
        b->listenTo<MouseLeftClickEvt>([&log, i](const auto&)
        {
            log.warn("clicked on me {}", i);
        });
        b->listenTo<MouseLeftReleaseEvt>([&log, i](const auto&)
        {
            log.warn("released on me {}", i);
        });
        p->add(b);
    }
    // // // p->setScale({0.5_rel, 400_px});
    // for (int32_t i = 0; i < 4; ++i)
    // {
    //     UIPanePtr newpane = utils::make<UIPane>();
    //     newpane->setColor(utils::hexToVec4("#a31da3ff"));
    //     newpane->setScale({250_px, 180_px});
    //     newpane->setMargin({4, 4, 4, 4});
    //     p->add(newpane);

    //     if (i == 2)
    //     {
    //         newpane->setScrollEnabled(true, true);
    //         for (int32_t j = 0; j < 4; ++j)
    //         {
    //             UIButtonPtr b = utils::make<UIButton>();
    //             b->setColor(utils::randomRGB());
    //             newpane->add(b);
    //         }
    //     }
    // }

    // UIPanePtr newpane = utils::make<UIPane>();
    // newpane->setBorderColor(utils::hexToVec4("#a31da3ff"));
    // newpane->setScale({1.0_rel, 1.0_rel});
    // window.lock()->add(newpane);

    // UIPanePtr p = utils::make<UIPane>();
    // p->setScrollEnabled(true, true);
    // p->setBorderColor(utils::hexToVec4("#b14cb1ff"));
    // p->setScale({1.0_rel, 1.0_rel})
    //     // .setType(LayoutBase::Type::VERTICAL)
    //     .setAlign(LayoutBase::Align::BOTTOM_RIGHT)
    //     // .setSpacing(LayoutBase::Spacing::EVEN_GAP)
    //     // .setWrap(true)
    //     // .setPadding({4, 4, 4, 4})
    //     // .setBorder({4, 4, 4, 4})
    //     ;

    // p->setColor(utils::hexToVec4("#635a5aff"));

    // UIPanePtr p2 = utils::make<UIPane>();
    // // p2->setScale({0.5_rel, 0.5_rel});
    // p2->setScale({1_fit, 1_fit});
    // // p2->setPadding({4, 4, 4, 4});
    // // p2->setBorder({4, 4, 4, 4});
    // p2->setColor(utils::hexToVec4("#000000ff"));
    // p->add(p2);

    // // for (int32_t i = 0; i < 5; i++)
    // for (int32_t i = 0; i < 3; i++)
    // {
    //     UIPanePtr b = utils::make<UIPane>();
    //     // b->setScale({1_fit, 1_fit});
    //     b->setScale({200_px, 100_px});
    //     b->setMargin({1, 1, 1, 1});
    //     b->setColor(utils::hexToVec4("#ffffff96"));
        
    //     if (i == 1 || i == 3)
    //     {
    //         b->setScale({1_fit, 1_fit});
    //         b->setType(LayoutBase::Type::VERTICAL);
    //         for (int32_t j = 0; j < 3; ++j)
    //         {
    //             UIPanePtr c = utils::make<UIPane>();
    //             c->setScale({130_px, 50_px});
    //             c->setMargin({2});
    //             c->setColor(utils::hexToVec4("#1b6bc7d5"));
    //             b->add(c);
    //         }
    //         // b->setScale({200_px, 400_px});
    //     }
    //     p2->add(b);
    // }

    // window.lock()->add(p);
    // app.setWaitEvents(false);

    /* Blocks */
    app.run();
    return 0;
}