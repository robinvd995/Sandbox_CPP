#include "JupiterAllocator.h"

#include "JupiterAllocatorExceptions.h"

#include <stdlib.h>

namespace Jupiter {

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
		free(m_Start);
	}

	void* StackAllocator::allocate(size_t size, uint8 allignment) {
		// Calculate the adjustment based on the current top of the stack and the desired allignment
		uint8 adjustment = pointer_functions::calc_forward_alignment_adjustment(m_Top, allignment);

		// Calculate the total size of the block by adding the adjustment and the payload size
		size_t totalSize = size + adjustment;

		// Get the new top of the stack by adding the total size of the allocated memory to the current top of the stack
		void* top = pointer_functions::add(m_Top, totalSize);

		// The new size exceeded the amount allocated memory in this allocator!
		if (!(reinterpret_cast<uintptr_t>(top) < m_Size)) {
			throw std::bad_alloc();
		}

		// Get the start of the newly allocated block of memory adjusted for alignment
		void* start = pointer_functions::add(m_Top, adjustment);

		// Set the new top, increment the number of allocations and add the total size to used memory
		m_Top = top;
		m_Allocations++;
		m_UsedMemory += totalSize;

		// Return the start of the allocated memory block
		return start;
	}

	void StackAllocator::deallocate(void* p) {
		throw jpt_bad_free("Stack allocator cannot deallocate memory using this function, use clear or free instead!");
	}

	void StackAllocator::deallocate(void* p, size_t size) {
		throw jpt_bad_free("Stack allocator cannot deallocate memory using this function, use clear or free instead!");
	}

}
