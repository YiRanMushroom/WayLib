#include <iostream>

/*#include "Container/DLList.hpp"
#include "Util/Stream.hpp"
#include "Util/StreamUtil.hpp"
#include "Util/DataBuffer.hpp"
#include "Util/FileSystem.hpp"
#include "Util/StringLiteral.hpp"
#include "Container/ThreadSafeQueue.hpp"*/
#include <string>

#include "Util/ThreadPool.hpp"
#include "Util/Range/Range.hpp"
#include "Util/Range/RangeUtil.hpp"

void failedCode();

int main() {
    auto &pool = WayLib::ThreadPool::GlobalInstance();
    pool.dispatch([]() {
        std::cout << "Hello World!" << std::endl;
    }).get();

    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto range = vec | WayLib::Ranges::toRange();

    WayLib::Range rg = range | WayLib::Ranges::toRange() | WayLib::Ranges::map([](int item) {
        return std::to_string(item);
    }) | WayLib::Ranges::discardLast();

    auto future = range | WayLib::Ranges::move()
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

    std::cout << "======================\n" << std::endl;

    std::vector<std::string> vec2 = next | WayLib::Ranges::move() | WayLib::Ranges::collect(
                                        WayLib::Ranges::Collectors::ToVector());

    for (const auto &item: vec2) {
        std::cout << item << " ";
    }

    std::cout << std::endl;

    std::cout << "Result: " << typeid(future).name() << std::endl;

    /*std::vector{1, 2, 3, 4, 5} | WayLib::Ranges::toRange() | WayLib::Ranges::filter([](int item) {
        return item % 2 == 0;
    }) | WayLib::Ranges::map([](int item) {
        return '+' + std::to_string(item * 2);
    })
    | WayLib::Ranges::concat(std::vector{"a", "b", "c"})

    | WayLib::Ranges::forEach([](auto&& item) {
        std::cout << item << std::endl;
    }) | WayLib::Ranges::sync();*/


    /*auto bigNumber = pool.dispatch([]() {
        double result = 1;
        for (int i = 1; i <= 100; ++i) {
            result *= i;
        }
        return result;
    });

    auto result = bigNumber.get();
    std::cout << "Result: " << result << std::endl;*/
}

void failedCode() {}
