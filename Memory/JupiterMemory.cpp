#include "JupiterMemory.h"

#define MEMORY_EXCEPTION_CHECKING

#ifdef MEMORY_EXCEPTION_CHECKING
#include <stdexcept>
#endif //MEMORY_EXCEPTION_CHECKING

namespace Jupiter {

//	ptr_control_block* ptr_control_block::create() {
//		ptr_control_block* ctrlBlock = new ptr_control_block();
//		ctrlBlock->m_Valid = true;
//		ctrlBlock->m_ReferenceCount = 1;
//		return ctrlBlock;
//	}
//
//	void ptr_control_block::release(ptr_control_block* controlBlock) {
//		controlBlock->m_ReferenceCount--;
//		if (controlBlock->m_ReferenceCount == 0) {
//			delete controlBlock;
//		}
//	}

	ptr_control_block* ptr_control_block::create() noexcept {
		// Create new control block
		ptr_control_block* block = new ptr_control_block();

		// Set the initial variables
		block->m_StrongReferenceCount = 1;
		block->m_WeakReferenceCount = 1;
		block->m_Valid = true;

		// return the control block
		return block;
	}

	void ptr_control_block::releaseWeak(ptr_control_block* controlBlock) {
		// If the exception check flag is defined, check for errors
#ifdef MEMORY_EXCEPTION_CHECKING
		// If the weak reference count is equal to the strong reference count throw an exception since the weak 
		// reference count should always to larger or equal to the strong reference count
		if (controlBlock->m_WeakReferenceCount == controlBlock->m_StrongReferenceCount)
			throw new std::out_of_range("Trying to release a weak reference when its count is equal to the strong reference count!");
#endif //MEMORY_EXCEPTION_CHECKING

		// Decrement the weak reference counter
		controlBlock->m_WeakReferenceCount--;

		// If the weak reference count = 0, delete the control block
		if (controlBlock->m_WeakReferenceCount == 0)
			delete controlBlock;
	}

	bool Jupiter::ptr_control_block::releaseStrong(ptr_control_block* controlBlock) {
		// Initialize a flag to return
		bool flag = false;
		
#ifdef MEMORY_EXCEPTION_CHECKING // If the exception check flag is defined, check for errors
		// if the strong reference count is equal to zero, throw an exception since there are no strong references left to release
		if (controlBlock->m_StrongReferenceCount == 0)
			throw new std::out_of_range("Trying to release a strong reference when the reference count is 0!");
#endif //MEMORY_EXCEPTION_CHECKING

		// Decrement the strong reference counter
		controlBlock->m_StrongReferenceCount--;

		// If the strong reference count = 0, set the flag to true so the caller knows it should delete the data
		if (controlBlock->m_StrongReferenceCount == 0) {
			flag = true;

			// Set the valid flag to false, if there are still any weak references left, so they know the data is invalid
			controlBlock->m_Valid = false;
		}

		// Decrement the weak reference counter
		controlBlock->m_WeakReferenceCount--;

		// If the weak reference count = 0, delete the control block
		if (controlBlock->m_WeakReferenceCount == 0)
			delete controlBlock;

		return flag;
	}

	void ptr_control_block::incrementWeak(ptr_control_block* controlBlock) noexcept {
		// Increment the weak reference counter
		controlBlock->m_WeakReferenceCount++;
	}

	void ptr_control_block::incrementStrong(ptr_control_block* controlBlock) noexcept {
		// Increment the weak reference counter
		controlBlock->m_WeakReferenceCount++;

		// Increment the strong reference counter
		controlBlock->m_StrongReferenceCount++;
	}

}