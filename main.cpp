#include <iostream>

#include "Container/DLList.hpp"
#include "Util/Stream.hpp"
#include "Util/StreamUtil.hpp"
#include "Util/DataBuffer.hpp"

void failedCode();

int main() {
    std::vector v = {1, 2, 3, 4, 5, 5, 7, 1, 2, 3, 4, 5, 5, 7};
    auto res =
            WayLib::Streamers::of(v)
            .distincted()
            .sortedDesc()
            .then([] { std::cout << std::endl; })
            .forEach([](auto &&el) { std::cout << el << ' '; })
            .then([] { std::cout << std::endl; })
            .runningReduced(WayLib::Transformers::add())
            .sortedDesc()
            .apply(WayLib::Utils::printAll())
            .collect(WayLib::Collectors::toDLList())
            .apply([](auto &&lst) {
                WayLib::DataBuffer buffer;
                buffer.pushBack(lst);
                return buffer;
            });

    auto bs = WayLib::Streamers::of(v)
            .sorted()
            .apply(WayLib::Utils::printAll())
            .binarySearch([](int val) { return val >= 5; })
            .value();

    auto list = res.read<WayLib::DLList<int> >();

    list.apply(WayLib::Utils::printAll());

    auto res2 = WayLib::Streamers::of(std::vector<std::vector<int> >{{1, 2, 3}, {7, 9, 8}, {6, 5, 4}})
            .flatMapped(WayLib::Transformers::allOf()).sortedByDesc(WayLib::Transformers::identityOf())
            .mapped(WayLib::Transformers::makeUnique<int>())
            .forEach([](std::unique_ptr<int> &&ptr) {
                std::unique_ptr<int> p = std::forward<decltype(ptr)>(ptr);
            });

    res2.move().forEach([](std::unique_ptr<int> &&ptr) {
        std::unique_ptr<int> p = std::forward<decltype(ptr)>(ptr);
    });

    using PairType = std::invoke_result_t<decltype([](int el) { return std::make_pair(el, el * el); }), int>;

    PairType p = std::make_pair(1, 2);

    WayLib::DataBuffer buffer;
    buffer.pushBack(std::string{"Hello, World!"});
    auto str = buffer.read<std::string>();
    std::cout << str << std::endl;

    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8};
    buffer.pushBack(vec);
    auto vec2 = buffer.read<std::vector<int> >();
    for (auto &&el: vec2) {
        std::cout << el << ' ';
    }

    std::cout << std::endl;

    std::pair<int, int> pair = {1, 2};
    buffer.pushBack(pair);
    auto pair2 = buffer.read<std::pair<int, int> >();
    std::cout << pair2.first << ' ' << pair2.second << std::endl;

    std::tuple<int, std::string, int> tuple = {1, "Hi there", 3};
    buffer.pushBack(tuple);
    auto tuple2 = buffer.read<std::tuple<int, std::string, int> >();
    std::cout << std::get<0>(tuple2) << ' ' << std::get<1>(tuple2) << ' ' << std::get<2>(tuple2) << std::endl;

    std::map<int, std::string> map = {{1, "One"}, {2, "Two"}, {3, "Three"}};
    buffer.pushBack(map);
    auto map2 = buffer.read<std::map<int, std::string> >();
    for (auto &&[K, V]: map2) {
        std::cout << K << " -> " << V << std::endl;
    }

    std::unordered_map<int, std::string> umap = {{1, "One"}, {2, "Two"}, {3, "Three"}};
    buffer.pushBack(umap);
    auto umap2 = buffer.read<std::unordered_map<int, std::string> >();
    for (auto &&[K, V]: umap2) {
        std::cout << K << " -> " << V << std::endl;
    }

    std::unordered_set<int> uset = {1, 2, 3, 4, 5};
    buffer.pushBack(uset);
    auto uset2 = buffer.read<std::unordered_set<int> >();
    for (auto &&el: uset2) {
        std::cout << el << ' ';
    }

    std::set<int> set = {1, 2, 3, 4, 5};
    buffer.pushBack(set);
    auto set2 = buffer.read<std::set<int> >();
    for (auto &&el: set2) {
        std::cout << el << ' ';
    }

    // test fail:
    try {
        failedCode();
    } catch (WayLib::RichException &e) {
        e.what();
    }
}


void failedCode() {
    WayLib::DataBuffer buffer;
    buffer.read<int>();
}
