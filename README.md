WayLib is a open source library developed by Yiran Wang, which is originally for many of his small projects. It is a C++ library that provides some useful functions and classes for general use. It is still under development and will be updated from time to time. The library is designed to be easy to use and easy to understand.

WayLib is (hopefully) an inline header only library, which means you can simply copy the include directory into your own project, and include the header files in your source code.

WayLib is only for C++23 or later, and can now only be compiled using MSVC. I will try to make it compatible with other compilers in the future.

If you want to use this lib, simply copy the WayLib directory into your own project, and add the "WayLib/include" directory to your include path. Then you can include the header files in your source code.
```
# Please add WayLib/include to your include path, like this, assuming your project is called _Example_Project_, and the WayLib directory is in the root directory (with CMakeLists.txt):
# This is an example of how to include the header files in your source code.

include_directories(${CMAKE_SOURCE_DIR}/WayLib/include
#                   ...(other include directories that you need)
)

# This line must be added before the add_executable or add_library command.
```

This should be enough to get you started. If you have any questions or suggestions, please feel free to contact me. I will be happy to help you. Thank you for using WayLib!