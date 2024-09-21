#pragma once

#include <set>
#include <vector>
#include <Util/Stream.hpp>
#include <string>
#include <iostream>

namespace WayLib {
    namespace Collectors {
        inline auto ToVector() {
            return [](auto begin, auto end) {
                std::vector<std::remove_reference_t<decltype(*begin)> > result;
                for (auto it = begin; it != end; ++it) {
                    result.push_back(std::move(*it));
                }
                return result;
            };
        }

        inline auto ToUnorderedSet() {
            return [](auto begin, auto end) {
                std::unordered_set<std::remove_reference_t<decltype(*begin)> > result;
                for (auto it = begin; it != end; ++it) {
                    result.insert(std::move(*it));
                }
                return result;
            };
        }

        inline auto ToSet() {
            return [](auto begin, auto end) {
                std::set<std::remove_reference_t<decltype(*begin)> > result;
                for (auto it = begin; it != end; ++it) {
                    result.insert(std::move(*it));
                }
                return result;
            };
        }
    }

    namespace Streamers {
        template<typename T>
        inline Stream<T> Of(const std::vector<T> &vec) {
            return Stream<T>(vec.begin(), vec.end());
        }

        template<typename T>
        inline Stream<T> Of(std::vector<T> &&vec) {
            Stream<T> res{};
            res.setData(std::move(vec));
            return res;
        }

        // std::array
        template<typename T, size_t N>
        inline Stream<T> Of(const std::array<T, N> &arr) {
            return Stream<T>(arr.begin(), arr.end());
        }

        template<typename T, size_t N>
        inline Stream<T> Of(std::array<T, N> &&arr) {
            Stream<T> res{};
            for (auto &&e: arr) {
                res.add(std::move(e));
            }
            return res;
        }
    }

    namespace Transformers {
        inline auto ToString() {
            return [](auto &&e) {
                return std::to_string(e);
            };
        }

        template<typename T>
        inline auto CastTo() {
            return [](auto &&e) {
                return static_cast<T>(e);
            };
        }

        template<typename T>
        inline auto ConstructTo() {
            return []<typename... Args>(Args &&... args) {
                return T(std::forward<Args>(args)...);
            };
        }

        inline auto AllOf() {
            return [](auto &&e) {
                return std::make_pair(e.begin(), e.end());
            };
        }

        inline auto IdentityOf() {
            return [](auto &&e) -> decltype(auto) {
                return std::forward<decltype(e)>(e);
            };
        }

        inline auto AddressOf() {
            return [](auto &&e) {
                return &e;
            };
        }

        inline auto Add() {
            return [](auto &&a, auto &&b) {
                return a + b;
            };
        }

        inline auto Multiplies() {
            return [](auto &&a, auto &&b) {
                return a * b;
            };
        }

        template<typename T>
        inline auto MakeUnique() {
            return []<typename Type>(Type &&e) {
                if constexpr (!is_tuple_like_v<Type>) {
                    return std::make_unique<T>(std::forward<decltype(e)>(e));
                } else {
                    return std::apply([](auto &&... args) {
                        return std::make_unique<T>(std::forward<decltype(args)>(args)...);
                    }, std::forward<decltype(e)>(e));
                }
            };
        }

        template<typename T>
        inline auto MakeShared() {
            return []<typename Type>(Type &&e) {
                if constexpr (!is_tuple_like_v<Type>) {
                    return std::make_shared<T>(std::forward<decltype(e)>(e));
                } else {
                    return std::apply([](auto &&... args) {
                        return std::make_shared<T>(std::forward<decltype(args)>(args)...);
                    }, std::forward<decltype(e)>(e));
                }
            };
        }
    }

    namespace Utils {
        inline auto PrintAll(std::ostream &os = std::cout) {
            return [&](auto &&e) {
                e.forEach([&](auto &&el) { os << el << ' '; })
                        .then([&] { os << std::endl; });
            };
        }
    }
}
