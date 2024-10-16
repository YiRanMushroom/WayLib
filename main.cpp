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

    // auto vector = range | WayLib::Ranges::collect(WayLib::Ranges::Collectors::ToVector());

    auto res = std::move(range)
               | WayLib::Ranges::forEach([](int item) {
                   std::cout << item << std::endl;
               }) | WayLib::Ranges::filter([](int item) {
                   return item % 2 == 0;
               }) | WayLib::Ranges::map([](int item) {
                   return '+' + std::to_string(item * 2);
               }) | WayLib::Ranges::typeDecay()
               | WayLib::Ranges::forEach([](std::string &item) {
                   std::cout << item << std::endl;
               }) | WayLib::Ranges::discardLast()
               | WayLib::Ranges::syncAndDecay()
               | WayLib::Ranges::collect(WayLib::Ranges::Collectors::ToVector());

    std::cout << typeid(res).name() << std::endl;

    auto bigNumber = pool.dispatch([]() {
        double result = 1;
        for (int i = 1; i <= 100; ++i) {
            result *= i;
        }
        return result;
    });

    auto result = bigNumber.get();
    std::cout << "Result: " << result << std::endl;
}

void failedCode() {
}
