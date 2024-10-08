#include <iostream>

#include "Container/DLList.hpp"
#include "Util/Stream.hpp"
#include "Util/StreamUtil.hpp"
#include "Util/DataBuffer.hpp"
#include "Util/FileSystem.hpp"
#include "Util/StringLiteral.hpp"

void failedCode();

int main() {
    std::vector v{1, 2, 3, 4, 5, 5, 7, 1, 2, 3, 4, 5, 5, 7};
    auto res =
            WayLib::Streamers::Of(v)
            .distinct()
            .sortDesc()
            .then([] { std::cout << std::endl; })
            .forEach([](auto &&el) { std::cout << el << ' '; })
            .then([] { std::cout << std::endl; })
            .runningReduce(WayLib::Transformers::Add())
            .sortDesc()
            .let(WayLib::Utils::PrintAll())
            .collect(WayLib::Collectors::toDLList())
            .apply([](auto &&lst) {
                WayLib::DataBuffer buffer;
                buffer.pushBack(lst);
                return buffer;
            });

    auto bs = WayLib::Streamers::Of(v)
            .sort()
            .let(WayLib::Utils::PrintAll())
            .binarySearch([](int val) { return val >= 5; })
            .value();

    auto location = WayLib::Utils::FileLocation{"resources/test.txt"};

    std::cout << location.getPath() << std::endl;

    WayLib::DataBuffer buffer_wr;
    buffer_wr.pushBack(std::vector{1.0, 2.0, 3.0, 4.0, 5.0});
    auto ofs = location.ofstream();
    buffer_wr.writeToStream(ofs);
    ofs.close();

    auto ifs = location.ifstream();
    WayLib::DataBuffer buffer_rd = WayLib::CreateBufferFromStream(ifs);

    auto vec = buffer_rd.read<std::vector<double> >();

    auto list = res.read<WayLib::DLList<int> >();

    list.let(WayLib::Utils::PrintAll());

    auto res2 = WayLib::Streamers::Of(std::vector<std::vector<int> >{{1, 2, 3}, {7, 9, 8}, {6, 5, 4}})
            .flatMap(WayLib::Transformers::AllOf()).sortByDesc(WayLib::Transformers::IdentityOf())
            .map(WayLib::Transformers::MakeUnique<int>())
            .forEach([](std::unique_ptr<int> &&ptr) {
                std::unique_ptr<int> p = std::forward<decltype(ptr)>(ptr);
            });

    res2.move().forEach([](std::unique_ptr<int> &&ptr) {
        std::unique_ptr<int> p = std::forward<decltype(ptr)>(ptr);
    });

    using PairType = std::invoke_result_t<decltype([](int el) { return std::make_pair(el, el * el); }), int>;

    WayLib::DataBuffer buffer;

    PairType p = std::make_pair(1, 2);

    buffer.pushBack(std::string{"Hello, World!"});
    auto str = buffer.read<std::string>();
    std::cout << str << std::endl;

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

    std::cout << std::endl;

    WayLib::DLList lst{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    lst.sortDesc();

    lst.forEach([](int el) { std::cout << el << ' '; });

    WayLib::StringLiteral strLit{"Hello, World!"};
}

void failedCode() {
    /*struct Int {
        int val;

        Int() = default;

        int & operator++() {
            return ++val;
        }

        Int(int val) : val(val) {}

        bool operator==(const Int &rhs) const {
            return val == rhs.val;
        }

        bool operator!=(const Int &rhs) const {
            return !(rhs == *this);
        }

        bool operator<(const Int &rhs) const {
            return val < rhs.val;
        }

        bool operator>(const Int &rhs) const {
            return rhs < *this;
        }

        bool operator<=(const Int &rhs) const {
            return !(rhs < *this);
        }

        bool operator>=(const Int &rhs) const {
            return !(*this < rhs);
        }

        Int & operator=(const Int &rhs) {
            val = rhs.val;
            return *this;
        }
    };

    std::map<Int, Int> mp1;

    Int b = 0;

    mp1[++b].operator=(++b);

    Int c{};

    std::map<Int, Int> mp2;

    mp2[++c] = ++c;*/

    WayLib::DataBuffer buffer;
    buffer.read<int>();
}
