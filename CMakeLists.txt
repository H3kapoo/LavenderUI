cmake_minimum_required(VERSION 3.22)

# All this needs glm, glew, glfw3 and freetype2
project(toying)

set(CXX_STANDARD 23)
set(PLATFORM_NAME ${CMAKE_SYSTEM_NAME})

set(GLEW_INCLUDE_PATH     "vendor/glew/include/")
set(GLFW_INCLUDE_PATH     "vendor/glfw/include/")
set(FREETYPE_INCLUDE_PATH "vendor/freetype/include/")

set(GLEW_LIB_PATH     "vendor/glew/lib/")
set(GLFW_LIB_PATH     "vendor/glfw/lib/")
set(FREETYPE_LIB_PATH "vendor/freetype/lib/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -ggdb -g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../debug)

if(${PLATFORM_NAME} MATCHES "Linux")
    message(STATUS "Configuring for Linux..")

    include_directories (${CMAKE_SOURCE_DIR})
    include_directories (${GLEW_INCLUDE_PATH})
    include_directories (${GLFW_INCLUDE_PATH})
    include_directories (${FREETYPE_INCLUDE_PATH})

    add_executable(${PROJECT_NAME}
        main.cpp
        src/App.cpp
        src/Core/LavParser/LavParser.cpp
        src/Core/ResourceHandler/Mesh.cpp
        src/Core/ResourceHandler/Shader.cpp
        src/Core/ResourceHandler/MeshLoader.cpp
        src/Core/ResourceHandler/ShaderLoader.cpp
        src/Core/ResourceHandler/FontLoader.cpp
        src/Core/ResourceHandler/TextureLoader.cpp
        src/Core/TextHandler/TextAttribs.cpp
        src/Core/LayoutHandler/LayoutBase.cpp
        src/Core/LayoutHandler/BasicCalculator.cpp
        src/Node/UIBase.cpp
        src/Node/UIWindow.cpp
        src/Node/UIButton.cpp
        src/Node/UILabel.cpp
        src/Node/UIPane.cpp
        # src/UIElements/UISplitPane.cpp
        src/Node/UISlider.cpp
        src/Node/InternalUse/UIScroll.cpp
        # src/UIElements/UITreeView.cpp
        # src/UIElements/UIDropdown.cpp
        src/Node/UIImage.cpp
        src/Core/Binders/WindowBinder.cpp
        src/Core/Binders/GPUBinder.cpp
        src/Core/Binders/FileResourceBinder.cpp
        src/Utils/Logger.cpp

        vendor/xml/HkXml.cpp
        vendor/xml/Utility.cpp
    )

    # Compile features
    target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

    # Needed for absolute include paths
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}
    )

    # Link libs directories
    target_link_directories(${PROJECT_NAME} PUBLIC
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
    )

# If the operating system is not recognized
else()
    message(FATAL_ERROR "Unsupported operating system: ${CMAKE_SYSTEM_NAME}")
endif()