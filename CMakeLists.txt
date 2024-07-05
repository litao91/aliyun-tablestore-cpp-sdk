cmake_minimum_required(VERSION 3.2)

project(ots)

set(CMAKE_CXX_STANDARD 23)
include_directories(${ots_SOURCE_DIR}/src)
find_package(Boost 1.85.0 REQUIRED COMPONENTS system)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()
add_subdirectory(src)
