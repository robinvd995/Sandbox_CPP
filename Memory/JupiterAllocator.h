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

	namespace pointer_functions {

		inline uint8 calc_forward_alignment_adjustment(void* ptr, uint8 alignment) {
			uint8 alignmentMask = alignment - 1;
			uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
			uint8 adjustment = alignment - (address & alignmentMask);
//			if (adjustment == alignment) return 0;
			return adjustment;
		}

		inline void* add(void* ptr, size_t x) {
			return (void*)(reinterpret_cast<uintptr_t>(ptr) + x);
		}

		inline const void* add(const void* ptr, size_t x) {
			return (const void*)(reinterpret_cast<uintptr_t>(ptr) + x);
		}

		inline void* subtract(void* ptr, size_t x) {
			return (void*)(reinterpret_cast<uintptr_t>(ptr) - x);
		}

		inline const void* subtract(const void* ptr, size_t x) {
			return (const void*)(reinterpret_cast<uintptr_t>(ptr) - x);
		}
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

	class StackAllocator : public IAllocator {

		typedef size_t stack_marker;

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
		/// Marks the current point in the stack
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
		/// This effectivly just sets the top of the stack equal to the start
		/// </summary>
		void clear();

	private:
		void* m_Start;					// Pointer pointing the start of the allocator memory block
		size_t m_Size;					// The size of the allocated memory block

		void* m_Top;					// Pointer pointing to the top of the stack
		size_t m_UsedMemory;			// The total memory used by this allocator
		size_t m_Allocations;			// The total number of allocations this allocator has made
	};

	class LinearAllocator {};

	class PoolAllocator {};

	class DoubleStackAllocator {};

	class ListAllocator {};

	class DynamicListAllocator {};

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
