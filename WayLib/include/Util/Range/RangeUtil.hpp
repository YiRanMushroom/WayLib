#pragma once

#include <optional>

#include "Range.hpp"
#include "Util/TypeTraits.hpp"

#include <vector>
#include <unordered_map>
#include "Util/TypeTraits.hpp"

namespace WayLib::Ranges {
    namespace Collectors {
        inline auto ToVector() {
            return [](auto &&range) {
                return *range.get();
            };
        }

        inline auto ToString() {
            return [](auto &&range) {
                std::string result;
                for (auto &item: *range.get()) {
                    result += item;
                }
                return result;
            };
        }
    }

    struct ToRangeImplClass {
        template<typename Container>
        auto operator()(Container &&container) const {
            using T = typename std::decay_t<Container>::value_type;

            if constexpr (IsRangeV<std::decay_t<Container> >) {
                return std::forward<Container>(container);
            } else {
                if constexpr (std::is_same_v<Container, std::vector<T> &&>) {
                    return Range<T, void>(
                        [vec = std::make_shared<std::vector<T> >(std::forward<Container>(container))]() {
                            return vec;
                        });
                } else {
                    std::shared_ptr<std::vector<T> > vec = std::make_shared<std::vector<T> >();

                    vec->reserve(container.size());
                    for (auto &item: container) {
                        if constexpr (std::is_rvalue_reference_v<decltype(container)>) {
                            vec->push_back(std::move(item));
                        } else {
                            vec->push_back(item);
                        }
                    }

                    return Range<T, void>([vec]() {
                        return vec;
                    });
                }
            }
        }
    };

    inline auto toRange() {
        static ToRangeImplClass instance;
        return instance;
    }

    template<typename Collector>
    auto collect(Collector collector) {
        return [collector = std::move(collector)](auto &&range) {
            return std::invoke(collector, std::forward<decltype(range)>(range));
        };
    }

    template<typename Visitor>
    auto forEach(Visitor visitor) {
        return [visitor = std::move(visitor)](auto &&range) {
            using T = typename std::decay_t<decltype(range)>::value_type;
            using ParentType = std::decay_t<decltype(range)>;

            return Range<T, ParentType>(
                std::forward<decltype(range)>(range),
                [visitor](auto &&range) {
                    for (auto &item: *range.get()) {
                        std::invoke(visitor, item);
                    }

                    return range.get();
                });
        };
    }

    template<typename Predicate>
    auto filter(Predicate predicate) {
        return [predicate = std::move(predicate)](auto &&range) {
            using T = typename std::decay_t<decltype(range)>::value_type;
            using ParentType = typename std::decay_t<decltype(range)>;

            return Range<T, ParentType>{
                std::forward<decltype(range)>(range),
                [predicate](auto &&range) {
                    std::shared_ptr<std::vector<T> > vec = std::make_shared<std::vector<T> >();
                    for (auto &item: *range.get()) {
                        if (std::invoke(predicate, item)) {
                            vec->push_back(std::move(item));
                        }
                    }
                    return vec;
                }
            };
        };
    }

    template<typename Transformer>
    auto map(Transformer transformer) {
        return [transformer = std::move(transformer)](auto &&range) {
            using T = typename std::decay_t<decltype(range)>::value_type;
            using ParentType = std::decay_t<decltype(range)>;
            using U = std::remove_reference_t<std::invoke_result_t<decltype(transformer), T> >;
            return Range<U, ParentType>{
                std::forward<decltype(range)>(range),
                [transformer](auto &&range) {
                    std::shared_ptr<std::vector<U> > vec = std::make_shared<std::vector<U> >();

                    auto &&data = range.get();

                    for (auto &item: *data.get()) {
                        vec->push_back(std::invoke(transformer, item));
                    }

                    return vec;
                }
            };
        };
    }

    inline auto typeDecay() {
        return [](auto &&range) {
            using OriginalType = std::decay_t<decltype(range)>;
            using RangeType = Range<typename OriginalType::value_type, void>;
            return RangeType{
                [range]() {
                    return range.get();
                }
            };
        };
    }

    template<typename Visitor>
    auto let(Visitor visitor) {
        return [visitor = std::move(visitor)](auto &&range) -> decltype(auto) {
            visitor(std::forward<decltype(range)>(range));
            return std::forward<decltype(range)>(range);
        };
    }

