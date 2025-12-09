
#include "src/App.hpp"
#include "src/Core/Binders/WindowBinder.hpp"
#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LavParser/LavParser.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Core/ResourceHandler/Mesh.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIDropdown.hpp"
#include "src/Node/UIImage.hpp"
#include "src/Node/UILabel.hpp"
#include "src/Node/UISlider.hpp"
#include "src/Node/UIPane.hpp"
#include "src/Node/UIWindow.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"

using namespace lav::core;
using namespace lav::node;
using namespace lav;

int main()
{
    utils::Logger log("Main");

    log.debug("version {}", __cplusplus);

    App& app = App::get();

    if (!app.init()) { return 1; }

    app.enableTitleWithFPS();
    UIWindowWPtr window = app.loadLavView("views/test.xml");
    // UIWindowWPtr window = app.createWindow("myWindow", {1280, 720});
    // {
    //     auto lw = window.lock();
    //     auto pane = utils::as<UIPane>(lw->getElements()[0]);

    //     UIDropdownPtr dd = utils::make<UIDropdown>();
    //     dd->getBaseLayoutData().setScale({100_px, 1_fill});
    //     dd->setText("Star Wars");
    //     auto general = dd->addOption("General").lock();
    //     dd->addOption("Colonel");

    //     auto subMenu = dd->addSubMenu("Planets >").lock();
    //     subMenu->setPreferredOpenDir(UIDropdown::OpenDir::RIGHT);
    //     subMenu->addOption("Earth");
    //     auto endor = subMenu->addOption("Endor").lock();
    //     // endor->listenEvent<core::MouseLeftReleaseEvt>([&log](const auto&)
    //     endor->listenEvent<core::MouseLeftReleaseEvt>([&log](const auto&)
    //     {
    //         log.error("Released endor");
    //     });
    //     pane->add(dd);
    // }

    // UIPanePtr pn = utils::make<UIPane>();
    // pn->setScrollEnabled(true, true);
    // pn->setColor(utils::hexToVec4("#ffaaffff"));
    // pn->getBaseLayoutData().setScale({1_fill, 1_fill});
    // {
    //     auto wl = window.lock();
    //     wl->add(pn);
    // }

    // // pn->listenEvent<core::MouseEnterEvt>([&log, &pn](const auto&)
    // // {
    // //     log.info("Entered on my area");
    // //     pn->setColor(utils::hexToVec4("#037a3eff"));
    // // });

    // // pn->listenEvent<core::MouseExitEvt>([&log, &pn](const auto&)
    // // {
    // //     log.info("Exit on my area");
    // //     pn->setColor(utils::hexToVec4("#ffaaffff"));
    // // });

    // UIButtonPtr b = utils::make<UIButton>();
    // b->getBaseLayoutData().setScale({200_px, 20_px});
    // b->setText("Click me");

    // pn->add(b);

    app.run();
    return 0;
}