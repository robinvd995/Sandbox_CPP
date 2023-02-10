#pragma once

#include <iostream>
#include <vector>

typedef int uint;

// in case of a situation where 1 object owns a pointer and other object grab references to said pointer:
// The owner is the only class that is allowed to create a control block in this situation
// The reference is not allowed to set the value of the control block valid flag, only read it
// When the owner is validated, a control block is created and the reference counter is set to 1, and the valid flag to true
// When a reference is grabbed from the owner the reference counter within the control block is incremented by 1
// When a reference is destroyed, the reference counter will be decremented by 1
// When the owner is destroyed, the reference counter will be decremented by 1, and the valid flag will be set to false
// When to owner or the reference is destroyed, and the reference counter is 0, the control block will be released from memory
// 

namespace Jupiter {

	class ptr_control_block {

	public:
		ptr_control_block() { std::cout << "Control Block Created!" << std::endl; }
		~ptr_control_block() { std::cout << "Control Block Destroyed!" << std::endl; }

		/// <summary>
		/// Creates a new control block object
		/// </summary>
		/// <returns>A pointer to the now object created</returns>
		static ptr_control_block* create();

		/// <summary>
		/// Releases a reference to the control block
		/// </summary>
		/// <param name="controlBlock">The control block the where a reference needs to be released</param>
		static void release(ptr_control_block* controlBlock);

		/// <summary>
		/// Invalidates the control block, setting the m_Valid flag to false;
		/// </summary>
		inline void invalidate() { m_Valid = false; }

		/// <summary>
		/// Checks if the current pointer that this block controls is still valid
		/// </summary>
		/// <returns>True if the pointer is still valid</returns>
		inline bool isValid() { return m_Valid; }

	public:
		uint m_ReferenceCount = 0;
		bool m_Valid = false;
	};

	/// <summary>
	/// A reference pointer object pointing to data but does not control it
	/// </summary>
	/// <typeparam name="T">The type of the data</typeparam>
	template<typename T>
	class ptr_reference {

	public:
		// Only acceptable constructor is the one where a data pointer is given to the object
		ptr_reference(T* data, ptr_control_block* controlBlock) : 
			m_ReferencedData(data), 
			m_ControlBlock(controlBlock) 
		{
			m_ControlBlock->m_ReferenceCount++;
			std::cout << "ptr_reference argument constructor called" << std::endl;
		}

		ptr_reference(const ptr_reference<T>& other) : 
			m_ReferencedData(other.m_ReferencedData), 
			m_ControlBlock(other.m_ControlBlock) 
		{
			m_ControlBlock->m_ReferenceCount++; 
			std::cout << "ptr_reference copy constructor called" << std::endl;
		}

	public:
		ptr_reference() { std::cout << "ptr_reference constructor called" << std::endl; }
		~ptr_reference() { ptr_control_block::release(m_ControlBlock); std::cout << "ptr_reference destructor called" << std::endl; }

		/// <summary>
		/// Checks if the reference is valid
		/// This means that the control block != nullptr and the control block is valid aswell
		/// </summary>
		/// <returns>true if both the control block is not nullptr and the block is valid</returns>
		bool isValid() { return (m_ControlBlock) && (m_ControlBlock->isValid()); }

	public:
		// Operator used to acces a const of the raw pointer data.
		// since references should not modify the data the return type is defined as const
		const T* operator->() const { return m_ReferencedData; }

	private:
		T* m_ReferencedData = nullptr;
		ptr_control_block* m_ControlBlock = nullptr;
	};

	/// <summary>
	/// Wrapper for a pointer that is owned by a single entity
	/// </summary>
	/// <typeparam name="T">The type of the data</typeparam>
	template<typename T>
	class ptr_owned {

	private:
		// Only acceptable constructor is the one where a data pointer is given to the object
		ptr_owned(T* data) : m_Data(data) {
			m_ControlBlock = ptr_control_block::create();
			std::cout << "ptr_owned argument constructor called" << std::endl;
		}

	public:
		// Delete default constructor
		ptr_owned() = delete;

		// Delete copy constructor
		ptr_owned(const ptr_owned<T>& other) = delete;

		// Move constructor
		ptr_owned(ptr_owned<T>&& other) noexcept : 
			m_Data(std::move(other.m_Data)), 
			m_ControlBlock(std::move(other.m_ControlBlock)) 
		{ 
			m_ControlBlock->m_ReferenceCount++; 
			std::cout << "ptr_owned move constructor called" << std::endl;
		}

		// Delete pointer data in destructor, also invalidate the control block and decrement the reference counter of said control block
		~ptr_owned() { ptr_control_block::release(m_ControlBlock); std::cout << "ptr_owned destructor called" << std::endl; }

		// Default move assignment operator
		ptr_owned<T>& operator=(ptr_owned<T>&&) = default;

		// Delete copy assignment operator
		ptr_owned<T>& operator=(ptr_owned<T>&) = delete;

	public:
		// Operator used to acces the raw pointer data
		T* operator->() const { return m_Data; }

	public:
		/// <summary>
		/// Grabs a reference to this pointer.
		/// A reference is not in control of the data, and can therefore never delete the data or invalidate it
		/// </summary>
		/// <returns></returns>
		inline ptr_reference<T> grabReference() {
			return ptr_reference<T>(m_Data, m_ControlBlock);
		}

		/// <summary>
		/// Invalidates the pointer object/ptr_control_block and deletes the data.
		/// The control block will not be deleted when calling this.
		/// All references derived from this pointer still hold the control block data.
		/// However the references should always check if the control block is still valid before using the data.
		/// This method always needs to be called, otherwise the data that this is pointing towards will never be deleted!
		/// </summary>
		void invalidate() {
			m_ControlBlock->invalidate();
			ptr_control_block::release(m_ControlBlock);
			delete m_Data;
		}

	private:
		T* m_Data;
		ptr_control_block* m_ControlBlock;

		friend class JupiterMemory;
	};

	/// <summary>
	/// A pointer object that shares the ownership of the data with other objects
	/// </summary>
	/// <typeparam name="T">The type of the data</typeparam>
	template<typename T>
	class ptr_shared {

	public:

	};

	class JupiterMemory {

	public:
		template<typename T, typename ...Args>
		static ptr_owned<T> createOwned(Args&&... args) {
			T* data = new T(std::forward<Args>(args)...);
			return ptr_owned<T>(data);
		}
	};
}
