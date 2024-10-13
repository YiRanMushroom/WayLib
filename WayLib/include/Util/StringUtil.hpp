#pragma once
#include <string>
#include <vector>

#include "StreamUtil.hpp"

namespace WayLib {
    inline std::vector<std::string> split(const std::string &str, const std::unordered_set<char> &seps) {
        auto stream = Streamers::Of(str);
        return stream.fold(std::vector<std::string>{}, [&seps](std::vector<std::string> &&acc, char el) {
            if (seps.contains(el) && (!acc.empty() && !acc.back().empty())) {
                acc.emplace_back();
            } else {
                acc.back().push_back(el);
            }
            return acc;
        });
    }

    inline std::vector<std::string> split(const std::string &str, const std::string &seps = " ,\t\n") {
        std::unordered_set<char> sepsSet;
        for (auto &&el: seps) {
            sepsSet.insert(el);
        }
        return split(str, sepsSet);
    }

    inline std::vector<std::string> split(const std::string &str, char sep) {
        return split(str, std::unordered_set{sep});
    }
}
