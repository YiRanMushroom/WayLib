#pragma once
#include <functional>
#include <vector>

namespace WayLib {
    template<typename T>
    class Range {
        std::function<std::shared_ptr<std::vector<T> >()> m_Provider;

    public:
        using value_type = T;

        std::shared_ptr<std::vector<T>> get() const {
            return std::invoke(m_Provider);
        }

        explicit Range(std::function<std::shared_ptr<std::vector<T> >()> provider) : m_Provider(std::move(provider)) {}

        template<typename U>
        Range(Range<U> &&other, std::function<std::shared_ptr<std::vector<T> >
                  (Range<U> &&)> converter)
            : m_Provider([provider = std::move(other.m_Provider), converter = std::move(converter)]() {
                return std::invoke(converter, std::invoke(provider));
            }) {}

        Range(const Range &) = default;

        Range &operator=(const Range &) = default;

        Range(Range &&other) noexcept: m_Provider(other.m_Provider) {}

        Range &operator=(Range &&other) noexcept {
            m_Provider = other.m_Provider;
            return *this;
        }

        Range<T> copy() const {
            return Range<T>(m_Provider);
        }
    };

    namespace Ranges {
        struct ToRangeImplClass {
            template<typename Container>
            auto operator()(Container &&container) const {
                using T = typename std::decay_t<Container>::value_type;
                std::shared_ptr<std::vector<T> > vec = std::make_shared<std::vector<T> >();
                vec->reserve(container.size());
                for (auto &item: container) {
                    if constexpr (std::is_rvalue_reference_v<decltype(container)>) {
                        vec->push_back(std::move(item));
                    } else {
                        vec->push_back(item);
                    }
                }
                return Range<T>([vec]() {
                    return vec;
                });
            }
        };

        inline auto toRange() {
            static ToRangeImplClass instance;
            return instance;
        }

        auto collect(auto &&collector) {
            return [collector = std::forward<decltype(collector)>(collector)](auto &&range) {
                return std::invoke(collector, std::forward<decltype(range)>(range));
            };
        }

        auto forEach(auto &&visitor) {
            return [visitor = std::forward<decltype(visitor)>(visitor)](auto &&range) {
                using T = typename std::decay_t<decltype(range)>::value_type;

                return Range<T>{
                    [range = std::forward<decltype(range)>(range), visitor]() {
                        for (auto &item: *range.get()) {
                            std::invoke(visitor, item);
                        }
                        return range.get();
                    }
                };
            };
        }

        auto filter(auto predicate) {
            return [predicate = std::move(predicate)](auto &&range) {
                using T = typename std::decay_t<decltype(range)>::value_type;
                return Range<T>{
                    [range = std::forward<decltype(range)>(range), predicate]() {
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

        auto map(auto transformer) {
            return [transformer = std::move(transformer)](auto &&range) {
                using T = typename std::decay_t<decltype(range)>::value_type;
                using U = std::remove_reference_t<std::invoke_result_t<decltype(transformer), T> >;
                return Range<U>{
                    [range = std::forward<decltype(range)>(range), transformer]() {
                        std::shared_ptr<std::vector<U> > vec = std::make_shared<std::vector<U> >();

                        auto data = range.get();

                        for (auto &item: *data) {
                            vec->push_back(std::invoke(transformer, item));
                        }

                        return vec;
                    }
                };
            };
        }

        auto let(auto &&visitor) {
            return [visitor = std::forward<decltype(visitor)>(visitor)](auto &&range) -> decltype(auto) {
                visitor(std::forward<decltype(range)>(range));
                return std::forward<decltype(range)>(range);
            };
        }

        auto then(auto &&action) {
            return [action = std::forward<decltype(action)>(action)](auto &&range) -> decltype(auto) {
                action();
                return std::forward<decltype(range)>(range);
            };
        }

        inline auto evaluate() {
            return [](auto &&range) {
                return range.get();
            };
        }

        namespace Collectors {}
    }
}

template<typename T>
auto operator|(WayLib::Range<T> &&range, auto &&converter) {
    return std::invoke(converter, std::move(range));
}

template<typename T>
auto operator|(const WayLib::Range<T> &range, auto &&converter) {
    return std::invoke(converter, range);
}

template<typename T, typename F,
    std::enable_if_t<std::is_same_v<std::invoke_result_t<F, std::vector<T> &&>, WayLib::Range<T> >, int>  = 0>
auto operator|(std::vector<T> &&vec, F &&converter) {
    return WayLib::Range<T>{
        std::invoke(converter, std::move(vec))
    };
}

template<typename T, typename F,
    std::enable_if_t<std::is_same_v<std::invoke_result_t<F, const std::vector<T> &>, WayLib::Range<T> >, int>  = 0>
auto operator|(const std::vector<T> &vec, F &&converter) {
    return WayLib::Range<T>{
        std::invoke(converter, vec)
    };
}
