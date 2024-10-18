#pragma once
#include <type_traits>

namespace WayLib {
    template<auto... args>
    using value_void_ptr_t = void*;
}