    template<typename Action>
    auto then(Action action) {
        return [action = std::move(action)](auto &&range) -> decltype(auto) {
            action();
            return std::forward<Action>(range);
        };
    }

    inline auto evaluate() {
        return [](auto &&range) {
            return range.get();
        };
    }

    inline auto syncAndDecay() {
        return [](auto &&range) {
            return std::forward<decltype(range)>(range) | typeDecay() | sync();
        };
    }

    inline auto discardLast() {
        return [](auto &&range) {
            using RangeType = typename std::decay_t<decltype(range)>::parent_type;

            static_assert(!std::is_same_v<RangeType, void>, "Cannot discard last operation of a primary range");

            return range.getParent();
        };
    }

    template<typename Container, typename = void>
    auto concat(Container &&container) {
        return [container = std::forward<Container>(container)](auto &&range) {
            using T = typename std::decay_t<decltype(range)>::value_type;
            using ParentType = typename std::decay_t<decltype(range)>;

            std::vector<T> content;

            for (auto &item: container) {
                if constexpr (std::is_rvalue_reference_v<decltype(container)>) {
                    content.push_back(std::move(item));
                } else {
                    content.push_back(item);
                }
            }

            return Range<T, ParentType>{
                std::forward<decltype(range)>(range),
                [other = std::make_shared<std::vector<T> >(std::move(content))](auto &&range) {
                    auto data = *range.get();
                    for (auto &item: *other) {
                        data.push_back(std::move(item));
                    }
                    return std::make_shared<std::vector<T> >(std::move(data));
                }
            };
        };
    }

    template<typename... Ts>
    auto append(Ts &&... items) {
        return [items = std::make_tuple(std::forward<Ts>(items)...)](auto &&range) {
            using T = typename std::decay_t<decltype(range)>::value_type;
            using ParentType = typename std::decay_t<decltype(range)>;

            return Range<T, ParentType>{
                std::forward<decltype(range)>(range),
                [items = std::move(items)](auto &&range) {
                    auto data = *range.get();
                    std::apply([&data](auto &&... items) {
                        (data.push_back(std::forward<decltype(items)>(items)), ...);
                    }, items);
                    return std::make_shared<std::vector<T> >(std::move(data));
                }
            };
        };
    }

    template<typename F>
    auto groupBy(F &&f) {
        return [f = std::forward<F>(f)](auto &&range) {
            using T = typename std::decay_t<decltype(range)>::value_type;
            using ParentType = std::decay_t<decltype(range)>;

            using KeyType = typename std::invoke_result_t<F, T>::first_type;
            using ValueType = typename std::invoke_result_t<F, T>::second_type;

            std::unordered_map<KeyType, ValueType> map;

            range | forEach([&map, &f](auto &&item) {
                auto [key, value] = f(item);
                map[std::move(key)] = std::move(value);
            }) | sync();

            return map;
        };
    }

    template<typename F>
    auto groupWith(F &&f) {
        return [f = std::forward<F>(f)](auto &&range) {
            using T = typename std::decay_t<decltype(range)>::value_type;
            using ParentType = std::decay_t<decltype(range)>;

            using KeyType = typename std::invoke_result_t<F, T>;
            using ValueType = T;

            std::unordered_map<KeyType, ValueType> map;

            range | forEach([&map, &f](auto &&item) {
                auto key = f(item);
                map[std::move(key)] = std::move(item);
            }) | sync();
        };
    }

    template<typename F>
    auto groupTo(F &&f) {
        return [f = std::forward<F>(f)](auto &&range) {
            using T = typename std::decay_t<decltype(range)>::value_type;
            using ParentType = std::decay_t<decltype(range)>;

            using KeyType = T;
            using ValueType = std::invoke_result_t<F, T>;

            std::unordered_map<KeyType, ValueType> map;

            range | forEach([&](auto &&item) {
                auto value = f(item);
                map[std::move(item)] = std::move(value);
            }) | sync();

            return map;
        };
    }

