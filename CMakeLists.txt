cmake_minimum_required(VERSION 3.28)
project(WayLib)

set(CMAKE_CXX_STANDARD 23)

file(GLOB WAYLIB_SOURCES vendor/WayLib/impl/*.cpp)
include_directories(WayLib vendor/WayLib/include)

add_executable(WayLib
        main.cpp
        vendor/WayLib/include/util/Stream.hpp
        vendor/WayLib/include/util/StreamUtil.hpp
        vendor/WayLib/impl/Stream/impl.cpp
        vendor/WayLib/include/util/Containers.hpp
        vendor/WayLib/include/CRTP/inject_container_traits.hpp
        vendor/WayLib/include/util/TypeTraits.hpp
)

target_sources(WayLib PRIVATE ${WAYLIB_SOURCES})