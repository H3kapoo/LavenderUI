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
#include "src/UIElements/UITreeView.hpp"
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
    // UIButtonPtr d = utils::make<UIButton>();
    // UIButtonPtr e = utils::make<UIButton>();
    // UITreeViewPtr tv = utils::make<UITreeView>();

    // frame.lock()->add(tv);
    // frame.lock()->tempPosOffset = {20, 20};
    // tv->setLayoutScale({600_px, 300_px});
    // // c->setLayoutScale({0.6_rel, 0.9_rel}).setLayoutType(LayoutAttribs::Type::VERTICAL);
    // // tv->setLayoutScale({0.8_rel, .8_rel});
    // tv->setColor(utils::hexToVec4("#b40a0aff"));

    // for (int32_t i = 0; i < 7; i++)
    // {
    //     UIButtonPtr x = utils::make<UIButton>();
    //     x->setLayoutScale({
    //         {(float)utils::randomInt(50, 200)},
    //         {(float)utils::randomInt(75, 200)}
    //     })
    //     // .setLayoutMargin({4, 4, 10, 4});
    //     .setLayoutMargin({0});
    //     ;
    //     x->setColor(utils::randomRGB());
    //     tv->add(x);
    // }

    std::println("{}", frame.lock());
    log.debug("id of pane {}", UIPane::typeId);
    log.debug("id of tv {}", UITreeView::typeId);
    log.debug("id of button {}", UIButton::typeId);

    /*
        UIBase
        UIBaseCRTP
        UIPane
        UITreeView

        UIBaseCRTP : UIBase
        UIPane : UIBaseCRTP<UIPane>

        UIPane : UIBase, UIBaseCRTP<UIPane>
    
    */

    /* Blocks */
    app.run();
    return 0;
}