
#include "src/App.hpp"
#include "src/ElementComposable/IEvent.hpp"
#include "src/ElementComposable/LayoutBase.hpp"
#include "src/UINodes/UIButton.hpp"
#include "src/UINodes/UIBase.hpp"
#include "src/UINodes/UIWindow.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"
#include "src/WindowManagement/Input.hpp"

using namespace src::windowmanagement;
using namespace src::uinodes;
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

    for(uint i = 0; i < 2; ++i)
    {
        UIButtonPtr b = utils::make<UIButton>();
        b->getBaseLayoutData()
            .setScale({200_px, 40_px})
            .setBorderRadius({4})
            .setGridPos({i, i});
        b->setText("Ceva text");
        b->setColor(utils::hexToVec4("#2d69adff"));

        if (i == 0)
        {
            b->setDisabled();
        }
        window.lock()->add(b);

        b->getEventManager()
            .listenTo<MouseLeftClickEvt>([&log, id = b->getId()](const auto&){ log.debug("clicked BTN {}", id);})
            .listenTo<MouseLeftReleaseEvt>([&log, id = b->getId()](const auto&){ log.debug("released BTN {}", id);});
    }

    window.lock()->getEventManager()
        .listenTo<MouseLeftReleaseEvt>([&log, window](const auto&)
            {
                log.debug("released");
                auto btn = utils::as<UIButton>(window.lock()->getElements().at(0));
                btn->isEnabled() ? btn->setDisabled() : btn->setEnabled();
            });

    window.lock()->getBaseLayoutData()
        .setGrid({{1_fr, 1_fr, 1_fr, 1_fr, 1_fr}, {1_fr, 1_fr, 1_fr, 1_fr, 1_fr}})
        .setType(LayoutBase::Type:: GRID)
        .setAlign(LayoutBase::Align::CENTER)
        .setSpacing(LayoutBase::Spacing::EVEN_GAP);

    /* Blocks */
    app.run();
    return 0;
}