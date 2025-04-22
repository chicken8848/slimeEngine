cmake_minimum_required(VERSION 3.10) # Minimum CMake version
project(slimeEngine LANGUAGES C CXX) # Define project and language

set(CMAKE_CXX_STANDARD 20) # Use C++17 (or change to 20 if needed)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Enforce C++ standard
set(CMAKE_BUILD_TYPE Debug) # Use Debug mode
set(CMAKE_CXX_FLAGS "-Wall -Wextra -g") # Add warning and debug flags

set(GLFW3_INCLUDE "${CMAKE_SOURCE_DIR}/include/GLFW")
set(GLFW3_LIBRARY "${CMAKE_SOURCE_DIR}/libs/libglfw3.a")
set(ASSIMP_LIBRARY "${CMAKE_SOURCE_DIR}/libs/assimp/libassimp.so")

set(IMGUI_INCLUDE "${CMAKE_SOURCE_DIR}/include/imgui_DIR")
set(IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/include/imgui/cpp/imgui.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/cpp/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/cpp/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/cpp/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/cpp/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/cpp/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/include/imgui/cpp/imgui_impl_opengl3.cpp)

include_directories(${GLFW3_INCLUDE})
include_directories(${IMGUI_INCLUDE})
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

link_directories(${CMAKE_SOURCE_DIR}/libs)

set(SRC_FILES src/main.cpp src/stb_image.cpp)

configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

add_library(GLAD STATIC "src/glad.c")
set(LIBS GLAD)
target_include_directories(GLAD PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(slimeEngine ${SRC_FILES} ${IMGUI_SOURCES})
target_include_directories(slimeEngine PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(slimeEngine PUBLIC ${CMAKE_SOURCE_DIR}/src/structs)
# target_link_libraries(slimeEngine GLAD ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY})
target_link_libraries(slimeEngine GLAD glfw3 assimp)

if(APPLE)
  target_link_libraries(slimeEngine "-framework OpenGL" "-framework Cocoa"
                        "-framework IOKit")
elseif(UNIX)
  target_link_libraries(
    slimeEngine
    GL
    pthread
    X11
    Xrandr
    Xi
    dl)
endif()
