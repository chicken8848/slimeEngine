cmake_minimum_required(VERSION 3.10)  # Minimum CMake version
project(slimeEngine CXX)                # Define project and language

set(CMAKE_CXX_STANDARD 20)             # Use C++17 (or change to 20 if needed)
set(CMAKE_CXX_STANDARD_REQUIRED ON)    # Enforce C++ standard
set(CMAKE_BUILD_TYPE Debug)  # Use Debug mode
set(CMAKE_CXX_FLAGS "-Wall -Wextra -g")  # Add warning and debug flags
set(CMAKE_SOURCE_DIR src/)

find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

SET(LIBS ${GLFW3_LIBRARY})

include_directories(include)   # Add include directory
add_executable(slimeEngine ${SOURCES})

aux_source_directory(${CMAKE_SOURCE_DIR} SRC_LIST) # Define source files by directory

# add_library(my_library STATIC utils.cpp)   # Create a static library
# target_link_libraries(slimeEngine my_library)  # Link to the executable

# add_executable(slimeEngine ${SOURCES} ${SRC_LIST})
add_executable(slimeEngine ${SRC_LIST})
target_link_libraries(slimeEngine ${GLFW3_LIBRARY})
