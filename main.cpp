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
}

void failedCode() {}
