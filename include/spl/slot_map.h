/*******************************************************************************
* MIT License
*
* Copyright (c) 2021 Spirrwell
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
********************************************************************************/

#ifndef SPL_SLOTMAP
#define SPL_SLOTMAP

#include <vector>
#include <cstdint>
#include <utility>
#include <limits>
#include <stdexcept>
#include <iterator>

namespace spl {

template <typename T>
struct slot_map;

struct slot_handle
{
	template <typename T>
	friend struct slot_map;

private:
	std::size_t id;
	uint64_t generations;
};

template <typename T>
struct slot_wrap
{
	friend struct slot_map<T>;

	inline T* operator->();
	inline const T* operator->() const;

	inline T& operator*();
	inline const T& operator*() const;

	const slot_handle& get_key() const { return key; }

private:
	slot_handle key;
	slot_map<T>* owner;
};

template <typename T>
struct slot_map
{
	using iterator = typename std::vector<T>::iterator;
	using const_iterator = typename std::vector<T>::const_iterator;

	using reverse_iterator = typename std::vector<T>::reverse_iterator;
	using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;

	static constexpr std::size_t npos = (std::numeric_limits<std::size_t>::max)();

	constexpr slot_map() = default;
	constexpr slot_map(std::size_t initial_capacity)
	{
		elements.reserve(initial_capacity);
		indices.reserve(initial_capacity);
		eraser.reserve(initial_capacity);
	}

	constexpr slot_wrap<T> as_wrap(const slot_handle& key)
	{
		slot_wrap<T> wrap;
		wrap.key = key;
		wrap.owner = this;

		return wrap;
	}

	template <typename...Args>
	constexpr slot_handle emplace_back(Args&&... args)
	{
		slot_handle handle;

		if (free_tail == npos) {
			handle.id = indices.size();
			handle.generations = 1;
			eraser.emplace_back(indices.size());

			index& in = indices.emplace_back();
			in.index = elements.size();
			in.generations = 1;

			elements.emplace_back(std::forward<Args>(args)...);

			return handle;
		}

		eraser.emplace_back(free_tail);
		index& in = indices[free_tail];

		handle.id = free_tail;
		handle.generations = in.generations;

		elements.emplace_back(std::forward<Args>(args)...);

		free_tail = in.index;
		in.index = elements.size() - 1;

		return handle;
	}

	constexpr void clear()
	{
		elements.clear();
		indices.clear();
		eraser.clear();
	}

	constexpr void erase(iterator pos)
	{
		std::size_t erase_index = std::distance(elements.begin(), pos);
		std::size_t next_free_slot = eraser[erase_index];

		*pos = std::move(elements.back());
		elements.pop_back();

		eraser[erase_index] = eraser.back();
		eraser.pop_back();

		index& in = indices[next_free_slot];
		in.index = free_tail;
		in.generations++;

		free_tail = next_free_slot;
	}

	constexpr void erase(const slot_handle& key)
	{
		if (key.id >= indices.size())
			return;

		index& erase_index = indices[key.id];

		if (erase_index.generations != key.generations) {
			return;
		}

		std::size_t next_free_slot = key.id;

		// Swap & pop
		elements[erase_index.index] = std::move(elements.back());
		elements.pop_back();

		eraser[erase_index.index] = eraser.back();
		eraser.pop_back();

		erase_index.index = free_tail;
		erase_index.generations++;

		free_tail = next_free_slot;
	}

	constexpr const T& operator[](const slot_handle& key) const
	{
		index& index = indices[key.id];

		if (index.generations != key.generations) {
			throw std::out_of_range("invalid handle");
		}

		return elements[index.index];
	}

	constexpr T& operator[](const slot_handle& key)
	{
		index& index = indices[key.id];
		
		if (index.generations != key.generations) {
			throw std::out_of_range("invalid handle");
		}

		return elements[index.index];
	}

	constexpr T* data() { return elements.data(); }
	constexpr const T* data() const { return elements.data(); }

	constexpr iterator begin() { return elements.begin(); }
	constexpr iterator end() { return elements.end(); }

	constexpr iterator begin() const { return elements.begin(); }
	constexpr iterator end() const { return elements.end(); }

	constexpr const_iterator cbegin() const { return elements.cbegin(); }
	constexpr const_iterator cend() const { return elements.cend(); }

	constexpr reverse_iterator rbegin() { return elements.rbegin(); }
	constexpr reverse_iterator rend() { return elements.rend(); }

	constexpr reverse_iterator rbegin() const { return elements.rbegin(); }
	constexpr reverse_iterator rend() const { return elements.rend(); }

	constexpr const_reverse_iterator crbegin() const { return elements.crbegin(); }
	constexpr const_reverse_iterator crend() const { return elements.crend(); }

private:
	struct index
	{
		std::size_t index;
		uint64_t generations;
	};

	std::vector<T> elements;
	std::vector<index> indices;
	std::vector<std::size_t> eraser;

	std::size_t free_tail = npos;
};

template <typename T>
inline T* slot_wrap<T>::operator->()
{
	return &(*owner)[key];
}

template <typename T>
inline const T* slot_wrap<T>::operator->() const
{
	return &(*owner)[key];
}

template <typename T>
inline T& slot_wrap<T>::operator*()
{
	return (*owner)[key];
}

template <typename T>
inline const T& slot_wrap<T>::operator*() const
{
	return (*owner)[key];
}

}

#endif // SPL_SLOTMAP