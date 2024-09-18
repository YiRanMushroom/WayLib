#pragma once

#include <unordered_set>
#include <algorithm>
#include <functional>
#include <optional>
#include <unordered_map>

#include "util/TypeTraits.hpp"

namespace WayLib {
    template<template<typename...> typename Container, typename T>
    struct inject_container_primitive_traits_check {
        auto begin(this auto &&self) {
            static_assert(false, "Container must have begin() method");
        }

        auto end(this auto &&self) {
            static_assert(false, "Container must have end() method");
        }

        decltype(auto) add(this auto &&self, auto &&) {
            static_assert(false, "Container must have add() method");
        }

        size_t size(this auto &&self) {
            static_assert(false, "Container must have size() method");
        }

        decltype(auto) emplace(this auto &&self, auto &&...) {
            static_assert(false, "Container must have emplace() method");
        }

        decltype(auto) limit(this auto &&self, size_t) {
            static_assert(false, "Container must have limit() method");
        }

        decltype(auto) erase(this auto &&self, auto &&...) {
            static_assert(false, "Container must have erase() method");
        }

        /*decltype(auto) getData(this auto &&self) {
            static_assert(false, "Container must have getData() method");
        }*/

        decltype(auto) setData(this auto &&self, auto &&) {
            static_assert(false, "Container must have setData() method");
        }
    };

    template<template<typename...> typename Container, typename T>
    struct inject_container_traits : public inject_container_primitive_traits_check<Container, T> {
        auto collect(this auto &&self, auto &&collector) {
            return collector(self.begin(), self.end());
        }

        decltype(auto) forEach(this auto &&self, auto &&action) {
            std::for_each(self.begin(), self.end(), std::forward<decltype(action)>(action));
            return std::forward<decltype(self)>(self);
        }

        auto filtered(this auto &&self, auto &&filter) {
            Container<T> result;
            self.forEach([&](auto &&item) {
                if (filter(item)) {
                    result.add(std::move(item));
                }
            });
            return result;
        }

        decltype(auto) distincted(this auto &&self) {
            std::unordered_set<T> set;
            return self.filtered([&](auto &&item) {
                if (set.contains(item)) {
                    return false;
                }
                set.insert(item);
                return true;
            });
        }

        decltype(auto) sorted(this auto &&self) {
            std::sort(self.begin(), self.end());
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) sortedDesc(this auto &&self) {
            std::sort(self.begin(), self.end(), std::greater{});
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) sortedBy(this auto &&self, auto &&transform) {
            std::sort(self.begin(), self.end(), [&](const T &a, const T &b) {
                return transform(a) < transform(b);
            });
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) sortedByDesc(this auto &&self, auto &&transform) {
            std::sort(self.begin(), self.end(), [&](const T &a, const T &b) {
                return transform(a) > transform(b);
            });
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) sortedWith(this auto &&self, auto &&comparator) {
            std::sort(self.begin(), self.end(), std::forward<decltype(comparator)>(comparator));
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) let(this auto &&self, auto &&action) {
            action(std::forward<decltype(self)>(self));
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) then(this auto &&self, auto &&action) {
            action();
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) apply(this auto &&self, auto &&action) {
            return action(std::forward<decltype(self)>(self));
        }

        auto groupBy(this auto &&self, auto &&mapper) {
            using Type = std::invoke_result_t<decltype(mapper), T>;
            if constexpr (is_tuple_like_v<Type>) {
                using K = std::tuple_element_t<0, Type>;
                using V = std::tuple_element_t<1, Type>;
                std::unordered_map<K, V> result;
                self.forEach([&](auto &&item) {
                    auto &&[key, value] = mapper(std::forward<decltype(item)>(item));
                    result[key] = std::move(value);
                });
                return result;
            } else {
                std::unordered_map<Type, T> result;

                for (auto &&item: self) {
                    auto key = mapper(std::forward<decltype(item)>(item));
                    result[key] = std::move(item);
                }

                return result;
            }
        }

        auto groupMultipleBy(this auto &&self, auto &&mapper) {
            // use std::unordered_multimap
            using PairType = std::invoke_result_t<decltype(mapper), T>;
            using K = typename PairType::first_type;
            using V = typename PairType::second_type;
            std::unordered_multimap<K, V> result;
            self.forEach([&](auto &&item) {
                auto [key, value] = mapper(std::move(item));
                result.insert({key, value});
            });
            return result;
        }


        auto fold(this auto &&self, auto &&init, auto &&reducer) {
            self.forEach([&](auto &&item) {
                init = reducer(std::move(init), std::move(item));
            });
            return init;
        }


