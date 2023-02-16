#pragma once

#include <iostream>
#include <vector>

typedef unsigned int uint;

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

	/// <summary>
	/// A control block for managing pointers
	/// The strong reference counter is in control of the data, meaning that when the strong ref counter hits 0 the data will be deleted
	/// The weak reference counter is in control of the control block itself, meaning that when the weak ref counter hits 0, the block will be deleted
	/// A strong reference is always a weak reference, but a weak reference is never a strong reference
	/// </summary>
	class ptr_control_block {

	private:
		ptr_control_block() noexcept { std::cout << "Control Block Created!" << std::endl; }
		~ptr_control_block() noexcept { std::cout << "Control Block Destroyed!" << std::endl; }

	public:
		/// <summary>
		/// Creates a new control block object
		/// This sets the counter for StrongReference to 1
		/// This sets the counter for WeakReference to 1
		/// This sets the valid flag to true
		/// </summary>
		/// <returns>A pointer to the newly created control block</returns>
		static ptr_control_block* create() noexcept;

		/// <summary>
		/// Releases a weak reference to the control block
		/// If the WeakReference counter is 0 after the decrement, the control block will be deleted
		/// Throws an std::out_of_range exception when the weak ref counter is equal to the strong ref counter
		/// </summary>
		/// <param name="controlBlock">The control block the where a reference needs to be released</param>
		static void releaseWeak(ptr_control_block* controlBlock);

		/// <summary>
		/// Releases a strong and weak reference to the control block
		/// If the StrongReference counter is 0 after the decrement, the data will be deleted and the valid flag will be set to false
		/// If the WeakReference counter is 0 after the decrement, the control block will be deleted
		/// Throws an std::out_of_range exception when the strong ref counter is 0
		/// </summary>
		/// <param name="controlBlock">The control block the where a reference needs to be released</param>
		/// <returns>True if the strong reference counter is 0 after the decrement</returns>
		static bool releaseStrong(ptr_control_block* controlBlock);

		/// <summary>
		/// Increments the weak reference counter by 1
		/// </summary>
		/// <param name="controlBlock">The control block where the weak reference need to be incremented</param>
		static void incrementWeak(ptr_control_block* controlBlock) noexcept;

		/// <summary>
		/// Incremenets the strong and weak reference counter by 1
		/// </summary>
		/// <param name="controlBlock">The control block where the strong and weak reference need to be incremented</param>
		static void incrementStrong(ptr_control_block* controlBlock) noexcept;

		/// <summary>
		/// Checks if the current pointer that this block controls is still valid
		/// </summary>
		/// <returns>True if the pointer is still valid</returns>
		inline bool isValid() noexcept { return m_Valid; }

	private:
		uint m_StrongReferenceCount = 0;		// Strong reference count is how many owners there are of this pointer
		uint m_WeakReferenceCount = 0;			// Weak reference count is how many references are there to this pointer
		bool m_Valid = false;					// Flag depending wether or not the control block points to valid data or not
	};

	/// <summary>
	/// 
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T>
	class ptr_reference {

	private:
		// Only acceptable contructor for creating a new reference
		ptr_reference(T* data, ptr_control_block* ctrlBlock) : m_ReferencedData(data), m_ControlBlock(ctrlBlock) {
			// A new reference is created, increment the weak ref counter
			ptr_control_block::incrementWeak(m_ControlBlock);
		}

	public:
		// Default constructor
		ptr_reference() : m_ReferencedData(nullptr), m_ControlBlock(nullptr) {}

		// Destructor
		~ptr_reference() {
			// A reference is released, decrement the weak ref counter
			ptr_control_block::releaseWeak(m_ControlBlock);
		}

		// Copy constructor
		ptr_reference(const ptr_reference<T>& other) : m_ReferencedData(other->m_ReferencedData), m_ControlBlock(other->m_ControlBlock) {
			// A copy of the reference is made, increment the weak ref counter
			ptr_control_block::incrementWeak(m_ControlBlock);
		}

		// Move constructor, the move constructor should generally not be called, use the copy constructor instead
		// May be deprecated later
		ptr_reference(ptr_reference<T>&& other) : m_ReferencedData(std::move(other.m_ReferencedData)), m_ControlBlock(std::move(other->m_ControlBlock)) {
			// A new instance of the reference is made, increment the weak ref counter
			ptr_control_block::incrementWeak(m_ControlBlock);
		}

		// Copy assignment operator
		ptr_reference<T>& operator=(ptr_reference<T>& other) {
			// Copy member variables
			m_ReferencedData = other.m_ReferencedData;
			m_ControlBlock = other.m_ControlBlock;

			// A copy is made, increment the weak ref counter
			ptr_control_block::incrementWeak();
		}

		// Move assignment operator, this operator should generally not be called, use the copy assignment operator instead
		// May be deperecated later
		ptr_reference<T>& operator=(ptr_reference<T>&& other) {
			// Move member variables
			m_ReferencedData = std::move(other.m_ReferencedData);
			m_ControlBlock = std::move(other.m_ControlBlock);

			// A new instance is made, increment the weak ref counter
			ptr_control_block::incrementWeak();
		}

		// Operator used to acces the raw pointer data
		T* operator->() const { return m_ReferencedData; }

	private:
		T* m_ReferencedData;
		ptr_control_block* m_ControlBlock;

		friend class PointerCreator;
	};

	/// <summary>
	/// 
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T>
	class ptr_owner {

	private:
		// Argument constructor, only constructor allowed for creating a control block
		ptr_owner(T* data) : m_Data(data) {
			// Create the control block
			m_ControlBlock = ptr_control_block::create();
		}

	public:
		// Default empty constructor, data and control block are nullptrs
		ptr_owner() : m_Data(nullptr), m_ControlBlock(nullptr) {}

		// Copy constructor, delete because only 1 pointer owner is allowed to control the data
		ptr_owner(const ptr_owner<T>& other) = delete;

		// Move constructor
		ptr_owner(ptr_owner<T>&& other) noexcept : m_Data(std::move(other.m_Data)), m_ControlBlock(std::move(other.m_ControlBlock)) {
			// increment the control block strong ref by 1, 
			// since the destructor is called after the move is complete the ref counter should still be 1
			// ??? Maybe there is a more ellegant solution for this ???
			ptr_control_block::incrementStrong(m_ControlBlock);
			std::cout << "Move Constructor!" << std::endl;
		}

		// Destructor
		~ptr_owner() {
			// Release the strong reference to this pointer object, resulting in deleting the data controlled by this if strong ref = 0
			// Calling relrease strong returns true if the strong ref counter is 0, therefore we must delete the data
			if (ptr_control_block::releaseStrong(m_ControlBlock))
				delete m_Data;
		}

		// Move assignment operator
		ptr_owner<T>& operator=(ptr_owner<T>&& other) noexcept {
			m_Data = std::move(other.m_Data);
			m_ControlBlock = std::move(other.m_ControlBlock);
			ptr_control_block::incrementStrong(m_ControlBlock);
			std::cout << "Move assignment operator" << std::endl;
			return *this;
		}

		// Delete copy assignment operator, since we don't want to make copies of this pointer
		ptr_owner<T>& operator=(ptr_owner<T>&) = delete;

		// Operator used to acces the data
		T* operator->() const { return m_Data; }

	private:
		T* m_Data;
		ptr_control_block* m_ControlBlock;

		friend class PointerCreator;
	};

	/// <summary>
	/// 
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T>
	class ptr_shared {

	private:
		// Argument constructor, only constructor allowed to create a control block
		ptr_shared(T* data) : m_SharedData(data) {
			m_ControlBlock = ptr_control_block::create();
		}

	public:
		// Default constructor, initializes both data and control block to nullptr
		ptr_shared() : m_SharedData(nullptr), m_ControlBlock(nullptr) {}

		// Copy constructor, increment strong ref count
		ptr_shared(const ptr_shared<T>& other) : m_SharedData(other.m_SharedData), m_ControlBlock(other.m_ControlBlock) {
			ptr_control_block::incrementStrong(m_ControlBlock);
		}

		// Move constructor, increment strong ref count
		ptr_shared(ptr_shared<T>&& other) : m_SharedData(std::move(other.m_SharedData)), m_ControlBlock(std::move(other.m_ConstrolBlock)) {
			ptr_control_block::incrementStrong(m_ControlBlock);
		}

		// Destructor, release strong ref and delete data when strong ref count = 0;
		~ptr_shared() {
			if (ptr_control_block::releaseStrong(m_ControlBlock))
				delete m_SharedData;
		}

		// Copy assignment operator, increment strong ref count
		ptr_shared<T>& operator=(ptr_shared<T>& other) {
			// Copy the data
			m_SharedData = other.m_SharedData;
			m_ControlBlock = other.m_ControlBlock;

			// Increment strong ref count
			ptr_control_block::incrementStrong(m_ControlBlock);

			// Return dereferenced this
			return *this;
		}

		// Move assignment operator, increment strong ref count
		ptr_shared<T>& operator=(ptr_shared<T>&& other) {
			// Move the data
			m_SharedData = std::move(other.m_SharedData);
			m_ControlBlock = std::move(other.m_ControlBlock);

			// Increment strong ref count
			ptr_control_block::incrementStrong(m_ControlBlock);

			// Return dereferenced this
			return *this;
		}

		// Operator used to access the data
		T* operator->() const { return m_SharedData; }

	private:
		T* m_SharedData;
		ptr_control_block* m_ControlBlock;

		friend class PointerCreator;

	};

	/// <summary>
	/// Class used to instantiate pointer objects
	/// Class should never be instantiated
	/// </summary>
	class PointerCreator {
		
	private:
		PointerCreator() = delete;								// Constructor deletion
		~PointerCreator() = delete;								// Destructor deletion
		PointerCreator(const PointerCreator&) = delete;			// Constructor deletion
		PointerCreator(const PointerCreator&&) = delete;		// Constructor deletion
		PointerCreator& operator=(PointerCreator&) = delete;	// Delete copy assignment operator
		PointerCreator& operator=(PointerCreator&&) = delete;	// Delete move assignment operator

	public:

		template<typename T, typename ...Args>
		static ptr_owner<T> createPtrOwner(Args&&... args) {
			T* data = new T(std::forward<Args>(args)...);
			return ptr_owner<T>(data);
		}

		template<typename T, typename ...Args>
		static ptr_shared<T> createPtrShared(Args&&... args) {
			T* data = new T(std::forward<Args>(args)...);
			return ptr_shared<T>(data);
		}

		template<typename T>
		static ptr_reference<T> grabPtrReference(ptr_owner<T> ptr) {
			return ptr_reference<T>(ptr.m_Data, ptr.m_ControlBlock);
		}

		template<typename T>
		static ptr_reference<T> grabPtrReference(ptr_shared<T> ptr) {
			return ptr_reference<T>(ptr.m_Data, ptr.m_ControlBlock);
		}
	};


