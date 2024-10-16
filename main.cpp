#include <iostream>

#include "Container/DLList.hpp"
#include "Util/Stream.hpp"
#include "Util/StreamUtil.hpp"
#include "Util/DataBuffer.hpp"
#include "Util/FileSystem.hpp"
#include "Util/StringLiteral.hpp"
#include "Container/ThreadSafeQueue.hpp"
#include "Util/ThreadPool.hpp"

void failedCode();

int main() {
    auto& pool = WayLib::ThreadPool::GlobalInstance();
    pool.dispatch([]() {
        std::cout << "Hello World!" << std::endl;
    }).get();
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
    WayLib::DataBuffer buffer;
    buffer.read<int>();
}
