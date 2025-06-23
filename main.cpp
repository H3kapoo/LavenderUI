#include <print>
#include <type_traits>

#include "src/App.hpp"
#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/MeshLoader.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIButton.hpp"
#include "src/UIElements/UIFrame.hpp"
#include "src/UIElements/UIImage.hpp"
#include "src/Utils/Logger.hpp"

#include "src/Utils/Misc.hpp"
#include "vendor/freetype/include/freetype/freetype.h"

using namespace src::windowmanagement;
using namespace src::uielements;
using namespace src;

int main()
{
    utils::Logger log("Main");

    log.debug("version {}", __cplusplus);

    App& app = App::get();

    if (!app.init()) { return 1; }

    // uielements::UIFrameWPtr win = app.createFrame("myWindow", glm::ivec2{1280, 720});
    uielements::UIFrameWPtr frame = app.createFrame("myWindow", {1280, 720});
    // uielements::UIFrameWPtr frame2 = app.createFrame("myWindow 2", glm::ivec2{680, 720});

    uielements::UIBasePtr a = utils::make<uielements::UIButton>("a");
    uielements::UIButtonPtr b = utils::make<uielements::UIButton>("a");
    uielements::UIButtonPtr c = utils::make<uielements::UIButton>("c");
    uielements::UIButtonPtr d = utils::make<uielements::UIButton>("d");
    uielements::UIButtonPtr e = utils::make<uielements::UIButton>("e");
    uielements::UIButtonPtr f = utils::make<uielements::UIButton>("f");
    uielements::UIButtonPtr g = utils::make<uielements::UIButton>("g");
    uielements::UIImagePtr h = utils::make<uielements::UIImage>("g");

    uielements::UIBasePtrVec x = {a, b, c, d, e, f, g};

    a->add(b);
    b->add(c);
    c->add(d);
    d->add(e);
    e->add(f);
    e->add(g);
    a->add(x);
    /*
        -- a
            -- b
                -- c
            -- d
    
    */
    // a->remove(b);
    // a->remove(std::move(x));
    // a->remove(std::move(x));
    // a->remove(b);
    // std::cout << *a << "\n";
    // std::cout << a << "\n";
    std::println("{}", a);
    // std::println("{}", a->getTypeId());
    // std::println("{}", frame.lock()->getTypeId());
    // std::println("{}", b->getTypeId());
    // a->add(uielements::UIBasePtrVec{});
    // a->add(x);
    // a->add(std::move(x));
    // a->add({a, b});
    // FT_Library ftLib;
    // if (FT_Init_FreeType(&ftLib))
    // {
    //     std::cout << "testing not good\n";
    // }
    // FT_Done_FreeType(ftLib);
    // log.warn("hey there");

    // App::setVsync(false);
    // app.setWaitEvents(false);

    /* Blocks */
    app.run();
    return 0;
}