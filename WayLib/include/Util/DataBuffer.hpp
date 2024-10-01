#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "RuntimeException.hpp"
#include "CRTP/inject_container_traits.hpp"
#include "Macro/DefWayMacro.hpp"

namespace WayLib {
    class DataBuffer;

    template<typename T, typename _ = void>
    void WriteBufferImpl(DataBuffer &buffer, const T &data);

    template<typename T, typename _ = void>
    void ReadBufferImpl(DataBuffer &buffer, T &ref);

    class BufferOverflowException : public RuntimeException {
    public:
        DeclWayLibExceptionConstructors(BufferOverflowException, "DataBuffer Overflow")

        [[nodiscard]] std::string exceptionType() const override {
            return "WayLib::DataBuffer::BufferOverflowException";
        }
    };
}

inline WayLib::DataBuffer &&operator>>(WayLib::DataBuffer &&buffer, auto &data);

inline WayLib::DataBuffer &&operator<<(WayLib::DataBuffer &&buffer, const auto &data);

inline WayLib::DataBuffer &operator>>(WayLib::DataBuffer &buffer, auto &data);

inline WayLib::DataBuffer &operator<<(WayLib::DataBuffer &buffer, const auto &data);

namespace WayLib {
    class DataBuffer : public inject_type_converts {
        std::vector<uint8_t> m_Data;

        size_t m_ReadIndex{};

    private:
        DataBuffer(const DataBuffer &rhs) : m_Data(rhs.m_Data) {
        }

        DataBuffer &operator=(const DataBuffer &rhs) {
            m_Data = rhs.m_Data;
            m_ReadIndex = 0;
            return *this;
        }

    public:
        DataBuffer(DataBuffer &&rhs) noexcept = default;

        DataBuffer &operator=(DataBuffer &&rhs) noexcept = default;

        DataBuffer copyWithReadIndex(_declself_) {
            DataBuffer copy(_self_);
            copy.m_ReadIndex = _self_.m_ReadIndex;
            return copy;
        }

        DataBuffer copyNoReadIndex(_declself_) {
            DataBuffer copy(_self_);
            copy.m_ReadIndex = 0;
            return copy;
        }

        decltype(auto) simpleAppend(_declself_, const void *data, size_t size) {
            _self_.m_Data.resize(_self_.m_Data.size() + size);
            std::memcpy(_self_.m_Data.data() + _self_.m_Data.size() - size, data, size);
            return _self_;
        }

        template<typename T>
        decltype(auto) simpleAppend(_declself_, const T *t) {
            constexpr size_t size = sizeof(T);
            _self_.simpleAppend(reinterpret_cast<const void *>(t), size);
            return _self_;
        }

        DataBuffer() = default;

        template<typename T>
        decltype(auto) write(_declself_, const T &data) {
            WriteBufferImpl(self, _forward_(data));
            return _self_;
        }

        template<typename T>
        decltype(auto) read(_declself_, T &t) {
            ReadBufferImpl(self, t);
            return _self_;
        }

    public:
        template<typename T>
        T read() {
            T t;
            read(t);
            return t;
        }

        decltype(auto) pushBack(_declself_, auto &&... args) {
            (_self_.write(_forward_(args)), ...);
            return _self_;
        }

        decltype(auto) popFront(_declself_, auto &... args) {
            (_self_.read(args), ...);
            return _self_;
        }

        decltype(auto) operator[](_declself_, size_t index) {
            return _self_.m_Data[index];
        }

        decltype(auto) getData(_declself_) {
            return _self_.m_Data;
        }

        std::pair<void *, size_t> getRawData(_declself_) {
            return std::make_pair(_self_.m_Data.data(), _self_.m_Data.size() * sizeof(uint8_t));
        }

        void checkSize(_declself_, size_t size) {
            if (_self_.m_ReadIndex + size > _self_.m_Data.size()) {
                throw BufferOverflowException("DataBuffer overflow after checking, requested size: " + std::to_string(size) +
                                              ", available size: " + std::to_string(
                                                  _self_.m_Data.size() - _self_.m_ReadIndex) + std::string(", read index: ") +
                                                      std::to_string(_self_.m_ReadIndex))
                .pushOptionalData("DataBuffer", std::make_shared<DataBuffer>(_self_.move()));
            }
        }

