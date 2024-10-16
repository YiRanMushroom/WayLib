#pragma once
#include <functional>
#include <vector>

namespace WayLib {
    namespace Ranges {
        inline bool alwaysSync = false;
    }

    template<typename T, typename R>
    class Range {
        mutable R m_Parent;
        mutable std::function<std::shared_ptr<std::vector<T> >(R &&)> m_Transformer;
        mutable std::shared_ptr<std::vector<T> > m_Cache;

    public:
        using value_type = T;
        using parent_type = R; // R must have a get() method

        const std::shared_ptr<std::vector<T> > &get() const {
            if (!m_Cache) {
                m_Cache = m_Transformer(std::move(m_Parent));
            }
            return m_Cache;
        }

        const std::shared_ptr<std::vector<T> > &getNoCache() const {
            return m_Cache = m_Transformer(std::move(m_Parent));
        }

        explicit Range(R &&parent, std::function<std::shared_ptr<std::vector<T> >(R &&)> transformer)
            : m_Parent(std::move(parent)), m_Transformer(std::move(transformer)) {}

        explicit Range(const R &parent, std::function<std::shared_ptr<std::vector<T> >(R &&)> transformer)
            : m_Parent(std::forward<decltype(parent)>(parent)), m_Transformer(std::move(transformer)) {}

        Range(const Range &) = default;

        Range &operator=(const Range &) = default;

        Range(Range &&other) noexcept: m_Parent(std::move(other.m_Parent)), m_Transformer(other.m_Transformer) {}

        Range &operator=(Range &&other) noexcept {
            m_Transformer = other.m_Transformer;
            m_Parent = std::move(other.m_Parent);
            return *this;
        }

        const std::function<std::shared_ptr<std::vector<T> >(R &&)> &getTransformer() const {
            return m_Transformer;
        }

        const R &getParent() const {
            return m_Parent;
        }

        virtual ~Range() = default;
    };

    template<typename T>
    class Range<T, void> {
    public:
        using value_type = T;
        using parent_type = void;
        std::function<std::shared_ptr<std::vector<T> >()> m_Transformer;
        mutable std::shared_ptr<std::vector<T> > m_Cache;

        const std::shared_ptr<std::vector<T> > &get() const {
            if (!m_Cache) {
                m_Cache = m_Transformer();
            }
            return m_Cache;
        }

        const std::shared_ptr<std::vector<T> > &getNoCache() const {
            return m_Cache = m_Transformer();
        }

        explicit Range(std::function<std::shared_ptr<std::vector<T> >()> transformer)
            : m_Transformer(std::move(transformer)) {}

        Range(const Range &) = default;

        Range &operator=(const Range &) = default;

        Range(Range &&other) noexcept: m_Transformer(other.m_Transformer) {}

        Range &operator=(Range &&other) noexcept {
            m_Transformer = other.m_Transformer;
            return *this;
        }

        const std::function<std::shared_ptr<std::vector<T> >()> &getTransformer() const {
            return m_Transformer;
        }
    };

    namespace Ranges {
        template<typename T, typename = void>
        struct IsRange : std::false_type {};

        template<typename T>
        struct IsRange<T, std::void_t<std::enable_if_t<
                    std::is_same_v<T, Range<typename T::value_type, typename T::parent_type> >,
                    void> > > : std::true_type {};

        template<typename T>
        inline constexpr bool IsRangeV = IsRange<T>::value;

        inline auto sync() {
            return [](auto &&range) -> decltype(auto) {
                range.get();
                return std::forward<decltype(range)>(range);
            };
        }

        inline auto autoSync() {
            return [](auto &&range) {
                if constexpr (!IsRangeV<std::decay_t<decltype(range)> >) {
                    return range;
                } else {
                    if (alwaysSync)
                        return sync()(std::forward<decltype(range)>(range));
                    return std::forward<decltype(range)>(range);
                }
            };
        }
    }
}

template<typename T, typename R, typename Convertor>
auto operator|(WayLib::Range<T, R> &&range, Convertor &&converter) {
    return WayLib::Ranges::autoSync()(converter(std::move(range)));
}

template<typename T, typename R, typename Convertor>
auto operator|(const WayLib::Range<T, R> &range, Convertor &&converter) {
    return WayLib::Ranges::autoSync()(converter(std::move(range)));
}

template<typename T, typename F,
    std::enable_if_t<std::is_same_v<std::invoke_result_t<F, std::vector<T> &&>, WayLib::Range<T, void> >, int>  = 0>
auto operator|(std::vector<T> &&vec, F converter) {
    return WayLib::Range<T, void>{
               [vec = std::forward<decltype(vec)>, converter = std::move(converter)] {
                   return converter(std::move(vec));
               }
           } | WayLib::Ranges::autoSync();
}

template<typename T, typename F,
    std::enable_if_t<std::is_same_v<std::invoke_result_t<F, const std::vector<T> &>, WayLib::Range<T, void> >, int>  =
            0>
auto operator|(const std::vector<T> &vec, F &&converter) {
    return WayLib::Range<T, void>{
        std::invoke(converter, vec)
    } | WayLib::Ranges::autoSync();
}
