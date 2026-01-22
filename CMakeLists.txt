cmake_minimum_required(VERSION 3.22)

# All this needs glm, glew, glfw3 and freetype2
project("lavenderui")
set(CXX_STANDARD 23)
set(PLATFORM_NAME ${CMAKE_SYSTEM_NAME})

# C++ Flags
set(LIB_DEBUG_INFO_FLAGS "-ggdb -g")
set(LIB_EXTRA_WARNINGS "-Wall -Wextra -Wshadow")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIB_EXTRA_WARNINGS} ${LIB_DEBUG_INFO_FLAGS}")

# Output library path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

function(setup_for_linux_x86_64)
    message(NOTICE "[>] Configuring for Linux x86_64!")

    # Thirdparty
    set(GLEW_INCLUDE_PATH     "thirdparty/glew/include/")
    set(GLFW_INCLUDE_PATH     "thirdparty/glfw/include/")
    set(FREETYPE_INCLUDE_PATH "thirdparty/freetype/include/")

    set(GLEW_LIB_PATH     "thirdparty/glew/lib/")
    set(GLFW_LIB_PATH     "thirdparty/glfw/lib/")
    set(FREETYPE_LIB_PATH "thirdparty/freetype/lib/")

    # Include dirs for thirdparty
    include_directories (${CMAKE_SOURCE_DIR})
    include_directories (${GLEW_INCLUDE_PATH})
    include_directories (${GLFW_INCLUDE_PATH})
    include_directories (${FREETYPE_INCLUDE_PATH})

    add_executable(${PROJECT_NAME}
        temporaryPlayground/main.cpp
        src/App.cpp

        # Binders
        src/Core/Binders/WindowBinder.cpp
        src/Core/Binders/GPUBinder.cpp
        src/Core/Binders/FileResourceBinder.cpp

        # Parser
        src/Core/ViewParser/LavParser.cpp
        src/Core/ViewParser/ParseHelpers.cpp
        src/Core/ViewParser/Rules/AppRule.cpp
        src/Core/ViewParser/Rules/PaneRule.cpp
        src/Core/ViewParser/Rules/ImageRule.cpp
        src/Core/ViewParser/Rules/ButtonRule.cpp
        src/Core/ViewParser/Rules/LabelRule.cpp
        src/Core/ViewParser/Rules/SliderRule.cpp
        src/Core/ViewParser/Rules/DropdownRule.cpp
        src/Core/ViewParser/Rules/SplitPaneRule.cpp

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
        src/Core/TextHandler/TextAttribs.cpp

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
        src/Node/UIList.cpp
        src/Node/UITreeView.cpp
        src/Node/InternalUse/UIScroll.cpp
        src/Node/InternalUse/UIViewItem.cpp

        # Misc
        src/Utils/Logger.cpp

        # Vendors (3rd party non bindable)
        thirdparty/xml/HkXml.cpp
        thirdparty/xml/Utility.cpp
    )

    # Compile features (C++ 23)
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

    # Needed for absolute include paths
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})

    # Link libs directories
    target_link_directories(${PROJECT_NAME} PRIVATE
        ${GLEW_LIB_PATH}
        ${GLFW_LIB_PATH}
        ${FREETYPE_LIB_PATH}
    )

    # Link libs
    target_link_libraries(${PROJECT_NAME}
        glfw3
        GLEW GL
        X11
        freetype png z brotlidec brotlicommon bz2
        # TODO: All this to end up depending on png, z, brotlidec, bla bla..not worth it.
        # Maybe packaging dynamic freetype is better
    )

endfunction()

function(setup_for_windows_x86_64)
    message(FATAL_ERROR "Shit operating system not yet supported!")
endfunction(setup_for_windows_x86_64)

function(main)
    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        setup_for_linux_x86_64()
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        setup_for_windows_x86_64()
    else()
        message(FATAL_ERROR "Unsupported operating system: ${CMAKE_SYSTEM_NAME}")
    endif()
endfunction(main)

# Entry
main()

# set_target_properties(MyApp PROPERTIES
#     INSTALL_RPATH "$ORIGIN/../lib"
# )