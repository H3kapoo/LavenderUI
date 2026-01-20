#include "include/LavenderUI/App.hpp"
#include "include/LavenderUI/Core/ViewModels/ListModels.hpp"
#include "include/LavenderUI/Core/ViewModels/TreeModels.hpp"
#include "include/LavenderUI/Node/UIList.hpp"
#include "include/LavenderUI/Node/UISplitPane.hpp"
#include "include/LavenderUI/Node/UITreeView.hpp"
#include "include/LavenderUI/Node/UIWindow.hpp"
#include "include/LavenderUI/Utils/Logger.hpp"
#include "include/LavenderUI/Utils/Misc.hpp"

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
    UIWindowWPtr window = app.loadLavView("temporaryPlayground/views/test.xml");

    // UIWindowWPtr window = app.createWindow("myWindow", {1280, 720});
    // window.lock()->setColor(utils::hexToVec4("#38455eff"));
    // window.lock()->getBaseLayoutData().setAlign(LayoutBase::Align::CENTER);

    std::vector<uint32_t> data =
        std::views::iota(0u, 100u) |
        std::ranges::to<std::vector<uint32_t>>();

    UITreeViewPtr tv = utils::make<UITreeView>();
    tv->setScrollEnabled();
    // tv->getBaseLayoutData().setScale({300_px, 0.9_rel});

    UIRecycleListPtr rl = utils::make<UIList>();
    rl->setScrollEnabled();
    // rl->getBaseLayoutData().setScale({300_px, 0.9_rel});

    ListBasicModel model{data};
    SimpleTreeItemS* root = createTree();
    TreeBasicModel<std::string> treeModel{root};
    // // ListOrderedModel orderedModel{model};
    ListFilteredModel filterModel{model,
        [](const uint64_t x) -> bool { return x % 2;}};

    tv->setModel(std::make_unique<TreeBasicModel<std::string>>(treeModel));
    tv->setAlternatingRowEnabled();
    tv->setScrollSensitivity(15);
    tv->setRowSize(20);

    // rl->setModel(std::make_unique<ListBasicModel>(model));
    rl->setModel(std::make_unique<ListFilteredModel>(filterModel));
    rl->setAlternatingRowEnabled();
    rl->setScrollSensitivity(15);
    rl->setRowSize(18);
    // tv->setModel(std::make_unique<TreeBasicModel<std::string>>(model));
    // // tv->setModel(std::make_unique<ListFilteredModel>(filterModel));

    // window.lock()->add(tv);

    auto pane1 = utils::as<UISplitPane>(window.lock()->getElements()[1])->getPaneIdx(0);
    rl->getBaseLayoutData().setScale({1_fill});
    pane1.lock()->add(rl);

    auto pane2 = utils::as<UISplitPane>(window.lock()->getElements()[1])->getPaneIdx(1);
    tv->getBaseLayoutData().setScale({1_fill});
    pane2.lock()->remove([](auto&){ return true;});
    pane2.lock()->add(tv);

    auto pane3 = utils::as<UISplitPane>(window.lock()->getElements()[1]->getElements()[4]);
    auto pane32 = pane3->getPaneIdx(1);
    auto label = utils::as<UILabel>(pane32.lock()->getElements()[0]);
    label->setText("Schimbat");

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

    rl->listenEvent<core::ViewLMBRelease>([&log, &data, &rl](const auto& e)
    {
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
