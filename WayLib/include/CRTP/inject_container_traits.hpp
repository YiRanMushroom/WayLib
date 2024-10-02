#pragma once

#include <unordered_set>
#include <algorithm>
#include <functional>
#include <optional>
#include <unordered_map>
#include <type_traits>

#include "Macro/DefWayMacro.hpp"
#include "Macro/DefWayMacro.hpp"
#include "Util/TypeTraits.hpp"

namespace WayLib {
    struct inject_container_primitive_traits_check {
        auto begin(_declself_) {
            static_assert(false, "Container must have begin() method");
        }

        auto end(_declself_) {
            static_assert(false, "Container must have end() method");
        }

        decltype(auto) push(_declself_, auto &&) {
            static_assert(false, "Container must have push() method");
        }

        size_t size(_declself_) {
            static_assert(false, "Container must have size() method");
        }

        decltype(auto) emplace(_declself_, auto &&...) {
            static_assert(false, "Container must have emplace() method");
        }

        decltype(auto) resize(_declself_, size_t) {
            static_assert(false, "Container must have resize() method");
        }

        decltype(auto) erase(_declself_, auto &&...) {
            static_assert(false, "Container must have erase() method");
        }

        decltype(auto) setData(_declself_, auto &&) {
            static_assert(false, "Container must have setData() method");
        }
    };

    struct inject_type_converts {
        decltype(auto) forward(_declself_, auto &&item) {
            if constexpr (std::is_rvalue_reference_v<decltype(_self_)>) {
                return std::move(item);
            } else {
                return item;
            }
        }

        decltype(auto) forward(_declself_) {
            return _self_;
        }

        decltype(auto) move(_declself_) {
            return std::move(self);
        }
    };

