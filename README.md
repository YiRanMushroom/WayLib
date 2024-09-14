WayLib is a open source library developed by Yiran Wang, which is originally for many of his small projects. It is a C++ library that provides some useful functions and classes for general use. It is still under development and will be updated from time to time. The library is designed to be easy to use and easy to understand. It is also designed to be cross-platform and can be used on Windows, Linux and MacOS. The library is licensed under the MIT license, so you can use it in your projects without any restrictions. If you have any questions or suggestions, please feel free to contact me.

If you want to use this lib, simply copy the vendor directory into your own project, and add the following line to your CmakeList.txt:
```
file(GLOB WAYLIB_SOURCES vendor/WayLib/impl/*.cpp)
include_directories(#<YOUR_PROJECT_NAME> vendor/WayLib/include)

# This line must be added after the add_executable command
target_sources(WayLib PRIVATE ${WAYLIB_SOURCES})
```

This should be enough to get you started. If you have any questions or suggestions, please feel free to contact me. I will be happy to help you. Thank you for using WayLib!