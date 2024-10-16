#pragma once
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

namespace WayLib {
    class ThreadPool {
    public:
        explicit ThreadPool(size_t maxThreads = 2 * std::thread::hardware_concurrency()) : m_MaxThreads(
            std::min<uint32_t>(maxThreads, 256u)) {
            m_Threads.reserve(m_MaxThreads);
            for (uint32_t i = 0; i < m_MaxThreads; ++i) {
                m_Threads.emplace_back([this] {
                    while (!m_Stop) {
                        std::function<void()> task; {
                            std::unique_lock lock(m_Mutex);
                            m_Condition.wait(lock, [this] { return m_Stop || !m_Tasks.empty(); });
                            if (m_Stop) {
                                return;
                            }
                            if (m_Tasks.empty()) {
                                // this should never reach
                                continue;
                            }
                            task = std::move(m_Tasks.front());
                            m_Tasks.pop();
                        }
                        std::invoke(task);
                    }
                });
            }
        }


        void stop() {
            m_Stop = true;
            m_Condition.notify_all();
        }

        ~ThreadPool() {
            stop();
            for (auto &thread: m_Threads) {
                thread.join();
            }
        }

        uint32_t getMaxThreads() const {
            return m_MaxThreads;
        }

        template<typename F, typename... Args>
        [[nodiscard]] auto dispatch(F &&function, Args &&... args) {
            using ReturnType = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
            auto task = std::make_unique<std::packaged_task<ReturnType()> >(
                std::bind(std::forward<F>(function), std::forward<Args>(args)...)
            );
            auto future = task->get_future(); {
                std::unique_lock lock(m_Mutex);
                m_Tasks.emplace(
                    [inner = task.get()]() mutable {
                        std::unique_ptr<std::packaged_task<ReturnType()> > task(inner);
                        std::invoke(*task);
                    });
            }
            task.release();
            m_Condition.notify_one();
            return future;
        }

        void dispatchDetached(auto... args) {
            dispatch(std::forward<decltype(args)>(args)...);
        }

        auto dispatchBlocked(auto... args) {
            return dispatch(std::forward<decltype(args)>(args)...).get();
        }

        ThreadPool(const ThreadPool &) = delete;

        ThreadPool &operator=(const ThreadPool &) = delete;

        static ThreadPool &GlobalInstance() {
            static ThreadPool instance;
            return instance;
        }

        static auto Async(auto &&... args) {
            return GlobalInstance().dispatch(std::forward<decltype(args)>(args)...);
        }

        static void AsyncDetached(auto &&... args) {
            GlobalInstance().dispatchDetached(std::forward<decltype(args)>(args)...);
        }

    private:
        uint32_t m_MaxThreads;
        std::mutex m_Mutex{};
        std::condition_variable m_Condition{};
        std::queue<std::function<void()> > m_Tasks{};
        std::atomic<bool> m_Stop{false};

        std::vector<std::thread> m_Threads{};
    };
}
