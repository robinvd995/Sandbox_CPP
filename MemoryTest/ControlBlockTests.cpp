#include "pch.h"

using namespace Jupiter;

#define MEMORY_EXCEPTION_CHECKING

class ControlBlockAccess {

public:
	uint m_StrongReferenceCount = 0;
	uint m_WeakReferenceCount = 0;
	bool m_Valid = false;
};

TEST(ControlBlockTest, Access) {
	ptr_control_block* block = ptr_control_block::create();
	ControlBlockAccess* access = (ControlBlockAccess*)block;

	EXPECT_EQ(1, access->m_StrongReferenceCount);
	EXPECT_EQ(1, access->m_WeakReferenceCount);
	EXPECT_EQ(true, access->m_Valid);

	delete access;
}

TEST(ControlBlockTest, Create) {
	ptr_control_block* block = ptr_control_block::create();
	ControlBlockAccess* access = (ControlBlockAccess*)block;

	EXPECT_EQ(1, access->m_StrongReferenceCount);
	EXPECT_EQ(1, access->m_WeakReferenceCount);
	EXPECT_EQ(true, access->m_Valid);

	delete access;
}

TEST(ControlBlockTest, ReleaseStrong) {
	ptr_control_block* block0 = ptr_control_block::create();
	bool deleted0 = ptr_control_block::releaseStrong(block0);
	EXPECT_EQ(true, deleted0);

	ptr_control_block* block1 = ptr_control_block::create();		// StrongRefCounter = 1
	ControlBlockAccess* access1 = (ControlBlockAccess*)block1;
	EXPECT_EQ(1, access1->m_StrongReferenceCount);
	EXPECT_EQ(1, access1->m_WeakReferenceCount);

	ptr_control_block::incrementStrong(block1);						// StrongRefCounter = 2
	EXPECT_EQ(2, access1->m_StrongReferenceCount);
	EXPECT_EQ(2, access1->m_WeakReferenceCount);
	ptr_control_block::incrementStrong(block1);						// StrongRefCounter = 3
	EXPECT_EQ(3, access1->m_StrongReferenceCount);
	EXPECT_EQ(3, access1->m_WeakReferenceCount);

	ptr_control_block::incrementStrong(block1);						// StrongRefCounter = 4
	EXPECT_EQ(4, access1->m_StrongReferenceCount);
	EXPECT_EQ(4, access1->m_WeakReferenceCount);

	EXPECT_EQ(false, ptr_control_block::releaseStrong(block1));		// StrongRefCounter = 3
	EXPECT_EQ(3, access1->m_StrongReferenceCount);
	EXPECT_EQ(3, access1->m_WeakReferenceCount);

	EXPECT_EQ(false, ptr_control_block::releaseStrong(block1));		// StrongRefCounter = 2
	EXPECT_EQ(2, access1->m_StrongReferenceCount);
	EXPECT_EQ(2, access1->m_WeakReferenceCount);

	EXPECT_EQ(false, ptr_control_block::releaseStrong(block1));		// StrongRefCounter = 1
	EXPECT_EQ(1, access1->m_StrongReferenceCount);
	EXPECT_EQ(1, access1->m_WeakReferenceCount);

	EXPECT_EQ(true, ptr_control_block::releaseStrong(block1));		// StrongRefCounter = 0

	// These expected values might potentially be wrong on different systems / instances
	// Goal is to check for the succesfull deletion of the memory block
	EXPECT_EQ(-572662307, access1->m_StrongReferenceCount);	
	EXPECT_EQ(-572662307, access1->m_WeakReferenceCount);
}

TEST(ControlBlockTest, ReleaseWeak) {

	// Check for an exception throw if release weak is called when weak ref count is equal to strong ref count
#ifdef MEMORY_EXCEPTION_CHECKING
	ptr_control_block* block0 = ptr_control_block::create();
	EXPECT_ANY_THROW(ptr_control_block::releaseWeak(block0));
	ptr_control_block::releaseStrong(block0);		// Release strong to delete the pointer block
#endif //MEMORY_EXCEPTION_CHECKING

	ptr_control_block* block1 = ptr_control_block::create();
	ControlBlockAccess* access1 = (ControlBlockAccess*)block1;
	EXPECT_EQ(1, access1->m_WeakReferenceCount);
	EXPECT_EQ(1, access1->m_StrongReferenceCount);

	ptr_control_block::incrementWeak(block1);
	EXPECT_EQ(2, access1->m_WeakReferenceCount);
	EXPECT_EQ(1, access1->m_StrongReferenceCount);

	ptr_control_block::incrementWeak(block1);
	EXPECT_EQ(3, access1->m_WeakReferenceCount);
	EXPECT_EQ(1, access1->m_StrongReferenceCount);

	ptr_control_block::incrementWeak(block1);
	EXPECT_EQ(4, access1->m_WeakReferenceCount);
	EXPECT_EQ(1, access1->m_StrongReferenceCount);

	ptr_control_block::releaseWeak(block1);
	EXPECT_EQ(3, access1->m_WeakReferenceCount);
	EXPECT_EQ(1, access1->m_StrongReferenceCount);

	ptr_control_block::releaseWeak(block1);
	EXPECT_EQ(2, access1->m_WeakReferenceCount);
	EXPECT_EQ(1, access1->m_StrongReferenceCount);

	ptr_control_block::releaseWeak(block1);
	EXPECT_EQ(1, access1->m_WeakReferenceCount);
	EXPECT_EQ(1, access1->m_StrongReferenceCount);

	ptr_control_block::releaseStrong(block1); // Effectivly deleted the block
}

TEST(ControlBlockTest, InvalidateWithWeakLeftover) {
	ptr_control_block* block0 = ptr_control_block::create();
	ptr_control_block::incrementWeak(block0);
	EXPECT_EQ(true, block0->isValid());
	ptr_control_block::releaseStrong(block0);
	EXPECT_EQ(false, block0->isValid());
	ptr_control_block::releaseWeak(block0);
}
