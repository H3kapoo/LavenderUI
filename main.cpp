
#include "src/App.hpp"
#include "src/Core/Binders/WindowBinder.hpp"
#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LavParser/LavParser.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Core/ResourceHandler/Mesh.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Node/UIBase.hpp"
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
    // UIWindowWPtr window = app.loadView("views/test.xml");
    UIWindowWPtr window = app.createWindow("myWindow", {1280, 720});
    // UIPanePtr pane = utils::make<UIPane>();
    // pane->setColor(utils::hexToVec4("#af0fafff"));
    // pane->setScrollEnabled(true, true);
    // pane->getBaseLayoutData().setScale({300_px, 400_px});

    window.lock()->getBaseLayoutData()
        .setType(LayoutBase::Type::VERTICAL)
        .setAlign(LayoutBase::Align::CENTER);
    // window.lock()->add(pane);

    UISliderPtr slider = utils::make<UISlider>();
    slider->setScrollFrom(0);
    slider->setScrollTo(255);
    slider->getBaseLayoutData().setScale({500_px, 20_px}).setMargin(2);
    window.lock()->add(slider);

    // UISliderPtr slider2 = utils::make<UISlider>();
    // slider2->setScrollFrom(0);
    // slider2->setScrollTo(255);
    // slider2->getBaseLayoutData().setScale({500_px, 20_px}).setMargin(2);

    // UISliderPtr slider3 = utils::make<UISlider>();
    // slider3->setScrollFrom(0);
    // slider3->setScrollTo(255);
    // slider3->getBaseLayoutData().setScale({500_px, 20_px}).setMargin(2);

    // UIImagePtr img = utils::make<UIImage>();
    // img->setImage("assets/textures/awesomeface.png");
    // img->setColor(utils::hexToVec4("#51ce51ff"));
    // img->getBaseLayoutData().setScale({500_px, 500_px});

    // // pane->add(img);
    // slider->getEventManager().listenTo<core::SliderEvt>(
    //     [&img](const auto& e)
    //     {
    //         glm::vec4 color = img->getColor();
    //         color.r = ((int)e.value % 256 ) / 255.0f;
    //         img->setColor(color);
    //     });

    // slider2->getEventManager().listenTo<core::SliderEvt>(
    //     [&img](const auto& e)
    //     {
    //         glm::vec4 color = img->getColor();
    //         color.g = ((int)e.value % 256 ) / 255.0f;
    //         img->setColor(color);
    //     });

    // slider3->getEventManager().listenTo<core::SliderEvt>(
    //     [&img](const auto& e)
    //     {
    //         glm::vec4 color = img->getColor();
    //         color.b = ((int)e.value % 256 ) / 255.0f;
    //         img->setColor(color);
    //     });

    // window.lock()->add({slider, slider2, slider3});
    // window.lock()->add(img);
    // window.lock()->add(pane);

    app.run();
    return 0;
}