#ifndef FIXED_LIST_H
#define FIXED_LIST_H

#include <iostream>
#include <cassert>
#include <stdexcept>

namespace ChessEngine
{
	// Custom deque class used for storing positions
	template<typename T, size_t size>
	class FixedList
	{
	private:
		size_t current_size = 0;
		std::array<T, size> fixed_list;
	public:
		FixedList() = default;

		inline size_t size() { return current_size; }
		inline bool is_empty() { return current_size == 0; }

		inline T begin() { return fixed_list.begin(); }
		inline T last() { return fixed_list.last(); }

		inline T& operator[] (size_t index) { return fixed_list[index]; }
		inline const T& operator[] (size_t index) const { return fixed_list[index]; }

		inline void clear() { current_size = 0; } // the memory will be released after the array goes out of scope

		inline void push(T element) { fixed_list[current_size++] = element; }
		inline void pop_at(size_t index)
		{
			if (index == --current_size) return;

			// Starting from the next element in the array 
			// we just have to left shift it by one
			// since we will pop at 'index'
			for (size_t i = index + 1; i <= current_size; i++)
				fixed_list[i - 1] = fixed_list[i];
		}

		inline T pop() { return fixed_list[--current_size]; }
	};
}

#endif // !DEQUE_H
