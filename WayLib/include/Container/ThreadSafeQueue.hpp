#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T, template<typename> typename Container = std::queue>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;

    ThreadSafeQueue(const ThreadSafeQueue &) = delete;

    ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;

    // ThreadSafeQueue is non-copyable, non-movable, if you want to move it, you can use std::unique_ptr<ThreadSafeQueue<T>>

    void emplace(auto &&... args) { {
            std::scoped_lock lock(m_Mutex);
            m_Data.emplace(std::forward<decltype(args)>(args)...);
        }

        m_Condition.notify_one();
    }

    void push(T &&data) {
        this->emplace(std::move(data));
    }

    std::optional<T> tryPull() {
        std::optional<T> result; {
            std::scoped_lock lock(m_Mutex);
            if (!m_Data.empty()) {
                result = std::move(m_Data.front());
                m_Data.pop();
            }
        }

        return result;
    }

    T pull() {
        std::unique_lock lock(m_Mutex);
        m_Condition.wait(lock, [this] { return !m_Data.empty(); });
        T result = std::move(m_Data.front());
        m_Data.pop();
        return result;
    }

    bool tryVisit(auto &&visitor) {
        std::unique_lock lock(m_Mutex);
        if (m_Data.empty()) {
            return false;
        }
        std::invoke(visitor, std::move(m_Data.front()));
        m_Data.pop();
        return true;
    }

    void visit(auto &&visitor) {
        std::unique_lock lock(m_Mutex);
        m_Condition.wait(lock, [this] { return !m_Data.empty(); });
        std::invoke(visitor, std::move(m_Data.front()));
        m_Data.pop();
    }

    const Container<T> &getData() const {
        return m_Data;
    }

    void notifyOne() const {
        m_Condition.notify_one();
    }

    void notifyAll() const {
        m_Condition.notify_all();
    }

    bool tryLock() const {
        return m_Mutex.try_lock();
    }

    void lock() const {
        m_Mutex.lock();
    }

    void unlock() const {
        m_Mutex.unlock();
    }

    std::mutex &getMutex() const {
        return m_Mutex;
    }

    std::condition_variable &getCondition() const {
        return m_Condition;
    }

private:
    Container<T> m_Data;
    mutable std::mutex m_Mutex;
    mutable std::condition_variable m_Condition;
};
