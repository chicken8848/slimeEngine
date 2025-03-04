cmake_minimum_required(VERSION 3.10) # Minimum CMake version
project(slimeEngine LANGUAGES C CXX) # Define project and language

set(CMAKE_CXX_STANDARD 20) # Use C++17 (or change to 20 if needed)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Enforce C++ standard
set(CMAKE_BUILD_TYPE Debug) # Use Debug mode
set(CMAKE_CXX_FLAGS "-Wall -Wextra -g") # Add warning and debug flags

set(GLFW3_LIBRARY
    "$ENV{HOME}/Documents/programming/third-party-libraries/opengl/libs/libglfw3.a"
)
set(GLFW3_INCLUDE
    "$ENV{HOME}/Documents/programming/third-party-libraries/opengl/include")

include_directories(${GLFW3_INCLUDE})
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(assimp REQUIRED)
link_directories(${CMAKE_SOURCE_DIR}/lib/)

set(SRC_FILES src/main.cpp src/stb_image.cpp)

add_library(GLAD STATIC "src/glad.c")
set(LIBS GLAD)
target_include_directories(GLAD PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(slimeEngine ${SRC_FILES})
target_include_directories(slimeEngine PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(slimeEngine PUBLIC ${CMAKE_SOURCE_DIR}/src/structs)
target_link_libraries(slimeEngine GLAD ${GLFW3_LIBRARY} assimp)

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
