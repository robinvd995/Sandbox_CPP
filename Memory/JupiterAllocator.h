#pragma once

#include <exception>
#include <iostream>

namespace Jupiter {

	typedef unsigned long long		uint64;		// Unsigned 64 bit integer
	typedef unsigned int			uint32;		// Unsigned 32 bit integer
	typedef unsigned short			uint16;		// Unsigned 16 bit integer
	typedef unsigned char			uint8;		// unsigned 8 bit integer

	typedef long long				int64;		// 64 bit integer
	typedef int						int32;		// 32 bit integer
	typedef short					int16;		// 16 bit integer
	typedef char					int8;		// 8 bit integer

	/// <summary>
	/// Namespace containing functions to do with pointers
	/// </summary>
	namespace pointer_functions {

		/// <summary>
		/// Calculates the adjustment needed to forward align a given memory address
		/// </summary>
		/// <param name="ptr">The memory address</param>
		/// <param name="alignment">The alignment value</param>
		/// <returns>The adjustment the given memory address need to be moved forward to be aligned</returns>
		inline uint8 calc_forward_alignment_adjustment(void* ptr, uint8 alignment);

		/// <summary>
		/// Calculates a new memory address by shifting another memory address forward by x amount of bytes
		/// </summary>
		/// <param name="ptr">The memory address needed to be shifted</param>
		/// <param name="x">the amount of bytes to be shifted</param>
		/// <returns>the shifted memory address</returns>
		inline void* shift_forward(void* ptr, size_t x);

		/// <summary>
		/// Calculates a new memory address by shifting another memory address forward by x amount of bytes
		/// </summary>
		/// <param name="ptr">The memory address needed to be shifted</param>
		/// <param name="x">the amount of bytes to be shifted</param>
		/// <returns>the shifted memory address as a const</returns>
		inline const void* shift_forward(const void* ptr, size_t x);

		/// <summary>
		/// Calculates a new memory address by shifting another memory address backward by x amount of bytes
		/// </summary>
		/// <param name="ptr">The memory address needed to be shifted</param>
		/// <param name="x">the amount of bytes to be shifted</param>
		/// <returns>the shifted memory address</returns>
		inline void* shift_back(void* ptr, size_t x);

		/// <summary>
		/// Calculates a new memory address by shifting another memory address backward by x amount of bytes
		/// </summary>
		/// <param name="ptr">The memory address needed to be shifted</param>
		/// <param name="x">the amount of bytes to be shifted</param>
		/// <returns>the shifted memory address as a const</returns>
		inline const void* shift_back(const void* ptr, size_t x);
	}

	/// <summary>
	/// Simple interface containing a allocate and deallocate method
	/// Every allocator need to implent these functions
	/// 
	/// ??? Maybe remove the deallocate function since some allocators are not compatible with that deallocate method eg. StackAllocator, LinearAllocator???
	/// </summary>
	class IAllocator {

	public:
		virtual ~IAllocator() = default;

		virtual void* allocate(size_t size, uint8 allignment = 4) = 0;
		virtual void deallocate(void* p) = 0;
		virtual void deallocate(void* p, size_t size) = 0;
	};

	/// <summary>
	/// Class that allocates memory in a stack like fashion, the total memory block is allocated on instantiation.
	/// Therefore all the allocation calls are static
	/// </summary>
	class StackAllocator : public IAllocator {

		typedef void* stack_marker;			// Typedef to differntiate between a normal void* and a void* used as a stack marker

	public:
		StackAllocator() = default;

		/// <summary>
		/// Creates a stack allocator with a static memory profile
		/// </summary>
		/// <param name="size">The size of the stack in bytes</param>
		StackAllocator(size_t size);

		virtual ~StackAllocator() override;										// Override virtual desctructor
		virtual void* allocate(size_t size, uint8 allignment = 4) override;		// Override allocate function
		virtual void deallocate(void* p) override;								// Throws exception, cannot deallocate on a stack!
		virtual void deallocate(void* p, size_t size) override;					// Throws exception, cannot deallocate on a stack!

		/// <summary>
		/// Marks the current point in the stack.
		/// Allocates a stack marker in the stack block, this effectivly snapshots some of the variables the allocator keeps track off.
		/// 
		/// ???
		/// Another way of doing it would be to keep an additional block of memory and allocate markers there.
		/// Could also use vectors if dynamic allocation is fine, or just reserve
		/// Allocating a "stack marker" in the stack seems to be the most clean, since it doesnt require any additional allocations and barely 
		/// takes up memory. If memory continuation is an issue this needs to be changed
		/// </summary>
		/// <returns></returns>
		stack_marker mark();

		/// <summary>
		/// Free's the stack from a marker onward
		/// Effectivly sets the top of the stack to the top when the marker was made
		/// All markers created after the marker freed are invalid and should not be used
		/// </summary>
		/// <param name="marker"></param>
		void freeForward(stack_marker marker);

		/// <summary>
		/// Clears the stack deallocating all memory
		/// This effectivly just sets the top of the stack equal to the start and sets the used memory and allocations to zero
		/// </summary>
		void clear();

	private:
		void* m_Start;					// Pointer pointing the start of the allocator memory block
		size_t m_Size;					// The size of the allocated memory block

		void* m_Top;					// Pointer pointing to the top of the stack
		size_t m_UsedMemory;			// The total memory used by this allocator
		size_t m_Allocations;			// The total number of allocations this allocator has made
	};

	/// <summary>
	/// A memory allocator that works similar to a heap
	/// </summary>
	class BlockAllocator : public IAllocator {};

	/// <summary>
	/// 
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T>
	class PoolAllocator : public IAllocator {};

//	class AllocatorBase {
//
//	public:
//		AllocatorBase(size_t size, void* start, EnumAllocationType allocationMethod) : m_Size(size), m_Start(start), m_AllocationMethod(allocationMethod) {
//			m_UsedMemory = 0;
//			m_NumAllocations = 0;
//		}
//
//		virtual ~AllocatorBase() {
//			// JPT_ASSERT((m_UsedMemory == 0 && m_NumAllocations == 0), "Allocator is beeing destroyed without releasing all memory!");
//			m_Start = nullptr;
//			m_Size = 0;
//		}
//
//		virtual void* allocate(size_t size, uint8 alignment = 4) = 0;
//		virtual void deallocate(void* p) = 0;
//
//		inline size_t getSize() const { return m_Size; }
//		inline size_t getUsedMemory() const { return m_UsedMemory; }
//		inline size_t getNumAllocations() const { return m_NumAllocations; }
//
//	protected:
//		const EnumAllocationType m_AllocationMethod;
//		void* m_Start;
//		size_t m_Size;
//		size_t m_UsedMemory;
//		size_t m_NumAllocations;
//	};

//	namespace Allocator {
//
//		template<typename T>
//		T* allocateNew(AllocatorBase& allocator) {
//			return new (allocator.allocate(sizeof(T), __alignof(T))) T;
//		}
//
//		template<typename T>
//		T* allocateNew(AllocatorBase& allocator, const T& t) {
//			return new (allocator.allocate(sizeof(T), __alignof(T))) T(t);
//		}
//
//		template<typename T>
//		void deallocateDelete(AllocatorBase& allocator, T& object) {
//			object.~T();
//			allocator.deallocate(&object);
//		}
//	}
}

#include "JupiterAllocator.inl"
