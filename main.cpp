#include <iostream>

#include "util/Containers.hpp"
#include "util/Stream.hpp"
#include "util/StreamUtil.hpp"

int main() {
    std::vector v = {1, 2, 3, 4, 5, 5, 7, 1, 2, 3, 4, 5, 5, 7};
    auto res =
            WayLib::Streamers::of(v)
            .distinct()
            .forEach([](auto &&el) { std::cout << el << ' '; })
            .sortedDesc()
            .then([] { std::cout << std::endl; })
            .forEach([](auto &&el) { std::cout << el << ' '; })
            .then([] { std::cout << std::endl; })
            .mapNotNull([](auto &&el) -> std::optional<int64_t> {
                return el % 2 == 0 ? std::optional<int>(el) : std::nullopt;
            })
            .collect(WayLib::Collectors::toDLList());

    auto res2 = WayLib::Streamers::of(std::vector<std::vector<int> >{{1, 2, 3}, {7, 9, 8}, {6, 5, 4}})
            .flatMap(WayLib::Transformers::allOf()).sortedByDesc(WayLib::Transformers::identityOf())
            .collect(WayLib::Collectors::toSet());

    for (auto &&val : res) {
        std::cout << val << std::endl;
    }
}
