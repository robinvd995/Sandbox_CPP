#include "pch.h"

#include "MemoryHelperWindows.hpp"

#include <vector>

using namespace Jupiter;

#define MEMORY_EXCEPTION_CHECKING

class Foo {

public:
	uint value0;
	uint value1;

	Foo() : value0(0), value1(0) {}
	Foo(uint v0, uint v1) : value0(v0), value1(v1) { std::cout << "Foo Constructor" << std::endl; }
	~Foo() { std::cout << "Foo Destructor!" << std::endl; }
};

/// <summary>
/// Class with the same data template as ptr_control_block used to access its private members
/// </summary>
class ControlBlockAccess {

public:
	uint m_StrongReferenceCount = 0;
	uint m_WeakReferenceCount = 0;
	bool m_Valid = false;
};

/// <summary>
/// Class with the same data template as ptr_owner used to access its private members
/// </summary>
template<typename T>
class PointerOwnerAccess {

public:
	T* m_Data;
	ptr_control_block* m_ControlBlock;
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

TEST(PointerOwnerTests, Access) {
	ptr_owner<Foo> ptr = PointerCreator::createPtrOwner<Foo>(4, 2);
	PointerOwnerAccess<Foo>* access = (PointerOwnerAccess<Foo>*)&ptr;
	ControlBlockAccess* blockAccess = (ControlBlockAccess*)access->m_ControlBlock;

	EXPECT_EQ(4, access->m_Data->value0);
	EXPECT_EQ(2, access->m_Data->value1);
	EXPECT_EQ(1, blockAccess->m_StrongReferenceCount);
	EXPECT_EQ(1, blockAccess->m_WeakReferenceCount);
	EXPECT_EQ(true, blockAccess->m_Valid);
}

TEST(PointerOwnerTests, FreeMemory) {
	size_t startrss = getCurrentRSS();
	std::vector<ptr_owner<Foo>> foovector;
	for (int i = 0; i < 1000; i++) foovector.push_back(PointerCreator::createPtrOwner<Foo>(i, 2000 - i));
	size_t afterrss = getCurrentRSS();
	foovector.clear();
	size_t clearrss = getCurrentRSS();

	EXPECT_LE(startrss, afterrss);
	EXPECT_GT(afterrss, clearrss);
}

TEST(PointerOwnerTests, Collections) {
	std::vector<ptr_owner<Foo>> foovector;
	foovector.resize(1);
	foovector[0] = PointerCreator::createPtrOwner<Foo>(5, 8);

	PointerOwnerAccess<Foo>* access = (PointerOwnerAccess<Foo>*)&foovector[0];
	ControlBlockAccess* blockAccess = (ControlBlockAccess*)access->m_ControlBlock;

	std::cout << "Access Casting Done" << std::endl;

	EXPECT_EQ(5, access->m_Data->value0);
	EXPECT_EQ(8, access->m_Data->value1);
	EXPECT_EQ(1, blockAccess->m_StrongReferenceCount);
	EXPECT_EQ(1, blockAccess->m_WeakReferenceCount);
	EXPECT_EQ(true, blockAccess->m_Valid);

	foovector.push_back(PointerCreator::createPtrOwner<Foo>(6, 11));

	access = (PointerOwnerAccess<Foo>*)&foovector[1];
	blockAccess = (ControlBlockAccess*)access->m_ControlBlock;

	EXPECT_EQ(6, access->m_Data->value0);
	EXPECT_EQ(11, access->m_Data->value1);
	EXPECT_EQ(1, blockAccess->m_StrongReferenceCount);
	EXPECT_EQ(1, blockAccess->m_WeakReferenceCount);
	EXPECT_EQ(true, blockAccess->m_Valid);

	foovector.emplace_back(PointerCreator::createPtrOwner<Foo>(7, 12));

	access = (PointerOwnerAccess<Foo>*) & foovector[2];
	blockAccess = (ControlBlockAccess*)access->m_ControlBlock;

	EXPECT_EQ(7, access->m_Data->value0);
	EXPECT_EQ(12, access->m_Data->value1);
	EXPECT_EQ(1, blockAccess->m_StrongReferenceCount);
	EXPECT_EQ(1, blockAccess->m_WeakReferenceCount);
	EXPECT_EQ(true, blockAccess->m_Valid);

	foovector.clear();	
}