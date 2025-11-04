
#include "src/App.hpp"
#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIImage.hpp"
#include "src/Node/UILabel.hpp"
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

    UIWindowWPtr window = app.createWindow("myWindow", {1280, 720});

    UIImagePtr img = utils::make<UIImage>();
    img->setImage("/home/hekapoo/Documents/probe/move_stuff/assets/textures/awesomeface.png");
    img->getBaseLayoutData().setScale({1_fill});

    UILabelPtr lab = utils::make<UILabel>();
    lab->setText("Some text");

    lab->getBaseLayoutData().setScale({300_px, 200_px});

    window.lock()->add({img, lab});
    // window.lock()->add(img);
    // window.lock()->add(lab);

    /* Blocks */
    app.run();
    return 0;
}