#include <iostream>

#include "util/Containers.hpp"
#include "util/Stream.hpp"
#include "util/StreamUtil.hpp"

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
            .apply(WayLib::Utils::printAll(std::cout))
            .mapped(WayLib::Transformers::makeUnique<int>())
            // .collect(WayLib::Collectors::toDLList())
            .groupBy([](auto &&el) { return *el; });

    auto res2 = WayLib::Streamers::of(std::vector<std::vector<int> >{{1, 2, 3}, {7, 9, 8}, {6, 5, 4}})
            .flatMapped(WayLib::Transformers::allOf()).sortedByDesc(WayLib::Transformers::identityOf())
            .collect(WayLib::Collectors::toSet());

    using PairType = std::invoke_result_t<decltype([](int el) { return std::make_pair(el, el * el); }), int>;

    PairType p = std::make_pair(1, 2);

    for (auto &&[K, V]: res) {
        std::cout << K << " -> " << V.get() << std::endl;
        std::cout << std::endl;
    }
}
