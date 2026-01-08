
#include "src/App.hpp"
#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Core/Binders/WindowBinder.hpp"
#include "src/Core/EventHandler/IEvent.hpp"
#include "src/Core/LavParser/LavParser.hpp"
#include "src/Core/LayoutHandler/LayoutBase.hpp"
#include "src/Core/ResourceHandler/Mesh.hpp"
#include "src/Core/ViewModels/TreeModels.hpp"
#include "src/Node/UIButton.hpp"
#include "src/Node/UIBase.hpp"
#include "src/Node/UIDropdown.hpp"
#include "src/Node/UIImage.hpp"
#include "src/Node/UILabel.hpp"
#include "src/Node/UIRecycleList.hpp"
#include "src/Node/UISlider.hpp"
#include "src/Node/UIPane.hpp"
#include "src/Node/UISplitPane.hpp"
#include "src/Node/UITreeView.hpp"
#include "src/Node/UIWindow.hpp"
#include "src/Utils/Logger.hpp"
#include "src/Utils/Misc.hpp"

#include <algorithm>
#include <ranges>
#include <thread>

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
    root->children = {root_A, root_B, root_C};
    // root->children = {root_C};

    SimpleTreeItemS* A_0 = createData("A_0", root_A);
    SimpleTreeItemS* A_1 = createData("A_1", root_A);
    root_A->children = {A_0, A_1};

    SimpleTreeItemS* C_0 = createData("C_0", root_C);
    SimpleTreeItemS* C_1 = createData("C_1", root_C);
    SimpleTreeItemS* C_2 = createData("C_2", root_C);
    SimpleTreeItemS* C_3 = createData("C_3", root_C);
    SimpleTreeItemS* C_4 = createData("C_4", root_C);
    SimpleTreeItemS* C_5 = createData("C_5", root_C);
    root_C->children = {C_0, C_1, C_2, C_3, C_4, C_5};
    // root_C->children = {C_0, C_1, C_2, C_3};
    // root_C->children = {C_0, C_1};

    // SimpleTreeItemS* C2_Child_0 = createData("C2_Child_0", C_2);
    // SimpleTreeItemS* C2_Child_1 = createData("C2_Child_1", C_2);
    // SimpleTreeItemS* C2_Child_2 = createData("C2_Child_2", C_2);
    // C_2->children = {C2_Child_0, C2_Child_1, C2_Child_2};

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
    // UIWindowWPtr window = app.loadLavView("views/test.xml");

    UIWindowWPtr window = app.createWindow("myWindow", {1280, 720});
    window.lock()->setColor(utils::hexToVec4("#38455eff"));
    window.lock()->getBaseLayoutData().setAlign(LayoutBase::Align::CENTER);

    // std::vector<uint64_t> data =
    //     std::views::iota(0u, 100u) |
    //     std::ranges::to<std::vector<uint64_t>>();

    UITreeViewPtr tv = utils::make<UITreeView>();
    tv->setScrollEnabled();
    tv->getBaseLayoutData().setScale({300_px, 0.9_rel});

    SimpleTreeItemS* root = createTree();
    TreeBasicModel<std::string> model{root};
    // ListOrderedModel orderedModel{model};
    // ListFilteredModel filterModel{orderedModel,
    //     [](const uint64_t x) -> bool { return x % 2;}};

    tv->setModel(std::make_unique<TreeBasicModel<std::string>>(model));
    // tv->setModel(std::make_unique<ListFilteredModel>(filterModel));

    tv->listenEvent<core::ViewLMBRelease>([&log](const auto& e)
    {
        SimpleTreeItem<std::string>* data = static_cast<SimpleTreeItem<std::string>*>
            (e.index.internalPtr);
        if (!data)
        {
            // log.error("clicked node id is {}", e.index.data);
            return;
        }
        log.error("clicked node id is {}", data->children[e.index.row]->data);
    });

    window.lock()->add(tv);

    app.run();
    return 0;
}
