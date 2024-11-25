#include <iostream>
#include <string>
#include <unordered_set>

#include "Util/OperatorExtension.hpp"
#include "Util/ThreadPool.hpp"
#include "Util/Range/Range.hpp"
#include "Util/Range/RangeUtil.hpp"

using namespace std::string_literals;

void failedCode();

double toDouble(const std::string &str) {
    double res{std::stod(str)};
    return res;
}

double square(double x) {
    return x * x;
}

int toInt(double x) {
    return static_cast<int>(x);
}

int factorial(int x) {
    return x == 0 ? 1 : x * factorial(x - 1);
}

struct SomeLambda {
    int x;

    SomeLambda(int x) : x(x) {
    }

    int operator()(int y) const {
        return x + y;
    }
};

int main() {
    auto &pool = WayLib::ThreadPool::GlobalInstance();
    pool.dispatch([]() {
        std::cout << "Hello World!" << std::endl;
    }).get();

    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto range = vec | WayLib::Ranges::toRange();

    std::cout << std::boolalpha << std::is_trivially_destructible_v<std::unique_ptr<int>>;
    std::cout << std::boolalpha << std::is_trivially_copy_constructible_v<std::unique_ptr<int>>;
    std::cout << std::boolalpha << std::is_trivially_copy_assignable_v<std::unique_ptr<int>>;

    std::function f = []() {
        return 1;
    };

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


    auto next = future.get();

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

    /*std::vector<std::string> vec2 = next | WayLib::Ranges::move() | WayLib::Ranges::collect(
                                        WayLib::Ranges::Collectors::ToVector());

    for (const auto &item: vec2) {
        std::cout << item << " ";
    }

    std::cout << std::endl;

    std::cout << "Result: " << typeid(future).name() << std::endl;*/

    struct X {
        X() = default;

        X(const X &) {
            std::cout << "Copy constructor" << std::endl;
        }

        X(X &&) noexcept {
            std::cout << "Move constructor" << std::endl;
        }

        X &operator=(const X &) {
            std::cout << "Copy assignment" << std::endl;
            return *this;
        }

        X &operator=(X &&) noexcept {
            std::cout << "Move assignment" << std::endl;
            return *this;
        }

        ~X() = default;
    };

    std::vector<X> xVec(1);

    std::cout << "======================\n" << std::endl;

    using T = typename std::decay_t<decltype(xVec)>::value_type;

    WayLib::Range<T, void>{
        [ptr = std::shared_ptr<std::vector<T> >(&xVec, [](std::vector<T> *) {
        })]() {
            return ptr;
        }
    };

    auto some = xVec | WayLib::Ranges::asRangeNoOwnership() | WayLib::Ranges::forEach([](X &x) {
        std::cout << "Hello" << std::endl;
    }) | WayLib::Ranges::map([](auto &&x) -> X &&{
        return std::move(x);
    }) | WayLib::Ranges::forEachImmediate([](X &x) {
        std::cout << "Hello" << std::endl;
    });

    std::cout << typeid(some).name();

    std::cout << std::endl;

    int b = 1;


    using namespace WayLib::OperatorExtensions;

    // 1 | bar;

    using Tp = decltype(Util::PipeForward(toDouble, square, toInt, factorial));

    Tp func = Util::PipeForward(toDouble, square, toInt, factorial);

    std::cout << func("3.5") << std::endl;

    auto sharedVector = std::make_shared<std::vector<int> >(4);

    sharedVector | WayLib::Ranges::shareRange() | WayLib::Ranges::forEachImmediate([](int &item) {
        std::cout << item << std::endl;
    });

    int whate = 0;

    int &whateRef = whate;

    whateRef = 1;

    int res = SomeLambda{1}(2);
}

void failedCode() {
}
