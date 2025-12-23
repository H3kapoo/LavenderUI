
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
#include "src/Node/UISplitPane.hpp"
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
    window.lock()->setColor(utils::hexToVec4("#ffffffff"));

    UISplitPanePtr sp = utils::make<UISplitPane>();
    sp->setColor(utils::hexToVec4("#ffffffff"));
    sp->getBaseLayoutData()
        .setScale({1_fill, 1_fill})
        .setType(LayoutBase::Type::HORIZONTAL);

    auto p = sp->createPane(1.0f, {30, 10'000});
    auto p2 = sp->createPane(3.0f, {30, 10'000});
    // sp->removePaneIdx(0);
    // sp->removePaneIdx(0);
    // sp->removePaneIdx(0);
    auto p3 = sp->createPane(3.0f, {30, 10'000});

    auto sp3 = sp->createSubsplit(4.0f, {30, 10'000});
    // auto sp3 = sp->createPane(4.0f, {30, 10'000});

    sp3.lock()->getBaseLayoutData().setType(LayoutBase::Type::VERTICAL);
    auto pp = sp3.lock()->createPane(1.0f, {30, 10'000});
    auto pp2 = sp3.lock()->createPane(1.0f, {30, 10'000});
    
    auto dd = utils::as<UIDropdown>(window.lock()->getElements()[0]->getElements()[0]);
    auto firstOpt = utils::as<UIButton>(dd->getOptionsHolder().lock()->getElements()[0]);
    firstOpt->listenEvent<core::MouseLeftReleaseEvt>([&log, sp](const auto&)
    {
        log.info("clicked on first option");
        auto ppx = sp->createPane(2.0f, {30, 10'000});
    });

    auto secondOpt = utils::as<UIButton>(dd->getOptionsHolder().lock()->getElements()[1]);
    secondOpt->listenEvent<core::MouseLeftReleaseEvt>([&log, sp](const auto&)
    {
        log.info("clicked on second option");
        sp->removePaneIdx(1);
    });

    auto thirdMenu = utils::as<UIDropdown>(dd->getOptionsHolder().lock()->getElements()[2]);
    auto menuFirstOpt = utils::as<UIButton>(thirdMenu->getOptionsHolder().lock()->getElements()[0]);
    menuFirstOpt->listenEvent<core::MouseLeftReleaseEvt>([&log, sp3](const auto&)
    {
        log.info("clicked on first option from menu");
        auto ppx = sp3.lock()->createPane(2.0f, {30, 10'000});
    });

    auto menuSecondOpt = utils::as<UIButton>(thirdMenu->getOptionsHolder().lock()->getElements()[1]);
    menuSecondOpt->listenEvent<core::MouseLeftReleaseEvt>([&log, sp3](const auto&)
    {
        log.info("clicked on second option from menu");
        sp3.lock()->removePaneIdx(0);
    });

    window.lock()->add(sp);

    app.run();
    return 0;
}
