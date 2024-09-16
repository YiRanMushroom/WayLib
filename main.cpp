#include <iostream>

#include "util/Containers.hpp"
#include "util/Stream.hpp"
#include "util/StreamUtil.hpp"
#include "CRTP/inject_container_traits.hpp"

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
            .mapped([](auto&& el) { return std::make_unique<int>(el); })
            .collect(WayLib::Collectors::toDLList());

    auto res2 = WayLib::Streamers::of(std::vector<std::vector<int> >{{1, 2, 3}, {7, 9, 8}, {6, 5, 4}})
            .flatMapped(WayLib::Transformers::allOf()).sortedByDesc(WayLib::Transformers::identityOf())
            .collect(WayLib::Collectors::toSet());

    for (auto &&val: res) {
        std::cout << *val << ' ';
    }
}
