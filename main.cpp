#include <print>
#include <type_traits>

#include "src/App.hpp"
#include "src/UIElements/UIPane.hpp"
#include "src/UIElements/UISlider.hpp"
#include "src/ElementComposable/LayoutAttribs.hpp"
#include "src/ElementComposable/IEvent.hpp"
#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/MeshLoader.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIFrame.hpp"
#include "src/UIElements/UIImage.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"
#include "vendor/glfw/include/GLFW/glfw3.h"

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

    // uielements::UIFrameWPtr win = app.createFrame("myWindow", glm::ivec2{1280, 720});
    UIFrameWPtr frame = app.createFrame("myWindow", {1280, 720});
    // uielements::UIFrameWPtr frame2 = app.createFrame("myWindow 2", glm::ivec2{680, 720});

    UISliderPtr a = utils::make<UISlider>();
    UISliderPtr b = utils::make<UISlider>();
    UIPanePtr c = utils::make<UIPane>();
    UIButtonPtr d = utils::make<UIButton>();
    UIButtonPtr e = utils::make<UIButton>();
    // UIButtonPtr c = utils::make<UIButton>();

    frame.lock()->add(c);
    frame.lock()->tempPosOffset = {20, 20};

    // c->setLayoutScale({600_px, 300_px});
    c->setLayoutScale({0.6_rel, 0.9_rel}).setLayoutType(LayoutAttribs::Type::VERTICAL);
    c->setColor(utils::hexToVec4("#b40a0aff"));

    for (int32_t i = 0; i < 7; i++)
    {
        UIButtonPtr x = utils::make<UIButton>();
        x->setLayoutScale({
            {(float)utils::randomInt(50, 200)},
            {(float)utils::randomInt(75, 200)}
        })
        // .setLayoutMargin({4, 4, 10, 4});
        .setLayoutMargin({4, 4, 4, 1});
        ;
        x->setColor(utils::randomRGB());
        c->add(x);
    }

    c->listenTo<MouseButtonEvt>([&c, &log](const MouseButtonEvt& e)
    {
        if (e.btn == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE)
        {
            c->setLayoutType(LayoutAttribs::Type::VERTICAL);
        }
        else if (e.btn == GLFW_MOUSE_BUTTON_RIGHT && e.action == GLFW_RELEASE)
        {
            c->setLayoutType(LayoutAttribs::Type::HORIZONTAL);
        }
        else if (e.btn == GLFW_MOUSE_BUTTON_4 && e.action == GLFW_RELEASE)
        {
            c->setLayoutWrap(!c->getLayoutWrap());
            log.debug("button pressed");
        }
    });

    std::println("{}", frame.lock());

    /* Blocks */
    app.run();
    return 0;
}