    template<typename... Ts>
    auto split(Ts &&... s) {
        return [separators = std::make_tuple(std::forward<Ts>(s)...)](auto &&range) {
            using T = typename std::decay_t<decltype(range)>::value_type;
            using ParentType = std::decay_t<decltype(range)>;

            return Range<std::vector<T>, ParentType>{
                std::forward<decltype(range)>(range),
                [separators = std::move(separators)](auto &&range) {
                    std::vector<std::vector<T> > result;
                    auto data = *range.get();
                    std::vector<T> current;
                    for (auto &item: data) {
                        bool found = false;
                        std::apply([&](auto &&... separators) {
                            ((found |= item == separators), ...);
                        }, separators);

                        if (found) {
                            if (!current.empty()) {
                                result.push_back(std::move(current));
                                current.clear();
                            }
                        } else {
                            current.push_back(std::move(item));
                        }
                    }

                    if (!current.empty()) {
                        result.push_back(std::move(current));
                    }

                    return std::make_shared<std::vector<std::vector<T> > >(std::move(result));
                }
            };
        };
    }

    template<typename F>
    inline auto flatMap(F &&f) {
        return [f = std::forward<F>(f)](auto &&range) {
            using T = typename std::decay_t<decltype(range)>::value_type;
            using ParentType = std::decay_t<decltype(range)>;

            using U = typename std::invoke_result_t<F, T>::value_type;
            return Range<U, ParentType>{
                std::forward<decltype(range)>(range),

                [f = std::move(f)](auto &&range) {
                    std::shared_ptr<std::vector<U> > vec = std::make_shared<std::vector<U> >();
                    for (auto &item: *range.get()) {
                        auto result = std::invoke(f, item);
                        for (auto &innerItem: result) {
                            vec->push_back(std::move(innerItem));
                        }
                    }
                    return vec;
                }

            };
        };
    }

    template<typename F>
    inline auto firstMatch(F &&f) {
        return [f = std::forward<F>(f)](auto &&range) {
            std::optional<typename std::remove_reference_t<decltype(range)>::value_type> result;
            for (auto &item: *range.get()) {
                if (std::invoke(f, item)) {
                    result = item;
                    break;
                }
            }
            return result;
        };
    }

    template<typename F>
    inline auto anyMatch(F &&f) {
        return firstMatch(std::forward<F>(f));
    }

    template<typename F>
    inline auto sortedWith(F &&f) {
        return [f = std::forward<F>(f)](auto &&range) {
            using T = typename std::decay_t<decltype(range)>::value_type;
            using ParentType = std::decay_t<decltype(range)>;

            // return std::make_shared<std::vector<T> >(std::move(data));
            return Range<T, ParentType>{
                std::forward<decltype(range)>(range),
                [f = std::move(f)](auto &&range) {
                    auto data = *range.get();
                    std::sort(data.begin(), data.end(), f);
                    return std::make_shared<std::vector<T> >(std::move(data));
                }
            };
        };
    }

    template<typename F>
    inline auto sortedBy(F &&f) {
        return sortedWith([f = std::forward<F>(f)](const auto &lhs, const auto &rhs) {
            return f(lhs) < f(rhs);
        });
    }

    template<typename F>
    inline auto sortedByDescending(F &&f) {
        return sortedWith([f = std::forward<F>(f)](const auto &lhs, const auto &rhs) {
            return f(lhs) > f(rhs);
        });
    }

    inline auto sorted() {
        return sortedWith(
            [](const auto &lhs, const auto &rhs) {
                return lhs < rhs;
            }
        );
    }

    inline auto sortedDescending() {
        return sortedWith(
            [](const auto &lhs, const auto &rhs) {
                return lhs > rhs;
            }
        );
    }

    namespace Mapper {
        inline auto Add() {
            return [](auto &&container) {
                using OperandType = typename std::decay_t<decltype(container)>::value_type;
                using ResultType = decltype(std::declval<OperandType>() + std::declval<OperandType>());
                ResultType result{};
                for (auto &item: container) {
                    result += item;
                }
                return result;
            };
        }

        inline auto CharVectorToString() {
            return [](auto &&container) {
                std::string result;
                result.reserve(container.size());
                for (auto &item: container) {
                    result += item;
                }
                return result;
            };
        }

        inline auto ToString() {
            return [](auto &&container) {
                return std::to_string(container);
            };
        }
    }
}

template<typename F, std::void_t<decltype(std::is_invocable_v<F, std::string &&>)>* = nullptr>
decltype(auto) operator|(std::string &&str, F &&converter) {
    return converter(std::move(str)) | WayLib::Ranges::autoSync();
}

template<typename F, std::void_t<decltype(std::is_invocable_v<F, const std::string &>)>* = nullptr>
decltype(auto) operator|(const std::string &str, F &&converter) {
    return converter(str) | WayLib::Ranges::autoSync();
}
