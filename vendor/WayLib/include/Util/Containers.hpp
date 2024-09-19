#pragma once
#include <algorithm>
#include <memory>
#include "DataBuffer.hpp"

#include "Stream.hpp"

namespace WayLib {
    template<size_t N>
    class StringLiteral {
    public:
        constexpr StringLiteral(const char (&tar)[N]) {
            std::copy(tar, tar + N, str);
        }

        constexpr const char *get() const {
            return str;
        }

        constexpr size_t size() const {
            return N;
        }

        constexpr char operator[](size_t index) const {
            return str[index];
        }

        char str[N];
    };

    template<typename T>
    class DLList : public inject_container_traits<DLList, T> {
        class Node;
        std::shared_ptr<Node> m_Head;
        std::weak_ptr<Node> m_Tail;

        size_t m_Size{};

        friend class Node;

    public:
        using value_type = T;

        std::shared_ptr<Node> getHead() const {
            return m_Head;
        }

        std::shared_ptr<Node> getTail() const {
            return m_Tail.lock();
        }

    public:
        // primitive operations for CRTP
        [[nodiscard]] size_t size() const {
            return m_Size;
        }

        // begin, end, erase
        decltype(auto) add(this auto &&self, auto &&item) {
            self.insertBack(std::make_shared<T>(std::forward<decltype(item)>(item)));
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) emplace(this auto &&self, auto &&... args) {
            self.insertBack(std::make_shared<T>(std::forward<decltype(args)>(args)...));
            return std::forward<decltype(self)>(self);
        }

        decltype(auto) limit(this auto &&self, size_t n) {
            if (self.size() > n) {
                auto it = self.begin();
                std::advance(it, n);
                self.erase(it, self.end());
            }
        }

        decltype(auto) setData(this auto &&self, DLList &&data) {
            self = std::move(data);
        }

    private:
        class Node {
            std::shared_ptr<Node> m_Next;
            std::weak_ptr<Node> m_Prev;
            std::shared_ptr<T> m_Value;
            DLList<T> *m_List;

            friend class DLList;

        public:
            explicit Node(DLList<T> *list) : m_List{list} {
                ++(m_List->m_Size);
            }

            std::shared_ptr<T> &getPtr() {
                return m_Value;
            }

            const std::shared_ptr<T> &getPtr() const {
                return m_Value;
            }

            T &operator*() {
                return *m_Value;
            }

            const T &operator*() const {
                return *m_Value;
            }

            T &getValue() {
                return *m_Value;
            }

            const T &getValue() const {
                return *m_Value;
            }

            std::shared_ptr<T> pop() {
                auto node = getThis();
                if (auto prev = m_Prev.lock()) {
                    prev->m_Next = m_Next;
                } else {
                    m_List->m_Head = m_Next;
                }

                if (m_Next) {
                    m_Next->m_Prev = m_Prev;
                } else {
                    m_List->m_Tail = m_Prev.lock();
                }

                --(m_List->m_Size);

                return node->m_Value;
            }

        private:
            std::shared_ptr<Node> getThis() const {
                if (m_Prev.lock()) {
                    return m_Prev.lock()->m_Next;
                } else {
                    return m_List->m_Head;
                }
            }

        public:
            std::shared_ptr<Node> share() const {
                return getThis();
            }

            std::shared_ptr<Node> getNext() const {
                return m_Next;
            }

            std::shared_ptr<Node> getPrev() const {
                return m_Prev.lock();
            }

            std::shared_ptr<Node> insertAfter(const std::shared_ptr<T> &value) {
                auto newNode = std::make_shared<Node>(m_List);
                newNode->m_Value = value;
                newNode->m_Next = m_Next;
                newNode->m_Prev = getThis();
                m_Next = newNode;
                if (newNode->m_Next) {
                    newNode->m_Next->m_Prev = newNode;
                } else {
                    m_List->m_Tail = newNode;
                }

                return newNode;
            }

            std::shared_ptr<Node> insertBefore(const std::shared_ptr<T> &value) {
                auto newNode = std::make_shared<Node>(m_List);
                newNode->m_Value = value;
                newNode->m_Prev = m_Prev;
                newNode->m_Next = getThis();
                if (auto prev = m_Prev.lock()) {
                    prev->m_Next = newNode;
                } else {
                    m_List->m_Head = newNode;
                }
                m_Prev = newNode;

                return newNode;
            }

            std::shared_ptr<Node> emplaceAfter(auto &&... args) {
                return insertAfter(std::make_shared<T>(std::forward<decltype(args)>(args)...));
            }

            std::shared_ptr<Node> emplaceBefore(auto &&... args) {
                return std::make_shared<T>(std::forward<decltype(args)>(args)...);
            }

            ~Node() = default;
        };

