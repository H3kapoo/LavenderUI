
#include "src/App.hpp"
#include "src/ElementComposable/IEvent.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIPane.hpp"
#include "src/UIElements/UIBase.hpp"
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

    UIPanePtr p = utils::make<UIPane>();
    p->setScrollEnabled(true, true);
    p->setScrollSensitivityMultiplier(0.01);
    p->setScale({1.0_rel, 1.0_rel});
    p->setColor(utils::hexToVec4("#505050ed"));

    glm::vec4 alt{utils::hexToVec4("#dfdfdfff")};
    glm::vec4 alt2{utils::hexToVec4("#a8a8a8ff")};
    window.lock()->add(p);

    for (int32_t i = 0; i < 20; ++i)
    {
        UIButtonPtr b = utils::make<UIButton>();
        // b->setColor(i % 2 ? alt : alt2);
        b->setColor(utils::hexToVec4("#ffffffff"));
        b->setText(std::format("Id: {}", i));
        b->setMargin({2, 2, 2, 2});

        b->listenTo<MouseEnterEvt>(
            [rb = std::weak_ptr<UIButton>(b)](const auto&)
            {
                rb.lock()->setColor(utils::hexToVec4("#a3a3a3ff"));
            });

        b->listenTo<MouseExitEvt>(
            [rb = std::weak_ptr<UIButton>(b)](const auto&)
            {
                rb.lock()->setColor(utils::hexToVec4("#ffffffff"));
            });
        p->add(std::move(b));
    }
    // window.lock()->tempPosOffset = {30, 40};

    // p->setScale({0.5_rel, 400_px});

    // app.setWaitEvents(false);


    


    /* Blocks */
    app.run();
    return 0;
}