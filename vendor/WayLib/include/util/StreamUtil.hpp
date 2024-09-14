#pragma once

#include <set>
#include <vector>
#include <util/Stream.hpp>
#include <string>

namespace Collectors {
    inline auto toVector() {
        return [](auto begin, auto end) {
            std::vector<std::remove_reference_t<decltype(*begin)> > result;
            for (auto it = begin; it != end; ++it) {
                result.push_back(std::move(*it));
            }
            return result;
        };
    }

    inline auto toUnorderedSet() {
        return [](auto begin, auto end) {
            std::unordered_set<std::remove_reference_t<decltype(*begin)> > result;
            for (auto it = begin; it != end; ++it) {
                result.insert(std::move(*it));
            }
            return result;
        };
    }

    inline auto toSet() {
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
    inline Stream<T> of(const std::vector<T> &vec) {
        return Stream<T>(vec.begin(), vec.end());
    }

    template<typename T>
    inline Stream<T> of(std::vector<T> &&vec) {
        Stream<T> res{};
        res.getData() = std::move(vec);
        return res;
    }

    // std::array
    template<typename T, size_t N>
    inline Stream<T> of(const std::array<T, N> &arr) {
        return Stream<T>(arr.begin(), arr.end());
    }

    template<typename T, size_t N>
    inline Stream<T> of(std::array<T, N> &&arr) {
        Stream<T> res{};
        auto &&vec = res.getData();
        for (auto &&e: arr) {
            vec.push_back(std::move(e));
        }
        return res;
    }
}

namespace Transformers {
    inline auto toString() {
        return [](auto &&e) {
            return std::to_string(e);
        };
    }

    template<typename T>
    inline auto castTo() {
        return [](auto &&e) {
            return static_cast<T>(e);
        };
    }

    template<typename T>
    inline auto constructTo() {
        return []<typename... Args>(Args &&... args) {
            return T(std::forward<Args>(args)...);
        };
    }

    inline auto allOf() {
        return [](auto &&e) {
            return std::make_pair(e.begin(), e.end());
        };
    }

    inline auto identityOf() {
        return [](auto &&e) -> decltype(auto) {
            return std::forward<decltype(e)>(e);
        };
    }

    inline auto addressOf() {
        return [](auto &&e) {
            return &e;
        };
    }
}