    public:
        std::shared_ptr<Node> insertFront(const std::shared_ptr<T> value) {
            if (this->size()) {
                return this->m_Head->insertBefore(value);
            } else {
                this->m_Head = std::make_shared<Node>(this);
                this->m_Head->m_Value = value;
                this->m_Tail = this->m_Head;
                return this->m_Head;
            }
        }

        std::shared_ptr<Node> insertBack(const std::shared_ptr<T> value) {
            if (this->size()) {
                return this->getTail()->insertAfter(value);
            } else {
                this->m_Head = std::make_shared<Node>(this);
                this->m_Head->m_Value = value;
                this->m_Tail = this->m_Head;
                return this->m_Head;
            }
        }

        std::shared_ptr<Node> emplaceFront(auto &&... args) {
            return insertFront(std::make_shared<T>(std::forward<decltype(args)>(args)...));
        }

        std::shared_ptr<Node> emplaceBack(auto &&... args) {
            return insertBack(std::make_shared<T>(std::forward<decltype(args)>(args)...));
        }

        std::shared_ptr<T> popFront() {
            if (this->size()) {
                return this->m_Head->pop();
            }
            return nullptr;
        }

        std::shared_ptr<T> popBack() {
            if (this->size()) {
                return this->getTail()->pop();
            }
            return nullptr;
        }

        [[nodiscard]] bool empty() const {
            return !m_Size;
        }

        // Iterator
        class Iterator {
            std::shared_ptr<Node> m_Node;

        public:
            explicit Iterator(const std::shared_ptr<Node> &node) : m_Node{node} {}

            Iterator() = default;

            operator bool() const {
                return m_Node;
            }

            T &operator*() {
                return this->getValue();
            }

            const T &operator*() const {
                return this->getValue();
            }

            T &getValue() {
                return m_Node->getValue();
            }

            const T &getValue() const {
                return m_Node->getValue();
            }

            std::shared_ptr<Node> getNode() const {
                return m_Node;
            }

            Node *operator->() const {
                return m_Node.get();
            }

            bool operator==(const Iterator &other) const {
                return m_Node.get() == other.m_Node.get();
            }

            bool operator!=(const Iterator &other) const {
                return !(*this == other);
            }

            operator T &() {
                return *m_Node->getPtr();
            }

            operator const T &() const {
                return *m_Node->getPtr();
            }

            operator std::shared_ptr<T> &() {
                return m_Node->getPtr();
            }

            operator const std::shared_ptr<T> &() const {
                return m_Node->getPtr();
            }

            Iterator &operator++() {
                m_Node = m_Node->getNext();
                return *this;
            }

            Iterator operator++(int) {
                auto copy = *this;
                ++(*this);
                return copy;
            }

            Iterator &operator--() {
                m_Node = m_Node->getPrev();
                return *this;
            }

            Iterator operator--(int) {
                auto copy = *this;
                --(*this);
                return copy;
            }
        };

        Iterator begin() const {
            return Iterator{this->getHead()};
        }

        Iterator end() const {
            return Iterator{nullptr};
        }

        Iterator rbegin() const {
            return Iterator{this->getTail()};
        }

        Iterator rend() const {
            return Iterator{nullptr};
        }

        static void erase(const Iterator &it) {
            it.getNode()->remove();
        }

        void erase(const Iterator &begin, const Iterator &end) {
            for (auto it = begin; it != end; ++it) {
                erase(it);
            }
        }

        void clear() {
            m_Head.reset();
            m_Tail.reset();
            m_Size = 0;
        }

        DLList() = default;

        DLList(std::initializer_list<std::shared_ptr<T> > list) {
            for (const auto &value: list) {
                insertBack(value);
            }
        }

        DLList(std::initializer_list<T> list) {
            for (const auto &value: list) {
                insertBack(std::make_shared<T>(value));
            }
        }

        Stream<T> stream() {
            Stream<T> stream;
            for (auto &&el: *this) {
                stream.getData().push_back(std::move(el));
            }
            return stream;
        }

        ~DLList() = default;
    };

    namespace Collectors {
        inline auto toDLList() {
            return [](auto begin, auto end) {
                DLList<std::remove_reference_t<decltype(*begin)> > list;
                for (auto it = begin; it != end; ++it) {
                    list.emplaceBack(std::move(*it));
                }
                return list;
            };
        }
    }

    template<typename T>
    inline void readBufferImpl(DataBuffer &buffer, DLList<T> &list) {
        auto size = buffer.read<decltype(list.size())>();
        for (size_t i = 0; i < size; ++i) {
            list.emplaceBack(buffer.read<T>());
        }
    }

    template<typename T>
    inline void writeBufferImpl(DataBuffer &buffer, const DLList<T> &list) {
        buffer.write(list.size());
        for (auto &&el: list) {
            buffer.write<T>(el);
        }
    }
}
