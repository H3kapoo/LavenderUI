#include "LavenderUI/App.hpp"
#include "LavenderUI/Core/Binders/WindowBinder.hpp"
#include "LavenderUI/Core/EventHandler/CoreEvents/FocusGain.hpp"
#include "LavenderUI/Core/EventHandler/CoreEvents/FocusLost.hpp"
#include "LavenderUI/Core/EventHandler/CoreEvents/MouseLeftRelease.hpp"
#include "LavenderUI/Core/EventHandler/CoreEvents/TextChanged.hpp"
#include "LavenderUI/Core/LayoutHandler/LayoutBase.hpp"
#include "LavenderUI/Core/TextHandler/TextOptions.hpp"
#include "LavenderUI/Core/ViewModels/ListModels.hpp"
#include "LavenderUI/Core/ViewModels/TreeModels.hpp"
#include "LavenderUI/Node/UIDropdown.hpp"
#include "LavenderUI/Node/UILineEdit.hpp"
#include "LavenderUI/Node/UIListView.hpp"
#include "LavenderUI/Node/UIPane.hpp"
#include "LavenderUI/Node/UISplitPane.hpp"
#include "LavenderUI/Node/UITreeView.hpp"
#include "LavenderUI/Node/UIWindow.hpp"
#include "LavenderUI/Utils/Logger.hpp"
#include "LavenderUI/Utils/Misc.hpp"
#include "LavenderUI/Core/Config.hpp"

#include <exception>
#include <ranges>

using namespace lav::core;
using namespace lav::node;
using namespace lav;

SimpleTreeItemS* createData(const std::string& inData, SimpleTreeItemS* inParent)
{
    SimpleTreeItemS* data = new SimpleTreeItemS;
    data->data = inData;
    data->parent = inParent;
    return data;
}

SimpleTreeItemS* createTree()
{
    SimpleTreeItemS* root = createData("Root", nullptr);
    SimpleTreeItemS* root_A = createData("Root_A", root);
    SimpleTreeItemS* root_B = createData("Root_B", root);
    SimpleTreeItemS* root_C = createData("Root_C", root);
    // root->children = {root_A, root_B, root_C};
    root->children = {root_A, root_C, root_B};

    SimpleTreeItemS* A_0 = createData("A_0", root_A);
    SimpleTreeItemS* A_1 = createData("A_1", root_A);
    root_A->children = {A_0, A_1};

    for (int32_t i = 2; i < 30; i++)
    {
        SimpleTreeItemS* A_x = createData("A_" + std::to_string(i), root_A);
        root_A->children.push_back(A_x);
    }

    for (int32_t i = 0; i < 200; i++)
    {
        SimpleTreeItemS* B_x = createData("B_" + std::to_string(i), root_A);
        
        if (i % 3 == 0)
        {
            SimpleTreeItemS* B_x_child_x = createData("B_Child_" + std::to_string(i), B_x);
            B_x->children.push_back(B_x_child_x);
        }
        root_B->children.push_back(B_x);
    }

    SimpleTreeItemS* C_0 = createData("C_0", root_C);
    SimpleTreeItemS* C_1 = createData("C_1", root_C);
    SimpleTreeItemS* C_2 = createData("C_2", root_C);
    SimpleTreeItemS* C_3 = createData("C_3", root_C);
    SimpleTreeItemS* C_4 = createData("C_4", root_C);
    SimpleTreeItemS* C_5 = createData("C_5", root_C);
    root_C->children = {C_0, C_1, C_2, C_3, C_4, C_5};

    SimpleTreeItemS* C2_Child_0 = createData("C2_Child_0", C_2);
    SimpleTreeItemS* C2_Child_1 = createData("C2_Child_1", C_2);
    SimpleTreeItemS* C2_Child_2 = createData("C2_Child_2", C_2);
    C_2->children = {C2_Child_0, C2_Child_1, C2_Child_2};

    /*
        - Root
            - Root_A
                - A_0  <- (0, Root_A)
                - A_1
            - Root_B
            - Root_C
                - C_0
                - C_1
                - C_2
                    - C2_Child_0
                    - C2_Child_1
                    - C2_Child_2
                - C_3
                - C_4
                - C_5

        getRowCount(ModelIndex{invalid_r, invalid_p}) => rows of root => 3
        index(ModelIndex{0, invalid_p})
            => shall return ModelIndex of first row of parent
            => ModelIndex{0, root_parent}
    */
    return root;
}

