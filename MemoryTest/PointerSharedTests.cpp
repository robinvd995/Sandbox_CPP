#include "pch.h"

using namespace Jupiter;

#define MEMORY_EXCEPTION_CHECKING

// Test container class
class Foo {

public:
	uint value0;
	uint value1;

	Foo() : value0(0), value1(0) {}
	Foo(uint v0, uint v1) : value0(v0), value1(v1) { std::cout << "Foo Constructor" << std::endl; }
	~Foo() { std::cout << "Foo Destructor!" << std::endl; }
};

// Class with the same data template as ptr_control_block used to access its private members
class ControlBlockAccess {

public:
	uint m_StrongReferenceCount = 0;
	uint m_WeakReferenceCount = 0;
	bool m_Valid = false;
};

// Class with the same data template as ptr_shared used to access its private members
template<typename T>
class PointerSharedAccess {

public:
	T* m_SharedData;
	ptr_control_block* m_ControlBlock;
};

TEST(PointerSharedTests, CreateDelete) {
	uint i0 = 10;
	uint i1 = 20;

	uint* i0ptr = nullptr;
	uint* i1ptr = nullptr;

	{
		ptr_shared<Foo> fooptr = PointerCreator::createPtrShared<Foo>(i0, i1);
		EXPECT_EQ(i0, fooptr->value0);
		EXPECT_EQ(i1, fooptr->value1);
		
		i0ptr = &fooptr->value0;
		i1ptr = &fooptr->value1;
	}

	EXPECT_FALSE(*(i0ptr) == i0);
	EXPECT_FALSE(*(i1ptr) == i1);
}

TEST(PointerSharedTests, Copy) {
	uint i0 = 10;
	uint i1 = 20;

	uint* i0ptr = nullptr;
	uint* i1ptr = nullptr;

	{
		ptr_shared<Foo> fooptr0 = PointerCreator::createPtrShared<Foo>(i0, i1);
		PointerSharedAccess<Foo>* ptraccess = (PointerSharedAccess<Foo>*)&fooptr0;
		ControlBlockAccess* ctrlaccess = (ControlBlockAccess*)ptraccess->m_ControlBlock;

		i0ptr = &fooptr0->value0;
		i1ptr = &fooptr0->value1;
		{
			ptr_shared<Foo> fooptr1 = fooptr0;
		
			EXPECT_EQ(i0, fooptr1->value0);
			EXPECT_EQ(i1, fooptr1->value1);
			std::cout << "First check" << std::endl;
		}

		EXPECT_TRUE(ctrlaccess->m_Valid);
		EXPECT_EQ(i0, fooptr0->value0);
		EXPECT_EQ(i1, fooptr0->value1);

		std::cout << "Second check" << std::endl;
	}

	EXPECT_FALSE(*(i0ptr) == i0);
	EXPECT_FALSE(*(i1ptr) == i1);
}