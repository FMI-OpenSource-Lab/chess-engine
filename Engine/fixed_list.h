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

		inline void clear() { current_size = 0; }
	};
}

#endif // !DEQUE_H
