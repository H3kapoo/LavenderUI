#include <print>
#include <type_traits>

#include "src/App.hpp"
#include "src/UIElements/UISlider.hpp"
#include "src/ElementComposable/LayoutAttribs.hpp"
#include "src/ElementEvents/IEvent.hpp"
#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/MeshLoader.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIFrame.hpp"
#include "src/UIElements/UIImage.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"

using namespace src::windowmanagement;
using namespace src::uielements;
using namespace src::elementevents;
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
    UIButtonPtr c = utils::make<UIButton>();

    frame.lock()->add(a);
    frame.lock()->getLayout().tempPosOffset = {20, 20};

    a->getTextAttribs().setText("Textaaaaaaa");
    a->getLayout().scale = {300_px, 50_px};
    a->getLayout().type = LayoutAttribs::Type::HORIZONTAL;

    a->getEvents().listen<SliderEvt>([&log, &a](const SliderEvt& e)
    {
        a->getTextAttribs().setText(std::to_string(e.value).substr(0, 5));
    });
    // UIButtonPtr b = utils::make<UIButton>();
    // UIButtonPtr c = utils::make<UIButton>();
    // UIButtonPtr d = utils::make<UIButton>();

    // // uielements::UIBasePtrVec x = {a, b, c, d, e, f, g};

    // frame.lock()->add({a});
    // frame.lock()->getLayout().tempPosOffset = {20, 20};

    // a->getLayout().pos = {150, 100};
    // // a->getLayout().computedScale = {300, 300};
    // a->getVisual().color = {0, 0, 0, 1};

    // // a->getLayout().scale = {300_px, 300_px};
    // a->getLayout().scale = {0.5_rel, 300_px};

    // // b->getLayout().pos = {390, 140};
    // b->getVisual().color = utils::randomRGB();
    // c->getVisual().color = utils::randomRGB();
    // d->getVisual().color = utils::randomRGB();

    std::println("{}", frame.lock());

    /* Blocks */
    app.run();
    return 0;
}