// #pragma once
#ifndef DEFWAYMACRO_HPP
#define DEFWAYMACRO_HPP

#define _self_ std::forward<decltype(self)>(self)
#define _forward_(...) std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)
#define _declself_ this auto&& self

#define WayLibOnlyMovable(Type) \
    Type(Type&&) = default; \
    Type& operator=(Type&&) = default; \
    Type(const Type&) = delete; \
    Type& operator=(const Type&) = delete;

#endif // DEFWAYMACRO_HPP
