#pragma once

#include <stdexcept>
#include <type_traits>
#include <functional>

namespace WayLib {
    namespace OperatorExtensions {
        template<typename OperatorType, typename OperandType, std::enable_if_t<std::is_invocable_v<OperatorType,
            OperandType>, void *>  = nullptr>
        decltype(auto) operator|(OperandType &&operand, OperatorType &&op) {
            return std::invoke(std::forward<OperatorType>(op), std::forward<OperandType>(operand));
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
