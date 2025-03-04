cmake_minimum_required(VERSION 3.10)  # Minimum CMake version
project(slimeEngine LANGUAGES C CXX)  # Define project and language

set(CMAKE_CXX_STANDARD 20)             # Use C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)    # Enforce C++ standard
set(CMAKE_BUILD_TYPE Debug)            # Use Debug mode

# Compiler-Specific Warning Flags
if(MSVC)  # Microsoft Visual C++ Compiler
    add_compile_options(/W4)  # Use /W4 for stricter warnings in MSVC
else()  # GCC/Clang (Linux/macOS)
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()


set(GLFW3_LIBRARY "C:/Users/zq/Documents/GitHub/slimeEngine/libs/glfw3.lib")
set(GLFW3_INCLUDE "C:/Users/zq/Documents/GitHub/slimeEngine/include/GLFW")


# Include directories
include_directories(${GLFW3_INCLUDE})
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SRC_FILES src/main.cpp src/stb_image.cpp)

# Add GLAD as a static library
add_library(GLAD STATIC "src/glad.c")
set(LIBS GLAD)
target_include_directories(GLAD PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Add executable
add_executable(slimeEngine ${SRC_FILES})
target_include_directories(slimeEngine PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(slimeEngine PUBLIC ${CMAKE_SOURCE_DIR}/src/structs)

# Add GLFW as a required library
target_link_libraries(slimeEngine GLAD ${GLFW3_LIBRARY})

