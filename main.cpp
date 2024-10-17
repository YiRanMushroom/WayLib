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
    });

    auto res = std::move(range)
               | WayLib::Ranges::forEach([](int item) {
                   // std::cout << item << std::endl;
               }) | WayLib::Ranges::filter([](int item) {
                   return item % 2 == 0;
               }) | WayLib::Ranges::map([](int item) {
                   return '+' + std::to_string(item * 2);
               })
               | WayLib::Ranges::concat(std::vector{"a", "b", "c"})
               | WayLib::Ranges::forEach([](std::string &item) {
                   // std::cout << item << std::endl;
               })
               | WayLib::Ranges::forEach([](std::string &item) {}) | WayLib::Ranges::append("hello", "world")
               | WayLib::Ranges::forEach([](std::string &item) {
                   // std::cout << item << std::endl;
               }) | WayLib::Ranges::sync();


    // auto next = (*res.get());

    std::cout << "======================\n" << std::endl;

    std::vector<std::string> vec2;

    res | WayLib::Ranges::forEach([&](std::string &item) {
        std::cout << item << std::endl;
        vec2.push_back(item);
    }) |WayLib::Ranges::sync();

    std::cout << "Result: " << typeid(res).name() << std::endl;

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