int main()
{
    utils::Logger log("Main");

    log.debug("version {}", __cplusplus);

    App& app = App::get();
    if (!app.init()) { return 1; }
    app.enableTitleWithFPS();
    UIWindowWPtr window = app.loadLavView(core::Config::testViewsPath / "test.xml");
    window.lock()->setColor(utils::hexToVec4("#38455eff"));

    // UIWindowWPtr window2 = app.createWindow("myWindow", {860, 480});
    // window2.lock()->setColor(utils::hexToVec4("#38455eff"));
    // window2.lock()->getBaseLayoutData().setAlign(LayoutBase::Align::CENTER);

    // UILineEditPtr le3 = utils::make<UILineEdit>();
    // le3->setText("13456789");

    // window2.lock()->add(le3);
    // app.run();
    // return 0;

    auto label = window.lock()->findElementByViewId<UILabel>("mylabel").lock();
    // label->getBaseLayoutData().setPadding({4});
    // label->setText("Foarte ciudat this is a certified yapper");
    label->setTextWrap(true);
    label->setTextEllipsis();
    label->setTextAlign(core::TextOptions::Align::CENTER);
    label->setTextColor(utils::hexToVec4("#f03434ff"));

    app.run();
    return 0;

    auto dd = window.lock()->findElementByViewId<UIDropdown>("my_dd").lock();
    auto option = dd->addOption("Hey").lock();
    option->setColor(utils::hexToVec4("#483ed3ff"));
    option->listenEvent<core::MouseLeftReleaseEvt>([&log](const auto& e)
    {
        (void)e;
        log.info("clicked me");
    });

    std::vector<uint32_t> data =
        std::views::iota(0u, 200u) |
        // std::views::iota(0u, 200'000u) |
        // std::views::iota(0u, 1'500'000u) |
        std::ranges::to<std::vector<uint32_t>>();

    // // tv->getBaseLayoutData().setScale({300_px, 0.9_rel});

    ListBasicModel model{data};
    ListFilteredModel filterModel{model,
        [](const uint64_t x) -> bool { return x % 2; }};

    // UIListPtr rl = window.lock().findElement<UIList>(recursive = true/false, lambda);

    UIListViewPtr rl = window.lock()->findElementByViewId<UIListView>("mylist").lock();

    // rl->setModel(std::make_unique<ListFilteredModel>(filterModel));
    rl->setAlternatingRowEnabled();
    rl->setScrollSensitivity(15);
    rl->setRowSize(18);

    rl->listenEvent<core::ViewLMBRelease>([&log, &data, &rl](const auto& e)
    {
        log.error("clicked node id is {}", data[e.index.row]);
    });

    std::vector<UIPaneWPtr> panes = window.lock()->findElements<UIPane>(
        [](const UIBasePtr e) -> bool
        {
            // here we can safely assume "e" it's always of type T
            return true;
        }, true);

    SimpleTreeItemS* root = createTree();
    TreeBasicModel<std::string> treeModel{root};
    // // // ListOrderedModel orderedModel{model};

    // tv->setModel(std::make_unique<TreeBasicModel<std::string>>(treeModel));
    // tv->setAlternatingRowEnabled();
    // tv->setScrollSensitivity(15);
    // tv->setRowSize(20);

    // rl->setModel(std::make_unique<ListFilteredModel>(filterModel));
    // rl->setAlternatingRowEnabled();

    // // tv->setModel(std::make_unique<TreeBasicModel<std::string>>(model));
    // // // tv->setModel(std::make_unique<ListFilteredModel>(filterModel));

    // // window.lock()->add(tv);

    // auto pane1 = utils::as<UISplitPane>(window.lock()->getElements()[1])->getPaneIdx(0);
    // rl->getBaseLayoutData().setScale({1_fill});
    // pane1.lock()->getBaseLayoutData().setType(LayoutBase::Type::VERTICAL);

    UITreeViewPtr tv = window.lock()->findElementByViewId<UITreeView>("mytree").lock();
    tv->getBaseLayoutData().setScale({1_fill});
    // tv->setModel(std::make_unique<TreeBasicModel<std::string>>(treeModel));
    tv->setAlternatingRowEnabled();
    tv->setScrollSensitivity(15);
    tv->setRowSize(18);

    // auto label = window.lock()->findElementByViewId<UILabel>("mylabel").lock();
    // label->getBaseLayoutData().setPadding({4});
    // label->setText("Foarte ciudat"\
    //     "this is a certified yapper");
    // label->setTextWrap(true);
    // label->setTextEllipsis();
    // label->setTextAlign(core::TextOptions::Align::CENTER);
    // label->setTextColor(utils::hexToVec4("#f03434ff"));

    // auto pane3 = utils::as<UISplitPane>(window.lock()->getElements()[1]->getElements()[4]);
    // auto pane32 = pane3->getPaneIdx(1);
    // auto label = utils::as<UILabel>(pane32.lock()->getElements()[0]);

    // UILineEditPtr le = utils::make<UILineEdit>();
    // le->setText("13456789");
    // le->enableNumbericOnly(true);
    // le->setColor(utils::hexToVec4("#62f562ff"));
    // le->getBaseLayoutData().setScale({1_fill, 30_px});



    UILineEditPtr le = window.lock()->findElementByViewId<UILineEdit>("list_le").lock();
    le->listenEvent<core::TextChangedEvt>([&log, &filterModel, &rl](const auto& e)
    {
        // log.error("text changed {}", e.text);
        try
        {
            int32_t num = std::stoi(e.text);
            filterModel.rebuild([num](const uint64_t x) -> bool { return x % num == 0; });

            // rl->setModel(std::make_unique<ListFilteredModel>(filterModel));
            WindowBinder::get().requestEmptyEvent();
        } catch(std::exception& ex)
        {
            (void)ex;
        }
    });

    UILineEditPtr le2 = utils::make<UILineEdit>();
    le2->setText("13456789");
    le2->setColor(utils::hexToVec4("#daf562ff"));
    le2->getBaseLayoutData().setScale({1_fill, 30_px});

    auto pp = le->getParent().lock();
    utils::as<UIPane>(pp)->add(le2, 1);

    // pane1.lock()->add(le);
    // pane1.lock()->add(le2);
    // pane1.lock()->add(rl);

    // pane32.lock()->listenEvent<core::FocusGainEvt>([&log](const auto&)
    // {
    //     log.error("focus gain!");
    // });
    // pane32.lock()->listenEvent<core::FocusLostEvt>([&log](const auto&)
    // {
    //     log.error("focus lost!");
    // });
    tv->listenEvent<core::ViewLMBRelease>([&log, &label](const auto& e)
    {
        SimpleTreeItem<std::string>* data = static_cast<SimpleTreeItem<std::string>*>
            (e.index.internalPtr);
        if (!data)
        {
            // log.error("clicked node id is {}", e.index.data);
            return;
        }
        // log.error("clicked node id is {}", data->data);
        label->setText(data->data);
    });

    rl->listenEvent<core::ViewLMBRelease>([&log, &data, &window](const auto& e)
    {
        if (e.index.row % 2 == 0)
        {
            window.lock()->setFullScreen(true);
        }
        else
        {
            window.lock()->setFullScreen(false);
        }
        log.error("clicked node id is {}", data[e.index.row]);
    });

    // std::jthread t1([&data, &log]()
    // {
    //     static uint32_t ctr = 0;
    //     while (ctr < 100)
    //     {
    //         std::this_thread::sleep_for(std::chrono::milliseconds(150));
    //         uint32_t value = ctr;
    //         ctr++;
    //         data.push_back(value);
    //         log.debug("Adding new value {}", value);
    //         WindowBinder::get().requestEmptyEvent();
    //     }
    // });
    app.run();
    return 0;
}
