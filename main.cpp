
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
#include "src/Node/UIRecycleList.hpp"
#include "src/Node/UISlider.hpp"
#include "src/Node/UIPane.hpp"
#include "src/Node/UISplitPane.hpp"
#include "src/Node/UIWindow.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"

#include <algorithm>
#include <ranges>

using namespace lav::core;
using namespace lav::node;
using namespace lav;

struct DerivedModel : UIRecycleList::Model
{
    DerivedModel(const std::vector<uint64_t>& data)
        : data_(data)
    {}
    const std::vector<uint64_t>& data_;

    auto makeAtIndex(UIButtonPtr& btn, const uint64_t index) -> void
    {
        if (index >= data_.size()) { return; }

        btn->setText(std::to_string(data_[index]));
        btn->setColor(index % 2
            ? utils::hexToVec4("#adadadff")
            : utils::hexToVec4("#e46b6bff"));
        btn->listenEvent<core::MouseLeftReleaseEvt>(
            [this, index](const auto& e)
            {
                (void)e;
                utils::Logger log{"InsideBtn"};
                log.debug("clicked on {} data {}", index, data_[index]);
            });
    }

    auto getItemsCount() -> uint64_t { return data_.size(); }
};

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
        // std::views::iota(0u, 10'200'001u) |
        std::views::iota(0u, 10'000u) |
        // std::views::filter([](uint32_t x){ return true; }) |
        std::ranges::to<std::vector<uint64_t>>();
    
    // std::ranges::for_each(data, [&log](uint32_t x){ log.info("{}", x); });

    UIRecycleListPtr rl = utils::make<UIRecycleList>();
    rl->setScrollEnabled();
    rl->getBaseLayoutData().setScale({300_px, 0.9_rel});
    // rl->getBaseLayoutData().setScale({300_px, 0.9_rel});

    rl->setModel(std::make_unique<DerivedModel>(data));

    // auto pane = utils::as<UISplitPane>(window.lock()->getElements()[1])->getPaneIdx(0);
    // pane.lock()->add(rl);
    window.lock()->add(rl);

    app.run();
    return 0;
}
