cmake_minimum_required(VERSION 3.6)

project(YAKKA)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Tell CMake to create the helloworld executable
set (CMAKE_CXX_STANDARD 14)

add_subdirectory(lib)
add_subdirectory(test)
