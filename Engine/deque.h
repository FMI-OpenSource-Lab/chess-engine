#ifndef DEQUE_H
#define DEQUE_H

#include <iostream>
#include <cassert>
#include <stdexcept>

namespace ChessEngine
{
	template<typename T, size_t size>
	class Deque
	{
	private:
		struct Node
		{
			// Data held by the node
			T data;
			// pointer to previous node
			Node* prev;
			// pointer to next node
			Node* next;
			// Index of the element
			size_t index;

			// Constructor to create a node with given value
			Node(T value, size_t i)
				: data(value), index(i), prev(nullptr), next(nullptr) {}
		};

		Node* head;
		Node* last;
		size_t current_size;
	public:
		// init empty deque
		Deque() : head(nullptr), last(nullptr), current_size(0) {}
		Deque(const Deque&) = delete;

		inline bool is_empty() { return current_size == 0; }
		inline size_t get_size() { return current_size; }

		void push_back(T value)
		{
			if (current_size > size)
				throw std::out_of_range("Exceeding the size");

			Node* new_node = new Node(value, current_size);

			// if deque is empty then head and tail are the same
			if (is_empty())
				head = last = new_node;
			else
			{
				// link the new node to the tail
				new_node->prev = last;
				// link the current tail to the new node
				last->next = new_node;
				// update tail to the new node
				last = new_node;
			}

			current_size++;
		}

		inline void pop_at(size_t index)
		{
			if (is_empty())
				throw std::out_of_range("No elements in deque");

			Node* temp = head;

			// loop to arrive at element to be poped
			for (; temp->index < index; temp = temp->next);

			// If previous node exists
			if (temp->prev)
				// Bypass the current node
				temp->prev->next = temp->next;
			else // if its the head node
				head = last->next;

			// If next node exists
			if (temp->next)
				// Bypass current node
				temp->next->prev = temp->prev;
			else
				// if its last node
				last = temp->prev;

			// Free memory
			delete temp;
			current_size--;
		}

		inline T begin()
		{
			assert(is_empty());
			return head->data;
		}

		inline T end()
		{
			assert(is_empty());
			return last->data;
		}

		inline T pop_last()
		{
			if (is_empty())
				throw std::out_of_range("No elements in deque");

			Node* temp = last;
			last = last->prev;

			if (last == nullptr)
				head = nullptr;
			else
				last->next = nullptr;

			T value = temp->data;

			delete temp; // free memory of old node
			current_size--;

			return value;
		}

		inline T pop(size_t index)
		{
			if (is_empty())
				throw std::out_of_range("No elements in deque");

			Node* temp = head;

			// loop to arrive at element to be poped
			for (; temp->index < index; temp = temp->next);
			T value = temp->data;

			// If previous node exists
			if (temp->prev)
				// Bypass the current node
				temp->prev->next = temp->next;
			else // if its the head node
				head = last->next;

			// If next node exists
			if (temp->next)
				// Bypass current node
				temp->next->prev = temp->prev;
			else
				// if its last node
				last = temp->prev;

			// Free memory
			delete temp;
			current_size--;

			return value;
		}

		inline T& operator[](size_t index) // access for read/write
		{
			if (index < 0 || index >= size)
				throw std::out_of_range("Index out of range");

			Node* current = head;

			// if the index searched is from the left half
			if (index < current_size / 2)
				for (current = head; current->index < index; current = current->next);
			else // if from the right half
				for (current = last; current->index > index; current = current->prev);

			return current->data;
		}

		// A function to display the nodes in the double ended queue
		// for debuging purposes
		friend std::ostream& operator<<(std::ostream& os, const Deque& deque)
		{
			Deque::Node* current = deque.head;

			os << "\nDeque elements: ";
			while (current != nullptr)
			{
				os << "\nCurrent node at index: " << current->index
					<< "\nValue: " << current->data;

				current = current->next;
			}

			return os;
		}
	};
}

#endif // !DEQUE_H
