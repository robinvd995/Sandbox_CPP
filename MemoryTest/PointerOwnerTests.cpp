#include "pch.h"

using namespace Jupiter;

#define MEMORY_EXCEPTION_CHECKING

class Foo {

public:
	uint value0;
	uint value1;

	Foo() : value0(0), value1(0) {}
	Foo(uint v0, uint v1) : value0(v0), value1(v1) {}
};

TEST(PointerOwnerTests, Create) {
	uint i0 = 10;
	uint i1 = 20;

	uint* i0ptr = nullptr;
	uint* i1ptr = nullptr;

	{
		ptr_owner<Foo> fooptr = PointerCreator::createPtrOwner<Foo>(i0, i1);
		EXPECT_EQ(i0, fooptr->value0);
		EXPECT_EQ(i1, fooptr->value1);

		i0ptr = &fooptr->value0;
		i1ptr = &fooptr->value1;
	}

	EXPECT_FALSE(*(i0ptr) == i0);
	EXPECT_FALSE(*(i1ptr) == i1);
}

TEST(PointerOwnerTests, Duplicate) {

}