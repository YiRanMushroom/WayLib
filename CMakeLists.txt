cmake_minimum_required(VERSION 3.28)
project(WayLib_Development)

set(CMAKE_CXX_STANDARD 17)

# Please add WayLib/include to your include path, like this, assuming your project is called _Example_Project_, and the WayLib directory is in the root directory (with CMakeLists.txt):
# This is an example of how to include the header files in your source code.

include_directories(${CMAKE_SOURCE_DIR}/WayLib/include
        #                   ...(other include directories that you need)
)

# This line must be added before the add_executable or add_library command.

add_executable(WayLib_Development
        main.cpp
        WayLib/include/Util/FileSystem.hpp
        WayLib/include/Util/Exceptions.hpp
        WayLib/include/CRTP/inject_stream_traits.hpp
        WayLib/include/Container/ThreadsafeQueue.hpp
        WayLib/include/Util/ThreadPool.hpp
        WayLib/include/Util/Range/Range.hpp
        WayLib/include/Util/Range/RangeUtil.hpp
        WayLib/include/Util/TypeTraits.hpp
        WayLib/include/Util/OperatorExtension.hpp
)