        size_t &getReadIndex(_declself_) {
            return _self_.m_ReadIndex;
        }

        void writeToStream(_declself_, std::ostream &os) {
            os.write(reinterpret_cast<const char *>(_self_.m_Data.data()), _self_.m_Data.size());
        }
    };

    inline DataBuffer CreateBufferFromStream(std::istream &is) {
        DataBuffer buffer;
        std::stringstream ss;
        ss << is.rdbuf();
        buffer.simpleAppend(ss.str().data(), ss.str().size());
        return buffer;
    }

    template<typename T, typename _>
    void WriteBufferImpl(DataBuffer &buffer, const T &data) {
        buffer.simpleAppend(&data);
    }

    template<typename T, typename _>
    void ReadBufferImpl(DataBuffer &buffer, T &ref) {
        buffer.checkSize(sizeof(T));
        ref = reinterpret_cast<T &>(buffer.getData()[buffer.getReadIndex()]);
        buffer.getReadIndex() += sizeof(T);
    }

    // std::string

    template<>
    inline void WriteBufferImpl<const char *, void>(DataBuffer &buffer, const char *const &data) = delete;

    template<>
    inline void WriteBufferImpl(DataBuffer &buffer, char *const &data) = delete;

    // please use std::string instead of char*

    template<>
    inline void ReadBufferImpl(DataBuffer &buffer, const char *&ref) = delete;

    template<>
    inline void ReadBufferImpl(DataBuffer &buffer, char *&ref) = delete;

    // please use std::string instead of char*

    template<>
    inline void WriteBufferImpl(DataBuffer &buffer, const std::string &data) {
        buffer.write(data.size());
        buffer.simpleAppend(data.data(), data.size());
    }

