#pragma once

#include "inject_container_traits.hpp"

#include "Macro/DefWayMacro.hpp"

namespace WayLib {

    template<template<typename...> typename Container, typename T>
    struct inject_stream_traits : public inject_container_primitive_traits_check {

        auto flatMap(_declself_, auto &&transformer) {
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

        auto runningReduce(_declself_, auto &&reducer) {
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

        auto runningFold(_declself_, auto &&init, auto &&reducer) {
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

        auto map(_declself_, auto &&transform) {
            using ResultType = std::remove_reference_t<std::invoke_result_t<decltype(transform), T> >;
            Container<ResultType> result;
            _self_.forEach([&](auto &&item) {
                result.push(transform(_self_.forward(item)));
            });
            return result;
        }

        // TransformerType: T -> std::optional<U>
        auto mapNotNull(_declself_, auto &&transformer) {
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
    };

} // namespace WayLib::Streamers

#include "Macro/UndefWayMacro.hpp"