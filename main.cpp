#include <print>
#include <type_traits>

#include "src/App.hpp"
#include "src/ResourceLoaders/Mesh.hpp"
#include "src/ResourceLoaders/MeshLoader.hpp"
#include "src/ResourceLoaders/Shader.hpp"
#include "src/UIElements/UIBase.hpp"
#include "src/UIElements/UIFrame.hpp"
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

    uielements::UIBasePtr a = utils::make<uielements::UIBase>("a");
    uielements::UIBasePtr b = utils::make<uielements::UIBase>("b");
    uielements::UIBasePtr c = utils::make<uielements::UIBase>("c");
    uielements::UIBasePtr d = utils::make<uielements::UIBase>("d");
    uielements::UIBasePtr e = utils::make<uielements::UIBase>("e");
    uielements::UIBasePtr f = utils::make<uielements::UIBase>("f");
    // a->add(std::move(b));

    uielements::UIBasePtrVec x = {a, b, c, d, e, f};

    // a->add(b);
    // b->add(c);
    // a->add(d);
    a->add(x);
    /*
        -- a
            -- b
                -- c
            -- d
    
    */
    a->remove(b);
    // a->remove(std::move(x));
    // a->remove(std::move(x));
    // a->remove(b);
    // std::cout << *a << "\n";
    // std::cout << a << "\n";
    std::println("{}", a);
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