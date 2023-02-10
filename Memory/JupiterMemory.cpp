#include "JupiterMemory.h"

namespace Jupiter {

	ptr_control_block* ptr_control_block::create() {
		ptr_control_block* ctrlBlock = new ptr_control_block();
		ctrlBlock->m_Valid = true;
		ctrlBlock->m_ReferenceCount = 1;
		return ctrlBlock;
	}

	void ptr_control_block::release(ptr_control_block* controlBlock) {
		controlBlock->m_ReferenceCount--;
		if (controlBlock->m_ReferenceCount == 0) {
			delete controlBlock;
		}
	}

}