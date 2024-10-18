#include <iostream>

/*#include "Container/DLList.hpp"
#include "Util/Stream.hpp"
#include "Util/StreamUtil.hpp"
#include "Util/DataBuffer.hpp"
#include "Util/FileSystem.hpp"
#include "Util/StringLiteral.hpp"
#include "Container/ThreadSafeQueue.hpp"*/
#include <string>

#include "Util/OperatorExtension.hpp"
#include "Util/ThreadPool.hpp"
#include "Util/Range/Range.hpp"
#include "Util/Range/RangeUtil.hpp"

using namespace std::string_literals;

void failedCode();


int main() {
    auto &pool = WayLib::ThreadPool::GlobalInstance();
    pool.dispatch([]() {
        std::cout << "Hello World!" << std::endl;
    }).get();

    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto range = vec | WayLib::Ranges::toRange();

    auto future = range
                  | WayLib::Ranges::move()
                  | WayLib::Ranges::forEach([](int item) {
                      std::cout << item << std::endl;
                  })
                  | WayLib::Ranges::filter([](int item) {
                      return item % 2 == 0;
                  })
                  | WayLib::Ranges::map([](int item) {
                      return '+' + std::to_string(item * 2);
                  })
                  | WayLib::Ranges::concat(std::vector{"a", "b", "c"})
                  | WayLib::Ranges::append("hello", "world")
                  | WayLib::Ranges::asyncSync();


    auto next = (*future.get());

    using namespace WayLib::OperatorExtensions;

    auto something = "a,b,c,d,e,f,gasdfas,askfl j"s | WayLib::Ranges::toRange() | WayLib::Ranges::split(',')
                     | WayLib::Ranges::map(WayLib::Ranges::Mapper::CharVectorToString())
                     | WayLib::Ranges::forEach([](const std::string &str) {
                         std::cout << str << std::endl;
                     }) | WayLib::Ranges::flatMap([](const std::string &str) {
                         return std::vector{str, str + "1", str + "2"};
                     }) | WayLib::Ranges::forEach([](const std::string &str) {
                         std::cout << str << std::endl;
                     }) | WayLib::Ranges::sortedByDescending([](const std::string &str) {
                         return str.size();
                     })
                     | WayLib::Ranges::forEach([](const std::string &str) {
                         std::cout << str << std::endl;
                     })
                     | WayLib::Ranges::append("hel"s, "lo "s)
                     | WayLib::Ranges::firstMatch([](const std::string &str) {
                         return str.size() == 3;
                     }) | WayLib::MemberFunctionExtensions::dereference();


    std::cout << "======================\n" << std::endl;

    std::vector<std::string> vec2 = next | WayLib::Ranges::move() | WayLib::Ranges::collect(
                                        WayLib::Ranges::Collectors::ToVector());

    for (const auto &item: vec2) {
        std::cout << item << " ";
    }

    std::cout << std::endl;

    std::cout << "Result: " << typeid(future).name() << std::endl;

    struct X {
        X() = default;

        X(const X &) {
            std::cout << "Copy constructor" << std::endl;
        }

        X(X &&) noexcept {
            std::cout << "Move constructor" << std::endl;
        }

        X &operator=(const X &) {
            std::cout << "Copy assignment" << std::endl;
            return *this;
        }

        X &operator=(X &&) noexcept {
            std::cout << "Move assignment" << std::endl;
            return *this;
        }

        ~X() = default;
    };

    std::vector<X> xVec(1);

    std::cout << "======================\n" << std::endl;

    using T = typename std::decay_t<decltype(xVec)>::value_type;

    WayLib::Range<T, void>{
        [ptr = std::shared_ptr<std::vector<T> >(&xVec, [](std::vector<T> *) {})]() {
            return ptr;
        }
    };

    /*[](const auto &vec) {
        using T = typename std::decay_t<decltype(vec)>::value_type;
        return WayLib::Range<X, void>{
            [ptr = std::shared_ptr<std::vector<X> >(&vec, [] (std::vector<X>*) {})]() {
                return ptr;
            }
        };
    }(xVec);*/

    xVec | WayLib::Ranges::asRangeNoOwnership() | WayLib::Ranges::forEach([](X &x) {
        std::cout << "Hello" << std::endl;
    }) | WayLib::Ranges::map([](X &x) -> X&& {
        return std::move(x);
    }) | WayLib::Ranges::forEach([](X &x) {
        std::cout << "Hello" << std::endl;
    }) | WayLib::Ranges::sync();

    int b = 1;

    std::shared_ptr<int> ptr = std::shared_ptr<int>(&b, [](int *ptr) {
        std::cout << "Deleting" << std::endl;
    });
}

void failedCode() {}
