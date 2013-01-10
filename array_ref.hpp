//
//  array_ref.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 29/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_array_ref_hpp
#define falling_array_ref_hpp

#include <initializer_list>
#include "base/iterators.hpp"

namespace falling {
	struct Empty {};

	template <typename T>
	struct ArrayRef {
	public:
		constexpr ArrayRef() : begin_(nullptr), end_(nullptr) {}
		constexpr ArrayRef(Empty e) : begin_(nullptr), end_(nullptr) {}
		template <size_t N, typename C = T>
		constexpr ArrayRef(const C(&data)[N]) : begin_(data), end_(data + N) { }
		constexpr ArrayRef(T* begin, T* end) : begin_(begin), end_(end) { /*ASSERT(begin_ <= end_);*/ }
		ArrayRef(const ArrayRef<T>& other) = default;
		ArrayRef(ArrayRef<T>&& other) = default;
		ArrayRef<T>& operator=(const ArrayRef<T>& other) = default;
		ArrayRef<T>& operator=(ArrayRef<T>&& other) = default;
		bool operator==(const ArrayRef<T>& other) const;
		bool operator!=(const ArrayRef<T>& other) const;
		
		size_t size() const { return end_ - begin_; }
		
		const T& operator[](size_t idx) const { ASSERT(idx < size()); return begin_[idx]; }
		T& operator[](size_t idx) { ASSERT(idx < size()); return begin_[idx]; }
		
		typedef const T value_type;
		using iterator = LinearMemoryIterator<T, false>;
		using const_iterator = LinearMemoryIterator<T, true>;
		iterator begin() { return begin_; }
		iterator end() { return end_; }
		const_iterator begin() const { return begin_; }
		const_iterator end() const { return end_; }
		
		const T* data() const { return begin_; }
	private:
		T* begin_;
		T* end_;
	};
	
	template <typename T>
	bool ArrayRef<T>::operator==(const ArrayRef<T> &other) const {
		if (size() == other.size()) {
			for (size_t i = 0; i < size(); ++i) {
				if (begin_[i] != other.begin_[i]) {
					return false;
				}
			}
			return true;
		}
		return false;
	}
	
	template <typename T>
	bool ArrayRef<T>::operator!=(const ArrayRef<T>& other) const {
		return !(*this == other);
	}
}

#endif
