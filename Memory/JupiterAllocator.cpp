#include "JupiterAllocator.h"

#include "JupiterAllocatorExceptions.h"

#include <stdlib.h>

namespace Jupiter {

	// This construct is kindoff a mess, clean this up soontm
	StackAllocator::StackAllocator(size_t size) {
		void* data = nullptr;
		m_UsedMemory = 0;
		m_Allocations = 0;

		// Allocate the memory
		data = malloc(size);

		// Check if memory allocation was successfull
		if (data == nullptr) {
			// Failed to allocate memory!
			m_Start = nullptr;
			m_Size = 0;

			m_Top = nullptr;
		}
		else {
			// Memory allocation success
			m_Start = data;
			m_Size = size;

			// Set the top of the stack to the start of the stack
			m_Top = m_Start;
		}
	}

	StackAllocator::~StackAllocator() {
		if (m_Allocations != 0) {
			// Warn the user that the allocator was deleted but not all allocations were freed
		}
		if (m_UsedMemory != 0) {
			// Warn the user that the allocator was deleted but not all memory was freed
		}
		free(m_Start);
	}

	void* StackAllocator::allocate(size_t size, uint8 allignment) {
		// Calculate the adjustment based on the current top of the stack and the desired allignment
		uint8 adjustment = pointer_functions::calc_forward_alignment_adjustment(m_Top, allignment);

		// Calculate the total size of the block by adding the adjustment and the payload size
		size_t totalSize = size + adjustment;

		// Get the new top of the stack by adding the total size of the allocated memory to the current top of the stack
		void* top = pointer_functions::shift_forward(m_Top, totalSize);

		// The new size exceeded the amount allocated memory in this allocator!
		if (!(reinterpret_cast<uintptr_t>(top) < m_Size)) {
			throw std::bad_alloc();
		}

		// Get the start of the newly allocated block of memory adjusted for alignment
		void* start = pointer_functions::shift_forward(m_Top, adjustment);

		// Set the new top, increment the number of allocations and add the total size to used memory
		m_Top = top;
		m_Allocations++;
		m_UsedMemory += totalSize;

		// Return the start of the allocated memory block
		return start;
	}

	void StackAllocator::deallocate(void* p) {
		// Maybe allow this function to deallocates, and treat the pointer 'p' as a stack marker.
		// This is more error prone and I am not entirly sure if it matters or not.
		// Could be usefull to some degree.
		// Could also just delete the entire stack if the pointer given was a nullptr, this might come in usefull
		// when allocators are gonna be pooled in a vector or something
		throw jpt_bad_free("Stack allocator cannot deallocate memory using this function, use clear or free instead!");
	}

	void StackAllocator::deallocate(void* p, size_t size) {
		throw jpt_bad_free("Stack allocator cannot deallocate memory using this function, use clear or free instead!");
	}

	StackAllocator::stack_marker StackAllocator::mark() {
		// Calculate the block size and get the alignment value of the stack_marker
		size_t blockSize = sizeof(m_UsedMemory) + sizeof(m_Allocations);
		uint8 alignment = __alignof(m_UsedMemory);

		// Allocate the marker, and get the the memory addresses of the values to set
		void* marker = allocate(blockSize, alignment);
		size_t* usedMemAddress = reinterpret_cast<size_t*>(marker);
		size_t* allocationsAddress = reinterpret_cast<size_t*>(pointer_functions::shift_forward(marker, sizeof(m_UsedMemory)));

		// Set the values
		*usedMemAddress = m_UsedMemory;
		*allocationsAddress = m_Allocations;

		// Return the marker
		return marker;
	}

	void StackAllocator::freeForward(stack_marker marker) {
		// Get the memory addresses of the values that are snapshot during the creation of the marker
		size_t* usedMemoryAddress = reinterpret_cast<size_t*>(marker);
		size_t* allocationsAddress = reinterpret_cast<size_t*>(pointer_functions::shift_forward(marker, sizeof(m_UsedMemory)));

		// Set the values to the snapshot values
		m_UsedMemory = *usedMemoryAddress;
		m_Allocations = *allocationsAddress;

		// Set the top of the stack to the address after the marker
		size_t blockSize = sizeof(m_UsedMemory) + sizeof(m_Allocations);
		m_Top = pointer_functions::shift_forward(marker, blockSize);
	}

	void StackAllocator::clear() {
		m_Top = m_Start;
		m_UsedMemory = 0;
		m_Allocations = 0;
	}
}
