
#include "src/App.hpp"
#include "src/ElementComposable/IEvent.hpp"
#include "src/ElementComposable/LayoutBase.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIDropdown.hpp"
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

    UIWindowWPtr window = app.createWindow("myWindow", {1280, 720});

    window.lock()->setAlign(LayoutBase::Align::CENTER);

    UIDropdownPtr dd = utils::make<UIDropdown>();
    dd->setScale({200_px, 34_px})
        .setPos({200_abs, 200_abs});
    dd->setText("Testing..");

    UIButtonWPtr opt1 = dd->addOption("Test");
    UIButtonWPtr opt2 = dd->addOption("Test2");
    UIButtonWPtr opt3 = dd->addOption("Test3");

    {
        UIDropdownPtr dd2 = dd->addSubMenu("Submenu").lock();
        dd2->setPreferredOpenDir(UIDropdown::OpenDir::RIGHT);
        UIButtonWPtr opt4 = dd2->addOption("SubOption_1");
        UIButtonWPtr opt5 = dd2->addOption("SubOption_2");

        opt5.lock()->listenTo<MouseLeftReleaseEvt>(
        [&log](const auto&)
        {
            log.error("clicked me deep");
        });

        UIDropdownPtr dd3 = dd2->addSubMenu("Submenu Deep").lock();
        dd3->setPreferredOpenDir(UIDropdown::OpenDir::RIGHT);

        UIButtonWPtr opt6 = dd3->addOption("SubOption_3");
        UIButtonWPtr opt7 = dd3->addOption("SubOption_4");

        opt7.lock()->listenTo<MouseLeftReleaseEvt>(
        [&log, &window](const auto&)
        {
            log.error("clicked me deeply");
            window.lock()->quit();
        });

    }

    opt3.lock()->listenTo<MouseLeftReleaseEvt>(
        [&log](const auto&)
        {
            log.error("clicked me");
        });
    window.lock()->add(dd);
    // window.lock()->listenTo<MouseLeftClickEvt>([&log, btn](const auto& e)
    // {
    //     log.error("pos {} {}", e.x, e.y);
    //     btn->setPos({{e.x, LayoutBase::PositionType::ABS}, {e.y, LayoutBase::PositionType::ABS}});
    // });

    /*
        - Dropdown
            - Pane (floating pos)
                - Buttons inside for clicking
                - Buttons inside for clicking
                - Dropdown_2
                    - Pane (floating pos)
                    - etc
    */
    /* Blocks */
    app.run();
    return 0;
}