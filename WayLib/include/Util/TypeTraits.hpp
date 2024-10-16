#pragma once
#include <type_traits>

namespace WayLib {
    template<template<typename...> class Template, typename Instance, typename Default = void>
    constexpr bool is_specialization_of_v = false;

}
