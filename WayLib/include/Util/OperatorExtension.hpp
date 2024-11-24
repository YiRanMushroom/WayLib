#pragma once

#include <stdexcept>
#include <type_traits>
#include <functional>
#include <tuple>

namespace WayLib {
    namespace OperatorExtensions {
        template<typename OperatorType, typename OperandType, std::enable_if_t<std::is_invocable_v<OperatorType,
                OperandType>, void *>  = nullptr>
        decltype(auto) operator|(OperandType &&operand, OperatorType &&op) {
            return std::invoke(std::forward<OperatorType>(op), std::forward<OperandType>(operand));
        }

        namespace Util {
            namespace Impl {
                template<typename T, typename Function>
                decltype(auto) InvokePipeChain(T &&arg, Function &&function) {
                    return function(std::forward<T>(arg));
                }

                template<typename T, typename First, typename... Function>
                decltype(auto) InvokePipeChain(T &&arg, First &&first, Function &&... functions) {
                    using namespace OperatorExtensions;
                    return InvokePipeChain(std::invoke(std::forward<First>(first),
                                                       std::forward<T>(arg)),
                                           std::forward<Function>(functions)...);
                }

                template<typename Tuple, size_t... indices>
                auto PipeTupleImpl(Tuple &&tup, std::index_sequence<indices...> seq) {
                    return [seq, tup = std::forward<Tuple>(tup)](auto &&arg) -> decltype(auto) {
                        return InvokePipeChain(arg, std::get<indices>(tup)...);
                    };
                }
            }


            template<typename FirstFunc, typename... Function>
            auto PipeRef(FirstFunc &&firstFunc, Function &&... functions) {
                return [firstFunc = std::forward_as_tuple(std::forward<FirstFunc>(firstFunc)), functions =
                std::forward_as_tuple(std::forward<Function>(functions)...)]
                        (auto &&... args) -> decltype(auto) {
                    return std::invoke(Impl::PipeTupleImpl(std::move(functions),
                                                           std::make_index_sequence<sizeof...(Function)>()),
                                       std::invoke(std::move(std::get<0>(firstFunc)),
                                                   std::forward<decltype(args)>(args)...));
                };
            }

            template<typename FirstFunc, typename... Function>
            auto PipeForward(FirstFunc &&firstFunc, Function &&... functions) {
                return [firstFunc = std::forward<FirstFunc>(firstFunc), functions =
                std::make_tuple(std::forward<Function>(functions)...)]
                        (auto &&... args) -> decltype(auto) {
                    return Impl::PipeTupleImpl(functions, std::make_index_sequence<sizeof...(Function)>())(
                            std::invoke(firstFunc,
                                        std::forward<decltype(args)>(args)...));
                };
            }
        }
    }

    namespace MemberFunctionExtensions {
        inline auto get() {
            return [](auto &&operand) -> decltype(auto) {
                return operand.get();
            };
        }

        inline auto getOrDefault() {
            return [](auto &&operand) -> decltype(auto) {
                using ResultType = decltype(operand.get());
                if (operand.has_value()) {
                    return operand.get();
                }
                return ResultType{};
            };
        }

        inline auto getOrThrow() {
            return [](auto &&operand) -> decltype(auto) {
                if (operand.has_value()) {
                    return operand.get();
                }
                throw std::runtime_error("getOrThrow: No value present");
            };
        }

        inline auto value() {
            return [](auto &&operand) -> decltype(auto) {
                return operand.value();
            };
        }

        template<typename DefaultValue>
        inline auto valueOr(DefaultValue &&defaultValue) {
            return [&](auto &&operand) -> decltype(auto) {
                return operand.value_or(std::forward<DefaultValue>(defaultValue));
            };
        }

        inline auto dereference() {
            return [](auto &&operand) -> decltype(auto) {
                return *operand;
            };
        }

        template<typename F>
        inline auto apply(F &&f) {
            return std::forward<F>(f);
        }
    }
}
