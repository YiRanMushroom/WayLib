#pragma once

namespace WayLib {
    template<typename T, typename = void>
struct is_tuple_like : std::false_type {
    };

    template<typename T>
    struct is_tuple_like<T, std::void_t<decltype(std::tuple_size<T>::value)> > : std::true_type {
    };

    template<typename T>
    inline constexpr bool is_tuple_like_v = is_tuple_like<T>::value;
}