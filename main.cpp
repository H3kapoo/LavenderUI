
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
    // UIWindowWPtr window = app.loadLavView("views/test.xml");
    UIWindowWPtr window = app.createWindow("myWindow", {1280, 720});
    window.lock()->getBaseLayoutData()
        // .setType(LayoutBase::Type::VERTICAL)
        .setAlign(LayoutBase::Align::CENTER)
        ;
    UIImagePtr img = utils::make<UIImage>();
    img->setImage("assets/textures/wall.jpg");
    img->getBaseLayoutData().setScale({200_px, 200_px});

    UILabelPtr lbl = utils::make<UILabel>();
    lbl->setText("my text");
    lbl->getBaseLayoutData().setScale({200_px, 200_px}).setBorderRadius({10});

    UIDropdownPtr dd = utils::make<UIDropdown>();
    dd->getBaseLayoutData().setBorder({4});
    dd->setText("My Dropdown").setPreferredOpenDir(UIDropdown::OpenDir::TOP);

    // dd->getOptionsHolder().lock()->getBaseLayoutData().setBorderRadius({4});
    dd->getOptionsHolder().lock()->getBaseLayoutData().setBorder({4}).setBorderRadius({10});

    auto btn1 = dd->addOption("NewOption");
    auto btn2 = dd->addOption("NewOption2");
    auto newMenu = dd->addSubMenu("NewMenu");
    newMenu.lock()->setPreferredOpenDir(UIDropdown::OpenDir::RIGHT);
    newMenu.lock()->addOption("SubOpt");
    newMenu.lock()->addOption("SubOpt2");

    // btn1.lock()->setDisabled();
    btn1.lock()->listenEvent<lav::core::MouseLeftReleaseEvt>([&log](const auto&)
    {
        log.error("bla bla from btn");
    });
    window.lock()->add({dd, img, lbl});

    app.run();
    return 0;
}