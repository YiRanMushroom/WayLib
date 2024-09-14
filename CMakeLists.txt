cmake_minimum_required(VERSION 3.28)
project(WayLib)

set(CMAKE_CXX_STANDARD 23)

file(GLOB WAYLIB_SOURCES vendor/WayLib/impl/*.cpp)
include_directories(WayLib vendor/WayLib/include)
add_executable(WayLib ${WAYLIB_SOURCES})

add_executable(WayLib
        main.cpp
        vendor/WayLib/include/util/Stream.hpp
        vendor/WayLib/include/util/StreamUtil.hpp
        vendor/WayLib/impl/Stream/impl.cpp
)