    template<template<typename...> typename Container, typename T>
    struct inject_container_traits : public inject_container_primitive_traits_check,
                                     public inject_type_converts {
        auto collect(_declself_, auto &&collector) {
            return collector(_self_.begin(), _self_.end());
        }

        decltype(auto) forEach(_declself_, auto &&action) {
            std::for_each(_self_.begin(), _self_.end(),
                          [&](auto &&item) {
                              action(_self_.forward(item));
                          });
            return _self_;
        }

        auto filtered(_declself_, auto &&filter) {
            Container<T> result;
            _self_.forEach([&](auto &&item) {
                if (filter(item)) {
                    result.push(_self_.forward(item));
                }
            });
            return result;
        }

        decltype(auto) distincted(_declself_) {
            std::unordered_set<T> set;
            return _self_.filtered([&](auto &&item) {
                if (set.contains(item)) {
                    return false;
                }
                set.insert(item);
                return true;
            });
        }

        // change sortedWith to change all sort behavior
        decltype(auto) sortWith(_declself_, auto &&comparator) {
            std::sort(_self_.begin(), _self_.end(), _forward_(comparator));
            return _self_;
        }

        decltype(auto) sort(_declself_) {
            return _self_.sortedWith(std::less{});
        }

        decltype(auto) sortDesc(_declself_) {
            return _self_.sortedWith(std::greater{});
        }

        decltype(auto) sortBy(_declself_, auto &&transform) {
            return _self_.sortedWith([&](const T &a, const T &b) {
                return transform(a) < transform(b);
            });
        }

        decltype(auto) sortByDesc(_declself_, auto &&transform) {
            return _self_.sortedWith([&](const T &a, const T &b) {
                return transform(a) > transform(b);
            });
        }

        // very dangerous, easy to unintentionally modify the container
        decltype(auto) let(_declself_, auto &&action) {
            static_assert(std::is_same_v<std::invoke_result_t<decltype(action), decltype(self)>, void>,
                          "The action must return void to avoid unintentional modification of the container");
            action(_self_);
            return _self_;
        }

        decltype(auto) then(_declself_, auto &&action) {
            action();
            return _self_;
        }

        decltype(auto) apply(_declself_, auto &&action) {
            return action(_self_);
        }

        auto groupBy(_declself_, auto &&mapper) {
            using Type = std::invoke_result_t<decltype(mapper), T>;
            if constexpr (is_tuple_like_v<Type>) {
                using K = std::tuple_element_t<0, Type>;
                using V = std::tuple_element_t<1, Type>;
                std::unordered_map<K, V> result;
                _self_.forEach([&](auto &&item) {
                    auto &&[key, value] = mapper(_forward_(item));
                    result[key] = std::move(value);
                });
                return result;
            } else {
                std::unordered_map<Type, T> result;

                for (auto &&item: _self_) {
                    auto key = mapper(_forward_(item));
                    result[key] = std::move(item);
                }

                return result;
            }
        }

        auto groupMultipleBy(_declself_, auto &&mapper) {
            // use std::unordered_multimap
            using Type = std::invoke_result_t<decltype(mapper), T>;
            if constexpr (is_tuple_like_v<Type>) {
                using K = std::tuple_element_t<0, Type>;
                using V = std::tuple_element_t<1, Type>;
                std::unordered_multimap<K, V> result;
                _self_.forEach([&](auto &&item) {
                    auto &&[key, value] = mapper(_forward_(item));
                    result.emplace(key, std::move(value));
                });
                return result;
            } else {
                std::unordered_multimap<Type, T> result;

                _self_.forEach([&](auto &&item) {
                    auto key = mapper(_forward_(item));
                    result.emplace(key, _self_.forward(item));
                });

                return result;
            }
        }


        auto fold(_declself_, auto &&init, auto &&reducer) {
            _self_.forEach([&](auto &&item) {
                init = reducer(_forward_(init), _self_.forward(item));
            });
            return init;
        }


        std::optional<T> reduce(_declself_, auto &&reducer) {
            if (_self_.empty()) {
                return std::nullopt;
            }
            T result = _self_.forward(*_self_.begin());
            for (auto it = _self_.begin() + 1; it != _self_.end(); ++it) {
                result = reducer(std::move(result), _self_.forward(*it));
            }
            return result;
        }

        decltype(auto) joined(_declself_, const auto &other) {
            std::for_each(other.begin(), other.end(), [&](auto &&item) {
                _self_.push(item);
            });
            return _self_;
        }

        decltype(auto) joined(_declself_, auto &&other) {
            std::for_each(other.begin(), other.end(), [&](auto &&item) {
                _self_.push(std::move(item));
            });
            return _self_;
        }

        decltype(auto) skipped(_declself_, size_t n) {
            if (_self_.size() > n) {
                _self_.erase(_self_.begin(), _self_.begin() + n);
            }
            return _self_;
        }

        auto partition(_declself_, auto &&predicate) {
            std::remove_reference_t<decltype(self)> result, other;
            _self_.forEach([&](auto &&item) {
                if (predicate(item)) {
                    result.push(_self_.forward(item));
                } else {
                    other.push(_self_.forward(item));
                }
            });
            return std::make_pair(result, other);
        }

        template<typename Comparator = std::less<> >
        auto min(_declself_, Comparator comparator = Comparator()) {
            if (_self_.empty()) {
                return std::nullopt;
            }
            return *std::min_element(_self_.begin(), _self_.end(), _forward_(comparator));
        }

        template<typename Comparator = std::less<> >
        auto max(_declself_, Comparator comparator = Comparator()) {
            if (_self_.empty()) {
                return std::nullopt;
            }
            return *std::max_element(_self_.begin(), _self_.end(), _forward_(comparator));
        }

        bool anyMatch(_declself_, auto &&predicate) {
            return std::any_of(_self_.begin(), _self_.end(), _forward_(predicate));
        }

        bool allMatch(_declself_, auto &&predicate) {
            return std::all_of(_self_.begin(), _self_.end(), _forward_(predicate));
        }

        bool noneMatch(_declself_, auto &&predicate) {
            return std::none_of(_self_.begin(), _self_.end(), _forward_(predicate));
        }

        auto findFirst(_declself_, auto &&predicate) {
            auto it = std::find_if(_self_.begin(), _self_.end(),
                                   _forward_(predicate));
            if (it == _self_.end()) {
                return std::nullopt;
            }
            return *it;
        }

        auto findAny(_declself_, auto &&predicate) {
            return _self_.findFirst(_forward_(predicate));
        }

        decltype(auto) forEachIndexed(_declself_, auto &&action) {
            size_t index = 0;
            _self_.forEach([&](auto &&item) {
                action(index++, _forward_(item));
            });
            return _self_;
        }

        auto flatMapped(_declself_, auto &&transformer) {
            using ResultType = std::remove_reference_t<decltype(*std::declval<
                std::invoke_result_t<decltype(transformer), T> >().first)>;
            Container<ResultType> result;
            _self_.forEach([&](auto &&item) {
                auto transformed = transformer(_self_.forward(item));
                auto &&[begin, end] = transformed;
                std::for_each(begin, end, [&](auto &&el) {
                    result.push(_self_.forward(el));
                });
            });
            return result;
        }

        auto runningReduced(_declself_, auto &&reducer) {
            if (self.empty()) {
                return Container<T>();
            }
            Container<T> result;
            T acc = _self_.forward(*_self_.begin());
            result.push(acc);
            for (auto it = _self_.begin() + 1; it != _self_.end(); ++it) {
                acc = reducer(std::move(acc), _self_.forward(*it));
                result.push(acc);
            }
            return result;
        }

        auto runningFolded(_declself_, auto &&init, auto &&reducer) {
            using U = decltype(reducer(_forward_(init), _self_.forward(*_self_.begin())));
            Container<U> stream;
            decltype(init) res = _forward_(init);
            stream.push(res);
            _self_.forEach([&](auto &&item) {
                res = reducer(std::move(res), _self_.forward(item));
                stream.push(res);
            });
            return stream;
        }

        auto mapped(_declself_, auto &&transform) {
            using ResultType = std::remove_reference_t<std::invoke_result_t<decltype(transform), T> >;
            Container<ResultType> result;
            _self_.forEach([&](auto &&item) {
                result.push(transform(_self_.forward(item)));
            });
            return result;
        }

        // TransformerType: T -> std::optional<U>
        auto mappedNotNull(_declself_, auto &&transformer) {
            using ResultType = std::remove_reference_t<decltype(*std::declval
                <std::invoke_result_t<decltype(transformer), T> >())>;
            Container<ResultType> result;
            _self_.forEach([&](auto &&item) {
                auto transformed = transformer(_self_.forward(item));
                if (transformed.has_value()) {
                    result.push(std::move(transformed.value()));
                }
            });
            return result;
        }

        // find the first element that satisfies the predicate
        // if not found, return std::nullopt
        std::optional<size_t> binarySearch(_declself_, const auto &func) {
            size_t left = 0;
            size_t right = _self_.size();

            while (left < right) {
                size_t mid = left + (right - left) / 2;
                if (func(_self_[mid])) {
                    right = mid;
                } else {
                    left = mid + 1;
                }
            }

            if (left < _self_.size() && func(_self_[left])) {
                return left;
            }

            return std::nullopt;
        }
    };
}

#include "Macro/UndefWayMacro.hpp"
