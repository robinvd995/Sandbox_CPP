#include "pch.h"

using namespace Jupiter;

TEST(AllignmentTests, CalculateForwardAdjustment) {
	int value0 = 1;
	void* ptr0 = &value0;
	uintptr_t address0 = reinterpret_cast<uintptr_t>(ptr0) & 0xFF;

	uint8 alignment0 = 32;
	uint8 adjustment0 = pointer_math::calc_forward_alignment_adjustment(ptr0, alignment0);

	std::cout << "Memory adress = " << std::hex << address0 << std::endl;
	std::cout << "Adjustment = " << std::hex << (uint32)adjustment0 << std::endl;
	std::cout << "Aligned memory = " << std::hex << (address0 + adjustment0) << std::endl;

	EXPECT_TRUE(((address0 + adjustment0) % alignment0 == 0));
}
