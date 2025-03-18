cmake_minimum_required(VERSION 3.10) # Minimum CMake version
project(slimeEngine LANGUAGES C CXX) # Define project and language

set(CMAKE_CXX_STANDARD 20) # Use C++17 (or change to 20 if needed)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Enforce C++ standard
set(CMAKE_BUILD_TYPE Debug) # Use Debug mode


# Compiler-Specific Warning Flags
if(MSVC)  # Microsoft Visual C++ Compiler
    add_compile_options(/W4)  # Use /W4 for stricter warnings in MSVC
    set(GLFW3_LIBRARY "C:/Users/zq/Documents/GitHub/slimeEngine/libs/glfw3.lib")
    set(GLFW3_INCLUDE "C:/Users/zq/Documents/GitHub/slimeEngine/include/GLFW")
    set(ASSIMP_LIBRARY "C:/Users/zq/Documents/GitHub/slimeEngine/libs/assimp/assimp-vc143-mtd.lib")
else()  # GCC/Clang (Linux/macOS)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -g") # Add warning and debug flags
    set(GLFW3_LIBRARY
    "$ENV{HOME}/Documents/programming/third-party-libraries/opengl/libs/libglfw3.a"
)
    set(GLFW3_INCLUDE
    "$ENV{HOME}/Documents/programming/third-party-libraries/opengl/include")
    find_package(assimp REQUIRED)
endif()




include_directories(${GLFW3_INCLUDE})
include_directories(${CMAKE_SOURCE_DIR}/include)

link_directories(${CMAKE_SOURCE_DIR}/lib/)

set(SRC_FILES src/main.cpp src/stb_image.cpp)

configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

add_library(GLAD STATIC "src/glad.c")
set(LIBS GLAD)
target_include_directories(GLAD PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(slimeEngine ${SRC_FILES})
target_include_directories(slimeEngine PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(slimeEngine PUBLIC ${CMAKE_SOURCE_DIR}/src/structs)

target_link_libraries(slimeEngine GLAD ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY})

if(UNIX)
  target_link_libraries(
    slimeEngine
    GL
    pthread
    X11
    Xrandr
    Xi
    dl)
endif()
