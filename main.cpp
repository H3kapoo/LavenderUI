
#include "src/App.hpp"
#include "src/UIElements/UIPane.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIFrame.hpp"
#include "src/UIElements/UITreeView.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"

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

    UIPanePtr x = utils::make<UIPane>();
    UITreeViewPtr tv = utils::make<UITreeView>();

    // UIBasePtrVec x = {c};
    frame.lock()->add(tv);
    frame.lock()->getLayout().tempPosOffset = {20, 20};
    tv->getLayout().setScale({0.5_rel, 0.4_rel}).setMargin({30, 40, 20, 10});
    tv->getProps().setColor(utils::hexToVec4("#043da7ff"));
    tv->enableScroll(true, true).setScrollSensitivity(0.055f);

    // for (int32_t i = 0; i < 20; i++)
    // {
    //     UITreeView::ItemPtr item = utils::make<UITreeView::Item>(std::to_string(i), utils::randomRGB());
    //     tv->addItem(item);
    // }
    auto a = utils::make<UITreeView::Item>("a", utils::hexToVec4("#ff0000ff"));
    auto b = utils::make<UITreeView::Item>("b", utils::hexToVec4("#0024eeff"));
    auto c = utils::make<UITreeView::Item>("c", utils::hexToVec4("#dd0000ff"));
    auto d = utils::make<UITreeView::Item>("d", utils::hexToVec4("#cc0000ff"));
    auto e = utils::make<UITreeView::Item>("e", utils::hexToVec4("#aa0000ff"));
    auto f = utils::make<UITreeView::Item>("f", utils::hexToVec4("#006199ff"));
    auto g = utils::make<UITreeView::Item>("g", utils::hexToVec4("#880000ff"));
    auto h = utils::make<UITreeView::Item>("h", utils::hexToVec4("#770000ff"));
    auto i = utils::make<UITreeView::Item>("i", utils::hexToVec4("#660000ff"));
    auto j = utils::make<UITreeView::Item>("j", utils::hexToVec4("#550000ff"));
    auto k = utils::make<UITreeView::Item>("k", utils::hexToVec4("#440000ff"));
    auto l = utils::make<UITreeView::Item>("l", utils::hexToVec4("#330000ff"));
    auto m = utils::make<UITreeView::Item>("m", utils::hexToVec4("#220000ff"));
    auto n = utils::make<UITreeView::Item>("n", utils::hexToVec4("#110000ff"));
    auto o = utils::make<UITreeView::Item>("o", utils::hexToVec4("#000000ff"));

    a->subItems.emplace_back(c);
    a->subItems.emplace_back(d);
    a->subItems.emplace_back(e);
    a->subItems.emplace_back(g);

    b->subItems.emplace_back(f);

    g->subItems.emplace_back(h);
    h->subItems.emplace_back(i);
    i->subItems.emplace_back(j);
    j->subItems.emplace_back(k);
    k->subItems.emplace_back(l);
    l->subItems.emplace_back(m);
    m->subItems.emplace_back(n);
    n->subItems.emplace_back(o);

    tv->addItem(a);
    tv->addItem(b);

    // tv->refreshItems();
    // // for (int32_t i = 0; i < 2'000'000; i++)
    // for (int32_t i = 0; i < 2'000; i++)
    // {
    //     auto item = utils::make<UITreeView::Item>("item" + std::to_string(i), utils::randomRGB());
    //     b->subItems.emplace_back(item);
    // }
    tv->refreshItems();


    // app.setWaitEvents(false);

    /* Blocks */
    app.run();
    return 0;
}