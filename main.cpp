#include <iostream>
#include <string>

#include "util/Stream.hpp"
#include "util/StreamUtil.hpp"

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5, 5, 7, 1, 2, 3, 4, 5, 5, 7};
    auto res =
            Streamers::of(v)
            .distinct()
            .forEach([](auto &&el) { std::cout << el << ' '; })
            .sortedDesc()
            .then([] { std::cout << std::endl; })
            .forEach([](auto &&el) { std::cout << el << ' '; })
            .then([] { std::cout << std::endl; })
            .mapNotNull([](auto &&el) -> std::optional<int64_t> {
                return el % 2 == 0 ? std::optional<int>(el) : std::nullopt;
            })
            .groupedBy([](auto &&el) { return std::make_pair(Transformers::toString()(el), el); });

    auto res2 = Streamers::of(std::vector<std::vector<int> >{{1, 2, 3}, {7, 9, 8}, {6, 5, 4}})
            .flatMap(Transformers::allOf()).sortedByDesc(Transformers::identityOf())
            .collect(Collectors::toSet());

    for (auto &&[key, value]: res) {
        std::cout << key << " -> " << value << std::endl;
    }
}
