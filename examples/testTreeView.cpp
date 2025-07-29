
#include "src/App.hpp"
#include "src/ElementComposable/IEvent.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIPane.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIWindow.hpp"
#include "src/UIElements/UITreeView.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"
#include <memory>

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
    app.enableTitleWithFPS();

    // uielements::UIFrameWPtr win = app.createFrame("myWindow", glm::ivec2{1280, 720});
    UIWindowWPtr window = app.createWindow("myWindow", {1280, 720});
    // uielements::UIFrameWPtr frame2 = app.createFrame("myWindow 2", glm::ivec2{680, 720});

    // UIPanePtr p = utils::make<UIPane>();
    UITreeViewPtr p = utils::make<UITreeView>();
    // UITreeViewPtr p = utils::make<UITreeView>();

    window.lock()->add(p);
    // window.lock()->tempPosOffset = {30, 40};

    // p->setScale({0.5_rel, 400_px});
    p->setScrollEnabled(true, true);
    p->setScrollSensitivityMultiplier(0.01);
    p->setScale({300_px, 1.0_rel});
    p->setColor(utils::hexToVec4("#69c553ff"));
    // app.setWaitEvents(false);

    glm::vec4 alt{utils::hexToVec4("#dfdfdfff")};
    glm::vec4 alt2{utils::hexToVec4("#a8a8a8ff")};
    
    int32_t col{0};
    UITreeView::ItemPtr x = utils::make<UITreeView::Item>();
    x->color = col++ % 2 ? alt : alt2;
    x->text = "root element";

    for (int32_t i = 0; i < 20; ++i)
    {
        UITreeView::ItemPtr y = utils::make<UITreeView::Item>();
        y->color = col++ % 2 ? alt : alt2;
        y->text = std::to_string(i) + " element";
        x->addItem(y);

        for (int32_t j = 0; j < 5; ++j)
        {
            UITreeView::ItemPtr z = utils::make<UITreeView::Item>();
            z->color = col++ % 2 == 1 ? alt : alt2;
            z->text = std::to_string(i) + std::to_string(j) + " element";

            y->addItem(z);
        }

        // UIButtonPtr b = utils::make<UIButton>();
        // b->setMargin({2, 2, 2, 2});

        // b->listenTo<MouseEnterEvt>(
        //     [rb = std::weak_ptr<UIButton>(b)](const auto&)
        //     {
        //         rb.lock()->setColor(utils::hexToVec4("#a3a3a3ff"));
        //     });

        // b->listenTo<MouseExitEvt>(
        //     [rb = std::weak_ptr<UIButton>(b)](const auto&)
        //     {
        //         rb.lock()->setColor(utils::hexToVec4("#ffffffff"));
        //     });
        // p->add(std::move(b));
    }

    p->addItem(std::move(x));
    p->refreshItems();

    /* Blocks */
    app.run();
    return 0;
}