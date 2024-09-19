#pragma once

#include <string>
#include <utility>
#include <vector>
#include <source_location>
#include <stacktrace>
#include <unordered_map>
#include <any>
#include <iostream>

#include "Macro/DefWayMacro.hpp"
#include "Macro/DefWayMacro.hpp"

namespace WayLib {
    inline bool isNewLine(std::ostream& os) {
        return os.rdbuf()->sputc('\n') == '\n';
    }

    class RichException {
        std::unordered_map<std::string, std::any> m_OptionalData;
        std::string m_Message;
        std::source_location m_Location;
        std::stacktrace m_Stacktrace;

    public:
        explicit RichException(std::string message,
                               const std::source_location &location = std::source_location::current(),
                               std::stacktrace stacktrace = std::stacktrace::current()) : m_Message(std::move(message)),
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
            return "Util::WayLib::Exception";
        }

        [[nodiscard]] virtual std::ostream &getOs() const {
            return std::cerr;
        }

        virtual void what() const {
            auto &os = getOs();
            if (isNewLine(os)) {
                os << std::endl;
            }
            os << '['<< exceptionType() << "]: " << m_Message << std::endl;
            os << "Error Occurred At: " << m_Location.file_name() << ":" << m_Location.line() << std::endl;
            os << "When Calling Function: " << m_Location.function_name() << std::endl;
            os << "Stacktrace: " << std::endl;
            for (auto &&frame: m_Stacktrace) {
                os << "  @At: " << frame << std::endl;
            }
        }

        virtual ~RichException() = default;
    };
}

#include "Macro/UndefWayMacro.hpp"
