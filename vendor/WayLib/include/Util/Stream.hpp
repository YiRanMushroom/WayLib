#pragma once

#include "CRTP/inject_container_traits.hpp"
#include "Macro/DefWayMacro.hpp"

namespace WayLib {
    template<typename T>
    class Stream : public inject_container_traits<Stream, T> {
        std::vector<T> m_Data;

        template<template<typename...> typename, typename>
        friend struct inject_container_traits;

    public:
        Stream<T>() = default;

        Stream<T>(auto begin, auto end) : m_Data(begin, end) {}

        Stream<T>(const Stream &) = delete;

        Stream<T>(Stream &&) = default;

        Stream<T> &operator=(const Stream &) = delete;

        Stream<T> &operator=(Stream &&) = default;

        // start of primitive operations

        auto begin(_declself_) {
            return _self_.m_Data.begin();
        }

        auto end(_declself_) {
            return _self_.m_Data.end();
        }

    private:
        decltype(auto) push(_declself_, auto &&item) {
            _self_.m_Data.push_back(std::forward<decltype(item)>(item));
            return _self_;
        }

    public:
        size_t size(_declself_) {
            return _self_.m_Data.size();
        }

        bool empty(_declself_) {
            return _self_.m_Data.empty();
        }

        decltype(auto) emplace(_declself_, auto &&... args) {
            _self_.m_Data.emplace_back(_forward_(args)...);
            return _self_;
        }

        decltype(auto) resize(_declself_, size_t n) {
            if (_self_.m_Data.size() > n) {
                _self_.m_Data.resize(n);
            }
            return _self_;
        }

        decltype(auto) erase(_declself_, auto &&... args) {
            _self_.m_Data.erase(_forward_(args)...);
            return _self_;
        }

        decltype(auto) setData(_declself_, std::vector<T> &&data) {
            _self_.m_Data = std::move(data);
            return _self_;
        }

        decltype(auto) setData(_declself_, const std::vector<T> &data) {
            _self_.m_Data = data;
            return _self_;
        }

        decltype(auto) operator[](_declself_, size_t index) {
            return _self_.m_Data[index];
        }

        // above is the primitive operations

        decltype(auto) getData(_declself_) {
            return _self_.m_Data;
        }

        decltype(auto) pushBack(_declself_, auto &&item) {
            return _self_.push(_forward_(item));
        }
    };
}

#include "Macro/UndefWayMacro.hpp"
