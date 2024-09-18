#pragma once

#include <cstdint>
#include <vector>

class DataBuffer {
    std::vector<uint8_t> m_Data;

    void pushBackImpl(auto &&arg) {
        m_Data.insert(m_Data.end(), &arg, &arg + 1);
    }

public:
    template<typename T>
    DataBuffer(T *data, size_t count) {
        T *endIt = data + count;
        m_Data.insert(m_Data.end(), reinterpret_cast<uint8_t *>(data), reinterpret_cast<uint8_t *>(endIt));
    }

    DataBuffer() = default;

    decltype(auto) push_back(this auto &&self, auto &&... args) {
        (self.pushBackImpl(std::forward<decltype(args)>(args)), ...);
        return std::forward<decltype(self)>(self);
    }

    decltype(auto) operator[](this auto &&self, size_t index) {
        return std::forward<decltype(self)>(self).m_Data[index];
    }
};