    template<>
    inline void ReadBufferImpl(DataBuffer &buffer, std::string &ref) {
        auto size = buffer.read<decltype(std::string{}.size())>();
        ref.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            ref.push_back(buffer.read<char>());
        }
    }

    // vector<T>

    template<typename T>
    inline void WriteBufferImpl(DataBuffer &buffer, const std::vector<T> &data) {
        buffer.write(data.size());
        for (auto &&el: data) {
            buffer.write(el);
        }
    }

    template<typename T>
    inline void ReadBufferImpl(DataBuffer &buffer, std::vector<T> &ref) {
        auto size = buffer.read<decltype(std::vector<T>{}.size())>();
        ref.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            ref.emplace_back(buffer.read<T>());
        }
    }

    // std::pair

    template<typename T1, typename T2>
    inline void WriteBufferImpl(DataBuffer &buffer, const std::pair<T1, T2> &data) {
        buffer.write(data.first);
        buffer.write(data.second);
    }

    template<typename T1, typename T2>
    inline void ReadBufferImpl(DataBuffer &buffer, std::pair<T1, T2> &ref) {
        buffer >> ref.first;
        buffer >> ref.second;
    }

    // std::tuple

    template<typename... Ts>
    inline void WriteBufferImpl(DataBuffer &buffer, const std::tuple<Ts...> &data) {
        std::apply([&buffer](auto &&... args) {
            (buffer.write(args), ...);
        }, data);
    }

    template<typename... Ts>
    inline void ReadBufferImpl(DataBuffer &buffer, std::tuple<Ts...> &ref) {
        std::apply([&buffer](auto &... args) {
            ((buffer >> args), ...);
        }, ref);
    }

    // std::map
    template<typename K, typename V>
    inline void WriteBufferImpl(DataBuffer &buffer, const std::map<K, V> &data) {
        buffer.write(data.size());
        for (auto &&[key, value]: data) {
            buffer.write(key);
            buffer.write(value);
        }
    }

    template<typename K, typename V>
    inline void ReadBufferImpl(DataBuffer &buffer, std::map<K, V> &ref) {
        auto size = buffer.read<decltype(std::map<K, V>{}.size())>();
        for (size_t i = 0; i < size; ++i) {
            K key = buffer.read<K>();
            V value = buffer.read<V>();
            ref[std::move(key)] = std::move(value);
        }
    }

    // std::unordered_map

    template<typename K, typename V>
    inline void WriteBufferImpl(DataBuffer &buffer, const std::unordered_map<K, V> &data) {
        buffer.write(data.size());
        for (auto &&[key, value]: data) {
            buffer.write(key);
            buffer.write(value);
        }
    }

    template<typename K, typename V>
    inline void ReadBufferImpl(DataBuffer &buffer, std::unordered_map<K, V> &ref) {
        auto size = buffer.read<decltype(std::unordered_map<K, V>{}.size())>();
        for (size_t i = 0; i < size; ++i) {
            K key = buffer.read<K>();
            V value = buffer.read<V>();
            ref[std::move(key)] = std::move(value);
        }
    }

    // std::set

    template<typename T>
    inline void WriteBufferImpl(DataBuffer &buffer, const std::set<T> &data) {
        buffer.write(data.size());
        for (auto &&el: data) {
            buffer.write(el);
        }
    }

    template<typename T>
    inline void ReadBufferImpl(DataBuffer &buffer, std::set<T> &ref) {
        auto size = buffer.read<decltype(std::set<T>{}.size())>();
        for (size_t i = 0; i < size; ++i) {
            ref.insert(buffer.read<T>());
        }
    }

    // std::unordered_set

    template<typename T>
    inline void WriteBufferImpl(DataBuffer &buffer, const std::unordered_set<T> &data) {
        buffer.write(data.size());
        for (auto &&el: data) {
            buffer.write(el);
        }
    }

    template<typename T>
    inline void ReadBufferImpl(DataBuffer &buffer, std::unordered_set<T> &ref) {
        auto size = buffer.read<decltype(std::unordered_set<T>{}.size())>();
        for (size_t i = 0; i < size; ++i) {
            ref.insert(buffer.read<T>());
        }
    }

    // std::array

    template<typename T, size_t N>
    inline void WriteBufferImpl(DataBuffer &buffer, const std::array<T, N> &data) {
        for (auto &&el: data) {
            buffer.write(el);
        }
    }

    template<typename T, size_t N>
    inline void ReadBufferImpl(DataBuffer &buffer, std::array<T, N> &ref) {
        for (size_t i = 0; i < N; ++i) {
            ref[i] = buffer.read<T>();
        }
    }

    // std::unique_ptr

    template<typename T>
    inline void WriteBufferImpl(DataBuffer &buffer, const std::unique_ptr<T> &data) {
        buffer.write(data.get());
    }

    template<typename T>
    inline void ReadBufferImpl(DataBuffer &buffer, std::unique_ptr<T> &ref) {
        ref = std::make_unique<T>(buffer.read<T>());
    }

    // std::shared_ptr

    template<typename T>
    inline void WriteBufferImpl(DataBuffer &buffer, const std::shared_ptr<T> &data) {
        buffer.write(data.get());
    }

    template<typename T>
    inline void ReadBufferImpl(DataBuffer &buffer, std::shared_ptr<T> &ref) {
        ref = std::make_shared<T>(buffer.read<T>());
    }
}

inline WayLib::DataBuffer &operator<<(WayLib::DataBuffer &buffer, const auto &data) {
    buffer.write(_forward_(data));
    return buffer;
}

inline WayLib::DataBuffer &&operator<<(WayLib::DataBuffer &&buffer, const auto &data) {
    buffer.write(_forward_(data));
    return std::move(buffer);
}

inline WayLib::DataBuffer &operator>>(WayLib::DataBuffer &buffer, auto &data) {
    buffer.read(data);
    return buffer;
}

inline WayLib::DataBuffer &&operator>>(WayLib::DataBuffer &&buffer, auto &data) {
    buffer.read(data);
    return std::move(buffer);
}

// operators
#include "Macro/UndefWayMacro.hpp"