        std::optional<T> reduce(this auto &&self, auto &&reducer) {
            if (self.empty()) {
                return std::nullopt;
            }
            T result = *self.begin();
            for (auto it = self.begin() + 1; it != self.end(); ++it) {
                result = reducer(std::move(result), std::move(*it));
            }
            return result;
        }

        decltype(auto) joined(this auto &&self, const auto &other) {
            std::for_each(other.begin(), other.end(), [&](auto &&item) {
                self.add(item);
            });
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) joined(this auto &&self, auto &&other) {
            std::for_each(other.begin(), other.end(), [&](auto &&item) {
                self.add(std::move(item));
            });
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) skipped(this auto &&self, size_t n) {
            if (self.size() > n) {
                self.erase(self.begin(), self.begin() + n);
            }
            return std::forward<decltype(self)>(self);
        }

        auto partition(this auto &&self, auto &&predicate) {
            std::remove_reference_t<decltype(self)> result, other;
            self.forEach([&](auto &&item) {
                if (predicate(item)) {
                    result.add(std::move(item));
                } else {
                    other.add(std::move(item));
                }
            });
            return std::make_pair(result, other);
        }

        template<typename Comparator = std::less<> >
        auto min(this auto &&self, Comparator comparator = Comparator()) {
            if (self.empty()) {
                return std::nullopt;
            }
            return *std::min_element(self.begin(), self.end(), std::forward<Comparator>(comparator));
        }

        template<typename Comparator = std::less<> >
        auto max(this auto &&self, Comparator comparator = Comparator()) {
            if (self.empty()) {
                return std::nullopt;
            }
            return *std::max_element(self.begin(), self.end(), std::forward<Comparator>(comparator));
        }

        bool anyMatch(this auto &&self, auto &&predicate) {
            return std::any_of(self.begin(), self.end(), std::forward<decltype(predicate)>(predicate));
        }

        bool allMatch(this auto &&self, auto &&predicate) {
            return std::all_of(self.begin(), self.end(), std::forward<decltype(predicate)>(predicate));
        }

        bool noneMatch(this auto &&self, auto &&predicate) {
            return std::none_of(self.begin(), self.end(), std::forward<decltype(predicate)>(predicate));
        }

        auto findFirst(this auto &&self, auto &&predicate) {
            auto it = std::find_if(self.begin(), self.end(),
                                   std::forward<decltype(predicate)>(predicate));
            if (it == self.end()) {
                return std::nullopt;
            }
            return *it;
        }

        auto findAny(this auto &&self, auto &&predicate) {
            return self.findFirst(std::forward<decltype(predicate)>(predicate));
        }

        decltype(auto) forEachIndexed(this auto &&self, auto &&action) {
            size_t index = 0;
            self.forEach([&](auto &&item) {
                action(index++, std::forward<decltype(item)>(item));
            });
            return std::forward<decltype(self)>(self);
        }

        auto flatMapped(this auto &&self, auto &&transformer) {
            using ResultType = std::remove_reference_t<decltype(*std::declval<
                std::invoke_result_t<decltype(transformer), T> >().first)>;
            Container<ResultType> result;
            self.forEach([&](auto &&item) {
                auto transformed = transformer(std::move(item));
                auto &&[begin, end] = transformed;
                std::for_each(begin, end, [&](auto &&el) {
                    result.add(std::move(el));
                });
            });
            return result;
        }

        auto runningReduced(this auto &&self, auto &&reducer) {
            if (self.empty()) {
                return Container<T>();
            }
            Container<T> result;
            T acc = *self.begin();
            result.add(acc);
            for (auto it = self.begin() + 1; it != self.end(); ++it) {
                acc = reducer(std::move(acc), std::move(*it));
                result.add(acc);
            }
            return result;
        }

        auto runningFolded(this auto &&self, auto &&init, auto &&reducer) {
            using U = decltype(reducer(std::move(init), *self.begin()));
            Container<U> stream;
            stream.add(init);
            self.forEach([&](auto &&item) {
                init = reducer(std::move(init), std::move(item));
                stream.add(init);
            });
            return stream;
        }

        auto mapped(this auto &&self, auto &&transform) {
            using ResultType = std::remove_reference_t<std::invoke_result_t<decltype(transform), T> >;
            Container<ResultType> result;
            self.forEach([&](auto &&item) {
                result.add(transform(std::move(item)));
            });
            return result;
        }

        // TransformerType: T -> std::optional<U>
        auto mappedNotNull(this auto &&self, auto &&transformer) {
            using ResultType = std::remove_reference_t<decltype(*std::declval
                <std::invoke_result_t<decltype(transformer), T> >())>;
            Container<ResultType> result;
            self.forEach([&](auto &&item) {
                auto transformed = transformer(std::move(item));
                if (transformed.has_value()) {
                    result.add(std::move(transformed.value()));
                }
            });
            return result;
        }
    };
}
