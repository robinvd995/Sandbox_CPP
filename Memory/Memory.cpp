//#include <iostream>
//
//#include "JupiterMemory.h"
//
//#include <vector>
//
//using namespace Jupiter;
//
//class Foo {
//
//public:
//    Foo() {
//        std::cout << "Foo default constructor called" << std::endl;
//    }
//    Foo(const Foo& other) {
//        std::cout << "Foo copy constructor called" << std::endl;
//    }
//    Foo(int i0, int i1) : v0(i0), v1(i1) {
//        std::cout << "Foo argument constructor called" << std::endl;
//    }
//    ~Foo() {
//        std::cout << "Foo destructor called" << std::endl;
//    }
//
//    int v0 = 0, v1 = 0;
//};
//
//struct FooCollection {
//    // std::vector<ptr_owned<Foo>> vectorFoo;
//};
//
//int main() {
//    // FooCollection coll;
//    // coll.vectorFoo.push_back(JupiterMemory::createOwned<Foo>(10, 20));
//    // ptr_reference<Foo> ref = coll.vectorFoo[0].grabReference();
//    // std::cout << ref.isValid() << std::endl;
//    // coll.vectorFoo[0].invalidate();
//    // std::cout << ref.isValid() << std::endl;
//}
