
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

    window.lock()->setAlign(LayoutBase::Align::CENTER);

    UIButtonPtr btn = utils::make<UIButton>();
    btn->setScale({200_px, 34_px})
        .setPos({200_abs, 200_abs});
    btn->setText("Testing..");

    for (int i = 0; i < 4; ++i)
    {
        UIButtonPtr b = utils::make<UIButton>();
        b->setScale({150, 34_px});
        b->setText("Not Testing..");
        window.lock()->add(b);
    }
    window.lock()->add(btn);
    window.lock()->listenTo<MouseLeftClickEvt>([&log, btn](const auto& e)
    {
        log.error("pos {} {}", e.x, e.y);
        btn->setPos({{e.x, LayoutBase::PositionType::ABS}, {e.y, LayoutBase::PositionType::ABS}});
    });

    for (int i = 0; i < 4; ++i)
    {
        UIButtonPtr b = utils::make<UIButton>();
        b->setScale({150, 34_px});
        b->setText("Not Testing..");
        window.lock()->add(b);
    }
    /* Blocks */
    app.run();
    return 0;
}