#pragma once

namespace Jupiter {

	uint8 pointer_functions::calc_forward_alignment_adjustment(void* ptr, uint8 alignment) {
		uint8 alignmentMask = alignment - 1;
		uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
		uint8 adjustment = alignment - (address & alignmentMask);
//		if (adjustment == alignment) return 0; Maybe this needs to be here because now when the address is already aligned, 
//		the adjustment is equal to the alignment, which means there is memory waste equal to the alignment,
//		could also just modulo the adjustment with the alignment, so the alignment is never greater then the alignment
//		But modulo operations are generally more expensive to do so probably not
		return adjustment;
	}

	void* pointer_functions::shift_forward(void* ptr, size_t x) {
		return (void*)(reinterpret_cast<uintptr_t>(ptr) + x);
	}

	const void* pointer_functions::shift_forward(const void* ptr, size_t x) {
		return (const void*)(reinterpret_cast<uintptr_t>(ptr) + x);
	}

	void* pointer_functions::shift_back(void* ptr, size_t x) {
		return (void*)(reinterpret_cast<uintptr_t>(ptr) - x);
	}

	const void* pointer_functions::shift_back(const void* ptr, size_t x) {
		return (const void*)(reinterpret_cast<uintptr_t>(ptr) - x);
	}
}