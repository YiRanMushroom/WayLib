#pragma once

#include "RuntimeException.hpp"

namespace WayLib {
    class IllegalArgumentException : public RuntimeException {
    public:
        DeclWayLibExceptionConstructors(IllegalArgumentException, "Unexpected Argument")

        [[nodiscard]] std::string exceptionType() const override {
            return "WayLib::IllegalArgumentException";
        }
    };

    class FileIOException : public RuntimeException {
    public:
        DeclWayLibExceptionConstructors(FileIOException, "File IO Error")

        [[nodiscard]] std::string exceptionType() const override {
            return "WayLib::FileIOException";
        }
    };

    class ArrayIndexOutOfBoundsException : public RuntimeException {
    public:
        DeclWayLibExceptionConstructors(ArrayIndexOutOfBoundsException, "Array Index Out Of Bounds")

        [[nodiscard]] std::string exceptionType() const override {
            return "WayLib::ArrayIndexOutOfBoundsException";
        }
    };
}