//	/// <summary>
//	/// A reference pointer object pointing to data but does not control it
//	/// </summary>
//	/// <typeparam name="T">The type of the data</typeparam>
//	template<typename T>
//	class ptr_reference {
//
//	public:
//		// Only acceptable constructor is the one where a data pointer is given to the object
//		ptr_reference(T* data, ptr_control_block* controlBlock) : 
//			m_ReferencedData(data), 
//			m_ControlBlock(controlBlock) 
//		{
//			m_ControlBlock->m_ReferenceCount++;
//			std::cout << "ptr_reference argument constructor called" << std::endl;
//		}
//
//		ptr_reference(const ptr_reference<T>& other) : 
//			m_ReferencedData(other.m_ReferencedData), 
//			m_ControlBlock(other.m_ControlBlock) 
//		{
//			m_ControlBlock->m_ReferenceCount++; 
//			std::cout << "ptr_reference copy constructor called" << std::endl;
//		}
//
//	public:
//		ptr_reference() { std::cout << "ptr_reference constructor called" << std::endl; }
//		~ptr_reference() { ptr_control_block::release(m_ControlBlock); std::cout << "ptr_reference destructor called" << std::endl; }
//
//		/// <summary>
//		/// Checks if the reference is valid
//		/// This means that the control block != nullptr and the control block is valid aswell
//		/// </summary>
//		/// <returns>true if both the control block is not nullptr and the block is valid</returns>
//		bool isValid() { return (m_ControlBlock) && (m_ControlBlock->isValid()); }
//
//	public:
//		// Operator used to acces a const of the raw pointer data.
//		// since references should not modify the data the return type is defined as const
//		const T* operator->() const { return m_ReferencedData; }
//
//	private:
//		T* m_ReferencedData = nullptr;
//		ptr_control_block* m_ControlBlock = nullptr;
//	};
//
//	/// <summary>
//	/// Wrapper for a pointer that is owned by a single entity
//	/// </summary>
//	/// <typeparam name="T">The type of the data</typeparam>
//	template<typename T>
//	class ptr_owned {
//
//	private:
//		// Only acceptable constructor is the one where a data pointer is given to the object
//		ptr_owned(T* data) : m_Data(data) {
//			m_ControlBlock = ptr_control_block::create();
//			std::cout << "ptr_owned argument constructor called" << std::endl;
//		}
//
//	public:
//		// Delete default constructor
//		ptr_owned() = delete;
//
//		// Delete copy constructor
//		ptr_owned(const ptr_owned<T>& other) = delete;
//
//		// Move constructor
//		ptr_owned(ptr_owned<T>&& other) noexcept : 
//			m_Data(std::move(other.m_Data)), 
//			m_ControlBlock(std::move(other.m_ControlBlock)) 
//		{
//			m_ControlBlock->m_ReferenceCount++; 
//			std::cout << "ptr_owned move constructor called" << std::endl;
//		}
//
//		// Delete pointer data in destructor, also invalidate the control block and decrement the reference counter of said control block
//		~ptr_owned() { ptr_control_block::release(m_ControlBlock); std::cout << "ptr_owned destructor called" << std::endl; }
//
//		// Default move assignment operator
//		ptr_owned<T>& operator=(ptr_owned<T>&&) = default;
//
//		// Delete copy assignment operator
//		ptr_owned<T>& operator=(ptr_owned<T>&) = delete;
//
//	public:
//		// Operator used to acces the raw pointer data
//		T* operator->() const { return m_Data; }
//
//	public:
//		/// <summary>
//		/// Grabs a reference to this pointer.
//		/// A reference is not in control of the data, and can therefore never delete the data or invalidate it
//		/// </summary>
//		/// <returns></returns>
//		inline ptr_reference<T> grabReference() {
//			return ptr_reference<T>(m_Data, m_ControlBlock);
//		}
//
//		/// <summary>
//		/// Invalidates the pointer object/ptr_control_block and deletes the data.
//		/// The control block will not be deleted when calling this.
//		/// All references derived from this pointer still hold the control block data.
//		/// However the references should always check if the control block is still valid before using the data.
//		/// This method always needs to be called, otherwise the data that this is pointing towards will never be deleted!
//		/// </summary>
//		void invalidate() {
//			m_ControlBlock->invalidate();
//			ptr_control_block::release(m_ControlBlock);
//			delete m_Data;
//		}
//
//	private:
//		T* m_Data;
//		ptr_control_block* m_ControlBlock;
//
//		friend class JupiterMemory;
//	};
//
//	/// <summary>
//	/// A pointer object that shares the ownership of the data with other objects
//	/// </summary>
//	/// <typeparam name="T">The type of the data</typeparam>
//	template<typename T>
//	class ptr_shared {
//
//	private:
//		// Argument constructor, only acceptable constructor to create a new control block
//		ptr_shared(T* data) : m_SharedData(data) {
//			m_ControlBlock = ptr_control_block::create();
//			m_ControlBlock->m_ReferenceCount++;
//			m_ControlBlock->m_Valid = true;
//		}
//
//	public:
//		// Default contructor
//		ptr_shared() = default;
//
//		// Destructor
//		~ptr_shared() {
//			ptr_control_block::release(m_ControlBlock);
//		}
//
//	private:
//		T* m_SharedData;
//		ptr_control_block* m_ControlBlock;
//
//		friend class JupiterMemory;
//	};
//
//	class JupiterMemory {
//
//	public:
//		template<typename T, typename ...Args>
//		static ptr_owned<T> createOwned(Args&&... args) {
//			T* data = new T(std::forward<Args>(args)...);
//			return ptr_owned<T>(data);
//		}
//
//		template<typename T, typename ...Args>
//		static ptr_shared<T> createShared(Args&&... args) {
//			T* data = new T(std::forward<Args>(args)...);
//			return ptr_shared<T>(data);
//		}
//	};
//
}
