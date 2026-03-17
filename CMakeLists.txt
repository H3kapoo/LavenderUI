cmake_minimum_required(VERSION 3.22)

# NOTE
# GLEW, GLFW and FreeType2 shall be used as shared libs both for usage and dev
# STBI_IMAGE and GLM are header only and need to be reachable only for dev

# NOTE 2
# XML Parser is also needed but this is a "lib" not currently integrated, it is "shipped" as is from git
#   only for dev

project("lavenderui")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(PLATFORM_NAME ${CMAKE_SYSTEM_NAME})

# C++ Flags
# set(VERBOSE_FLAG "--verbose")
set(COMPILE_FEATURES "cxx_std_23")
set(LIB_DEBUG_INFO_FLAGS "-ggdb -g")
set(LIB_EXTRA_WARNINGS "-Wall -Wextra -Wshadow")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VERBOSE_FLAG} ${LIB_EXTRA_WARNINGS} ${LIB_DEBUG_INFO_FLAGS}")

# Output library path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Dependencies include paths
set(GLEW_INCLUDE_PATH     "thirdparty/glew/include/")
set(GLFW_INCLUDE_PATH     "thirdparty/glfw/include/")
set(FREETYPE_INCLUDE_PATH "thirdparty/freetype/include/")

# Dependencies lib paths
set(GLEW_LIB_PATH     "thirdparty/glew/lib/")
set(GLFW_LIB_PATH     "thirdparty/glfw/lib/")
set(FREETYPE_LIB_PATH "thirdparty/freetype/lib/")

# Sources
function(addSources)
    add_executable(${PROJECT_NAME}
        temporaryPlayground/main.cpp
        src/App.cpp

        # Binders
        src/Core/Binders/WindowBinder.cpp
        src/Core/Binders/GPUBinder.cpp
        src/Core/Binders/FileResourceBinder.cpp

        # Parser
        src/ViewParser/LavParser.cpp
        src/ViewParser/ParseHelpers.cpp
        src/ViewParser/Rules/AppRule.cpp
        src/ViewParser/Rules/PaneRule.cpp
        src/ViewParser/Rules/ImageRule.cpp
        src/ViewParser/Rules/ButtonRule.cpp
        src/ViewParser/Rules/LabelRule.cpp
        src/ViewParser/Rules/SliderRule.cpp
        src/ViewParser/Rules/DropdownRule.cpp
        src/ViewParser/Rules/SplitPaneRule.cpp
        src/ViewParser/Rules/ListViewRule.cpp
        src/ViewParser/Rules/TreeViewRule.cpp
        src/ViewParser/Rules/LineEditRule.cpp

        # Resource Handlers
        src/Core/ResourceHandler/Mesh.cpp
        src/Core/ResourceHandler/Shader.cpp
        src/Core/ResourceHandler/MeshLoader.cpp
        src/Core/ResourceHandler/ShaderLoader.cpp
        src/Core/ResourceHandler/FontLoader.cpp
        src/Core/ResourceHandler/TextureLoader.cpp

        # Layout
        src/Core/LayoutHandler/LayoutBase.cpp
        src/Core/LayoutHandler/BaseCalculator.cpp
        src/Core/LayoutHandler/Calculators/PaneCalculator.cpp
        src/Core/LayoutHandler/Calculators/DropdownCalculator.cpp
        src/Core/LayoutHandler/Calculators/SliderCalculator.cpp
        src/Core/LayoutHandler/Calculators/SplitPaneCalculator.cpp

        # Text
        src/Core/TextHandler/CaretHelper.cpp
        src/Core/TextHandler/TextBatchStore.cpp

        # UI Nodes
        src/Node/UIBase.cpp
        src/Node/UIWindow.cpp
        src/Node/UIButton.cpp
        src/Node/UILabel.cpp
        src/Node/UILineEdit.cpp
        src/Node/UIPane.cpp
        src/Node/UISlider.cpp
        src/Node/UIImage.cpp
        src/Node/UIDropdown.cpp
        src/Node/UISplitPane.cpp
        src/Node/UIListView.cpp
        src/Node/UITreeView.cpp
        src/Node/InternalUse/UIScroll.cpp
        src/Node/InternalUse/UIViewItem.cpp

        # Misc
        src/Utils/Logger.cpp

        # Vendors (3rd party non bindable)
        thirdparty/xml/HkXml.cpp
        thirdparty/xml/Utility.cpp
    )
endfunction(addSources)


function (addIncludePaths)
    # Include dirs for project itself
    include_directories(${CMAKE_SOURCE_DIR}/include)
    include_directories(${CMAKE_SOURCE_DIR}/thirdparty)

    # Include dirs for thirdparty
    include_directories (${GLEW_INCLUDE_PATH})
    include_directories (${GLFW_INCLUDE_PATH})
    include_directories (${FREETYPE_INCLUDE_PATH})
endfunction(addIncludePaths)


function(addCompileFeaturesToTarget)
    target_compile_features(${PROJECT_NAME} PRIVATE ${COMPILE_FEATURES})
endfunction(addCompileFeaturesToTarget)


function(addLibraryPaths)
    target_link_directories(${PROJECT_NAME} PRIVATE
        ${GLEW_LIB_PATH}
        ${GLFW_LIB_PATH}
        ${FREETYPE_LIB_PATH}
    )
endfunction(addLibraryPaths)


function(linkTargetWithLibs_x86_64_Linux)
    target_link_libraries(${PROJECT_NAME}
        glfw3
        GLEW GL
        X11
        freetype png z brotlidec brotlicommon bz2
    )
endfunction(linkTargetWithLibs_x86_64_Linux)


function(setupForLinux_x86_64)
    message(NOTICE "[>] Configuring for Linux x86_64!")

    addIncludePaths()
    addSources()

    addCompileFeaturesToTarget()

    addLibraryPaths()
    linkTargetWithLibs_x86_64_Linux()

    message(NOTICE "[>] DONE for Linux x86_64!")
endfunction(setupForLinux_x86_64)


function(linkTargetWithLibs_x86_64_Windows)
    target_link_libraries(${PROJECT_NAME}
        glew32
        winglfw3
        opengl32
        gdi32
        user32
        kernel32
        shell32
        freetype-6
    )
endfunction(linkTargetWithLibs_x86_64_Windows)


function(setupForWindows_x86_64)
    message(NOTICE "[>] Configuring for Windows x86_64!")

    addIncludePaths()
    addSources()

    addCompileFeaturesToTarget()

    addLibraryPaths()
    linkTargetWithLibs_x86_64_Windows()

endfunction(setupForWindows_x86_64)

function(main)
    if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
        setupForLinux_x86_64()
    elseif ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
        setupForWindows_x86_64()
    elseif ("${CMAKE_SYSTEM_NAME}" MATCHES "MSYS")
        setupForWindows_x86_64()
    else()
        message(FATAL_ERROR "Unsupported operating system: ${CMAKE_SYSTEM_NAME}")
    endif()
endfunction(main)


# Entry
main()

# set_target_properties(MyApp PROPERTIES
#     INSTALL_RPATH "$ORIGIN/../lib"
# )