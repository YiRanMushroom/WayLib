#pragma once

#include <string>
#include <utility>
#include <source_location>
#include <stacktrace>
#include <unordered_map>
#include <any>
#include <sstream>

#include "CRTP/inject_container_traits.hpp"
#include "Macro/DefWayMacro.hpp"

namespace WayLib {
    class RuntimeException : public inject_type_converts, public std::exception {
        std::unordered_map<std::string, std::any> m_OptionalData;
        std::string m_Message;
        std::source_location m_Location;
        std::stacktrace m_Stacktrace;
        mutable std::string m_MessageCache;

    public:
        explicit RuntimeException(std::string message,
                                  const std::source_location &location = std::source_location::current(),
                                  std::stacktrace stacktrace = std::stacktrace::current()) : m_Message(std::move(
                message)),
            m_Location(location), m_Stacktrace(std::move(stacktrace)) {
        }

        decltype(auto) getOptionalData(_declself_) {
            return _self_.m_OptionalData;
        }

        decltype(auto) pushOptionalData(_declself_, const char *key, auto &&value) {
            _self_.m_OptionalData[key] = std::forward<decltype(value)>(value);
            return _self_;
        }

        [[nodiscard]] virtual std::string exceptionType() const {
            return "WayLib::Exception";
        }

        [[nodiscard]] const char *what() const override {
            std::stringstream ss;

            ss << '[' << exceptionType() << "]: " << m_Message << std::endl;
            ss << "Exception Threw At: " << m_Location.file_name() << ":" << m_Location.line() << std::endl;
            ss << "In Function: " << m_Location.function_name() << std::endl;
            ss << "Stacktrace: " << std::endl;
            for (auto &&frame: m_Stacktrace) {
                if (frame.source_line())
                    ss << "  at: " << frame.description() << " in file:" << frame.source_file() << " line: " <<
                            frame.source_line() << " [" << frame.native_handle() << ']' << std::endl;
            }

            m_MessageCache = ss.str();

            return m_MessageCache.c_str();
        }

        ~RuntimeException() override = default;
    };
}

#define DeclWayLibExceptionConstructors(ExceptionType, DefaultMessage) \
ExceptionType() : RuntimeException(DefaultMessage) {}\
\
explicit ExceptionType(const std::string &msg,\
    const std::source_location& location = std::source_location::current(),\
    const std::stacktrace& trace = std::stacktrace::current()) : RuntimeException(msg, location, trace) {}\
\
explicit ExceptionType(std::string &&msg,\
    const std::source_location& location = std::source_location::current(),\
    const std::stacktrace& trace = std::stacktrace::current()) : RuntimeException(std::move(msg), location, trace) {}

#include "Macro/UndefWayMacro.hpp"
