#pragma once

#include "Range.hpp"

namespace WayLib::Ranges::Collectors {
    inline auto ToVector() {
        return [](auto&& range) {
            return *range.get();
        };
    }
}
