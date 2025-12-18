
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

    auto p = sp->createPane(0.30f, {30, 10'000});
    auto sp3 = sp->createSubsplit(0.40f, {30, 10'000});
    auto p2 = sp->createPane(0.30f, {30, 10'000});

    sp3.lock()->getBaseLayoutData().setType(LayoutBase::Type::VERTICAL);
    auto pp = sp3.lock()->createPane(0.80f, {30, 10'000});
    auto pp2 = sp3.lock()->createPane(0.20f, {30, 10'000});

    window.lock()->add(sp);

    app.run();
    return 0;
}
