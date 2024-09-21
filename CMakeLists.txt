cmake_minimum_required(VERSION 3.28)
project(WayLib_Development)

set(CMAKE_CXX_STANDARD 23)

# Please add WayLib/include to your include path, like this, assuming your project is called _Example_Project_, and the WayLib directory is in the root directory (with CMakeLists.txt):
# This is an example of how to include the header files in your source code.

include_directories(${CMAKE_SOURCE_DIR}/WayLib/include
        #                   ...(other include directories that you need)
)

# This line must be added before the add_executable or add_library command.

add_executable(WayLib_Development
        main.cpp
)
