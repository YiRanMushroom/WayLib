#pragma once

#include "CRTP/inject_container_traits.hpp"

namespace WayLib {
    template<typename T>
    class Stream : public inject_container_traits<Stream, T> {
        std::vector<T> m_Data;
    public:
        Stream() = default;

        Stream(auto begin, auto end) : m_Data(begin, end) {
        }

        // start of primitive operations

        auto begin(this auto &&self) {
            return self.m_Data.begin();
        }

        auto end(this auto &&self) {
            return self.m_Data.end();
        }

        decltype(auto) add(this auto &&self, auto &&item) {
            self.m_Data.push_back(std::forward<decltype(item)>(item));
            return std::forward<decltype(self)>(self);
        }

        size_t size(this auto &&self) {
            return self.m_Data.size();
        }

        bool empty(this auto &&self) {
            return self.m_Data.empty();
        }

        decltype(auto) emplace(this auto &&self, auto &&... args) {
            self.m_Data.emplace_back(std::forward<decltype(args)>(args)...);
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) limit(this auto &&self, size_t n) {
            if (self.m_Data.size() > n) {
                self.m_Data.resize(n);
            }
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) erase(this auto &&self, auto &&... args) {
            self.m_Data.erase(std::forward<decltype(args)>(args)...);
            return std::forward<decltype(self)>(self);
        }

        std::vector<T> &getData() {
            return m_Data;
        }

        // above is the primitive operations
    };
}
