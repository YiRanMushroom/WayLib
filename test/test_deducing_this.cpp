#include <iostream>

template<typename T>
class Base {
public:
    void foo(this auto&& self) {
        self.bar();
    }

    void baz(this auto &&self) {
        self.fun();
    }
};

template<typename T>
class Derived : public Base<T> {
    friend class Base<T>;
private:
    decltype(auto) bar() {
        std::cout << "bar" << std::endl;
    }

    decltype(auto) fun(this auto &&self) {
        std::cout << "fun" << std::endl;
    }
};

void test() {
    Derived<int> d;
    d.foo();
    d.baz();
}
