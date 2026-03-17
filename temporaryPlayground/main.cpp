#include "LavenderUI/App.hpp"
#include "LavenderUI/Core/Binders/WindowBinder.hpp"
#include "LavenderUI/Core/EventHandler/CoreEvents/FocusGain.hpp"
#include "LavenderUI/Core/EventHandler/CoreEvents/FocusLost.hpp"
#include "LavenderUI/Core/EventHandler/CoreEvents/MouseLeftRelease.hpp"
#include "LavenderUI/Core/EventHandler/CoreEvents/Slide.hpp"
#include "LavenderUI/Core/EventHandler/CoreEvents/TextChanged.hpp"
#include "LavenderUI/Core/LayoutHandler/LayoutBase.hpp"
#include "LavenderUI/Core/ViewModels/ListModels.hpp"
#include "LavenderUI/Core/ViewModels/TreeModels.hpp"
#include "LavenderUI/Node/UIDropdown.hpp"
#include "LavenderUI/Node/UILineEdit.hpp"
#include "LavenderUI/Node/UIListView.hpp"
#include "LavenderUI/Node/UIPane.hpp"
#include "LavenderUI/Node/UISlider.hpp"
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

    // auto label = window.lock()->findElementByViewId<UILineEdit>("mylabel").lock();
    auto label = window.lock()->findElementByViewId<UILabel>("mylabel").lock();
    label->setTextWrap(true);
    // label->getBaseLayoutData().setPadding({4});
    // label->getBaseLayoutData().setBorder({4, 4, 4, 4});
    // label->setBorderColor(utils::hexToVec4("#c91c1cff"));
    // label->setText("Foarte ciudat this is a certified yapper");
    // label->setTextEllipsis();
    label->setTextAlign(core::TextOptions::Align::CENTER);
    // label->setFontSize(25);
    label->setFontSize(27);
    // label->setFontSize(34);
    // label->setScaleToTextSize();
    // label->setTextColor(utils::hexToVec4("#f03434ff"));

    // auto label2 = window.lock()->findElementByViewId<UILabel>("mylabel2").lock();
    // label2->getBaseLayoutData().setPadding({4});
    // // label->setText("Foarte ciudat this is a certified yapper");
    // label2->setTextWrap(true);
    // label2->setTextEllipsis();
    // label2->setTextAlign(core::TextOptions::Align::CENTER);
    // label2->setTextColor(utils::hexToVec4("#f03434ff"));

    // auto slider = window.lock()->findElementByViewId<UISlider>("slid").lock();
    // slider->listenEvent<core::SlideEvt>([&log, &label](const auto& e)
    // {
    //     uint32_t fs = static_cast<uint32_t>(e.value);
    //     // log.info("value {}", fs);
    //     label->setFontSize(fs);
    // });
    app.run();
    return 0;
}
