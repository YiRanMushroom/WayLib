#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace WayLib {
    template<typename T>
    class Stream {
        std::vector<T> m_Data;

    public:
        Stream() = default;

        std::vector<T> &getData() {
            return m_Data;
        }

        template<typename IIterator>
        Stream(IIterator begin, IIterator end) : m_Data(begin, end) {}

        auto begin() {
            return m_Data.begin();
        }

        auto end() {
            return m_Data.end();
        }

        template<typename ICollector>
        auto collect(ICollector collector) {
            return collector(begin(), end());
        }

        template<typename IFilter>
        decltype(auto) filter(this auto &&self, IFilter filter) {
            std::vector<T> result;
            for (auto &&item: self.m_Data) {
                if (filter(item)) {
                    result.push_back(std::move(item));
                }
            }
            self.m_Data = std::move(result);
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) distinct(this auto &&self) {
            std::unordered_set<T> set;
            self.filter([&](const T &item) {
                if (set.contains(item)) {
                    return false;
                }
                set.insert(item);
                return true;
            });
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) forEach(this auto &&self, auto &&action) {
            for (auto &&item: self.m_Data) {
                action(item);
            }
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) sorted(this auto &&self) {
            std::sort(self.m_Data.begin(), self.m_Data.end());
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) sortedDesc(this auto &&self) {
            std::sort(self.m_Data.begin(), self.m_Data.end(), std::greater<>());
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) sortedBy(this auto &&self, auto &&transform) {
            std::sort(self.m_Data.begin(), self.m_Data.end(), [&](const T &a, const T &b) {
                return transform(a) < transform(b);
            });
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) sortedByDesc(this auto &&self, auto &&transform) {
            std::sort(self.m_Data.begin(), self.m_Data.end(), [&](const T &a, const T &b) {
                return transform(a) > transform(b);
            });
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) sortedWith(this auto &&self, auto &&comparator) {
            std::sort(self.m_Data.begin(), self.m_Data.end(), std::forward<decltype(comparator)>(comparator));
            return std::forward<decltype(self)>(self);
        }

        template<typename TransformerType>
        auto map(TransformerType &&transform) {
            using ResultType = std::invoke_result_t<TransformerType, T>;
            Stream<ResultType> result;
            for (auto &&item: this->m_Data) {
                result.getData().push_back(transform(std::move(item)));
            }
            return result;
        }

        template<typename TransformerType> // TransformerType: T -> std::optional<U>
        auto mapNotNull(TransformerType &&transformer) {
            using ResultType = std::remove_reference_t<decltype(*std::declval<std::invoke_result_t<TransformerType, T> >())>
                    ;
            Stream<ResultType> result;
            for (auto &&item: this->m_Data) {
                auto transformed = transformer(std::move(item));
                if (transformed.has_value()) {
                    result.getData().push_back(std::move(transformed.value()));
                }
            }
            return result;
        }

        decltype(auto) let(this auto &&self, auto &&action) {
            action(self.m_Data);
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) then(this auto &&self, auto &&action) {
            action();
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) apply(auto &&action) {
            return action(this->m_Data);
        }

        template<typename Mapper>
        auto groupedDistinctBy(Mapper &&mapper) {
            using PairType = std::invoke_result_t<Mapper, T>;
            using K = typename PairType::first_type;
            using V = typename PairType::second_type;
            std::unordered_map<K, V> result;
            for (auto &&item: this->m_Data) {
                auto [key, value] = std::forward<Mapper>(mapper)(std::move(item));
                result[key] = value;
            }
            return result;
        }

        template<typename Mapper>
        auto groupedMultipleBy(Mapper &&mapper) {
            // use std::unordered_multimap
            using PairType = std::invoke_result_t<Mapper, T>;
            using K = typename PairType::first_type;
            using V = typename PairType::second_type;
            std::unordered_multimap<K, V> result;
            for (auto &&item: this->m_Data) {
                auto [key, value] = std::forward<Mapper>(mapper)(std::move(item));
                result.emplace(key, value);
            }
            return result;
        }

        template<typename U>
        U fold(U init, auto &&reducer) {
            U result = std::move(init);
            for (auto &&item: this->m_Data) {
                result = std::forward<decltype(reducer)>(reducer)(std::move(result), std::move(item));
            }
            return result;
        }

        template<typename U>
        Stream<U> runningFold(U init, auto &&reducer) {
            U result = std::move(init);
            Stream<U> stream;
            stream.getData().push_back(result);
            for (auto &&item: this->m_Data) {
                result = std::forward<decltype(reducer)>(reducer)(std::move(result), std::move(item));
                stream.getData().push_back(result);
            }
            return stream;
        }

        std::optional<T> reduce(auto &&reducer) {
            if (this->m_Data.empty()) {
                return std::nullopt;
            }
            T result = this->m_Data[0];
            for (size_t i = 1; i < this->m_Data.size(); ++i) {
                result = std::forward<decltype(reducer)>(reducer)(std::move(result), std::move(this->m_Data[i]));
            }
            return result;
        }

        decltype(auto) runningReduce(this auto &&self, auto &&reducer) {
            if (self.m_Data.empty()) {
                return self;
            }
            T result = self.m_Data[0];
            for (size_t i = 1; i < self.m_Data.size(); ++i) {
                result = std::forward<decltype(reducer)>(reducer)(std::move(result), std::move(self.m_Data[i]));
                self.m_Data[i] = result;
            }
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) join(this auto &&self, auto &&other) {
            // other should be Stream<T>, or other iterable object
            self.m_Data.insert(self.m_Data.end(), other.m_Data.begin(), other.m_Data.end());
            return std::forward<decltype(self)>(self);
        }

        auto flatMap(auto &&transformer) {
            using ResultType = std::remove_reference_t<decltype(*std::declval<
                std::invoke_result_t<decltype(transformer), T> >().first)>;
            Stream<ResultType> result;
            for (auto &&item: this->m_Data) {
                auto [begin, end] = std::forward<decltype(transformer)>(transformer)(std::move(item));
                result.getData().insert(result.getData().end(), begin, end);
            }
            return result;
        }

        decltype(auto) limit(this auto &&self, size_t n) {
            if (self.m_Data.size() > n) {
                self.m_Data.resize(n);
            }
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) skip(this auto &&self, size_t n) {
            if (self.m_Data.size() > n) {
                self.m_Data.erase(self.m_Data.begin(), self.m_Data.begin() + n);
            }
            return std::forward<decltype(self)>(self);
        }

        auto partition(this auto &&self, auto &&predicate) {
            Stream<T> result;
            Stream<T> other;
            for (auto &&item: self.m_Data) {
                if (predicate(item)) {
                    result.getData().push_back(std::move(item));
                } else {
                    other.getData().push_back(std::move(item));
                }
            }
            return std::make_pair(result, other);
        }

        template<typename Comparator = std::less<> >
        decltype(auto) min(this auto &&self, Comparator comparator = Comparator()) {
            if (self.m_Data.empty()) {
                return std::nullopt;
            }
            return *std::min_element(self.m_Data.begin(), self.m_Data.end(), std::forward<Comparator>(comparator));
        }

        template<typename Comparator = std::less<> >
        decltype(auto) max(this auto &&self, Comparator comparator = Comparator()) {
            if (self.m_Data.empty()) {
                return std::nullopt;
            }
            return *std::max_element(self.m_Data.begin(), self.m_Data.end(), std::forward<Comparator>(comparator));
        }

        bool anyMatch(auto &&predicate) {
            return std::any_of(this->m_Data.begin(), this->m_Data.end(), std::forward<decltype(predicate)>(predicate));
        }

        bool allMatch(auto &&predicate) {
            return std::all_of(this->m_Data.begin(), this->m_Data.end(), std::forward<decltype(predicate)>(predicate));
        }

        bool noneMatch(auto &&predicate) {
            return std::none_of(this->m_Data.begin(), this->m_Data.end(), std::forward<decltype(predicate)>(predicate));
        }

        auto findFirst(auto &&predicate) {
            auto it = std::find_if(this->m_Data.begin(), this->m_Data.end(), std::forward<decltype(predicate)>(predicate));
            if (it == this->m_Data.end()) {
                return std::nullopt;
            }
            return *it;
        }

        auto findAny(auto &&predicate) {
            return findFirst(std::forward<decltype(predicate)>(predicate));
        }

        decltype(auto) forEachIndexed(this auto &&self, auto &&action) {
            size_t index = 0;
            for (auto &&item: self.m_Data) {
                action(index++, item);
            }
            return std::forward<decltype(self)>(self);
        }
    };
}