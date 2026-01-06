
#include "src/App.hpp"
#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Core/Binders/WindowBinder.hpp"
#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LavParser/LavParser.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Core/ResourceHandler/Mesh.hpp"
#include "src/Core/ViewModels/ListAbstractModel.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIDropdown.hpp"
#include "src/Node/UIImage.hpp"
#include "src/Node/UILabel.hpp"
#include "src/Node/UIRecycleList.hpp"
#include "src/Node/UISlider.hpp"
#include "src/Node/UIPane.hpp"
#include "src/Node/UISplitPane.hpp"
#include "src/Node/UIWindow.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"

#include <algorithm>
#include <ranges>
#include <thread>

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
    window.lock()->setColor(utils::hexToVec4("#38455eff"));
    window.lock()->getBaseLayoutData().setAlign(LayoutBase::Align::CENTER);

    std::vector<uint64_t> data =
        std::views::iota(0u, 100u) |
        std::ranges::to<std::vector<uint64_t>>();

    UIRecycleListPtr rl = utils::make<UIRecycleList>();
    rl->setScrollEnabled();
    rl->getBaseLayoutData().setScale({300_px, 0.9_rel});

    ListBasicModel model{data};
    ListOrderedModel orderedModel{model};
    ListFilteredModel filterModel{orderedModel,
        [](const uint64_t x) -> bool { return x % 2;}};

    // model->setProxyModel(model);
    // ListOrderedModel orderedModel{model};

    // rl->setModel(std::make_unique<ListBasicModel>(model));
    // rl->setModel(std::make_unique<ListOrderedModel>(orderedModel));
    rl->setModel(std::make_unique<ListFilteredModel>(filterModel));

    rl->listenEvent<core::ViewLMBRelease>([&log, &data](const auto& e)
    {
        log.error("clicked node id is {}", data[e.index.row]);
    });

    // auto pane = utils::as<UISplitPane>(window.lock()->getElements()[1])->getPaneIdx(0);
    // pane.lock()->add(rl);
    window.lock()->add(rl);

    // std::jthread t([&data, &model]()
    // {
    //     static uint64_t x = 6969;

    //     utils::Logger l{"wow"};
    //     while (true)
    //     {
    //         std::this_thread::sleep_for(std::chrono::seconds(1));
    //         data.push_back(x);
    //         ++x;
    //         model->filterAll();
    //         WindowBinder::get().requestEmptyEvent();
    //         l.error("adding {}", x);
    //     }
    // });

    app.run();
    // t.join();
    return 0;
}
