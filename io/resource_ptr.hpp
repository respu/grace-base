//
//  resource_ptr.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 28/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_resource_ptr_hpp
#define grace_resource_ptr_hpp

#include "io/resource.hpp"

namespace grace {
	template <typename T>
	class ResourcePtr {
	public:
		ResourcePtr() : ptr_(nullptr) {}
		ResourcePtr(std::nullptr_t) : ptr_(nullptr) {}
		explicit ResourcePtr(T* ptr) : ptr_(ptr) { retain(); }
		explicit ResourcePtr(const T* ptr) : ptr_(const_cast<T*>(ptr)) { retain(); }
		ResourcePtr(const ResourcePtr<T>& other) : ptr_(other.ptr_) { retain(); }
		ResourcePtr(ResourcePtr<T>&& other) : ptr_(other.ptr_) { other.ptr_ = nullptr; }
		~ResourcePtr() { release(); }
		ResourcePtr<T>& operator=(const ResourcePtr<T>& other);
		ResourcePtr<T>& operator=(ResourcePtr<T>&& other);
		ResourcePtr<T>& operator=(std::nullptr_t);
		bool operator==(const ResourcePtr<T>& other) const;
		bool operator!=(const ResourcePtr<T>& other) const;
		bool operator==(std::nullptr_t null) const { return ptr_ == nullptr; }
		bool operator!=(std::nullptr_t null) const { return ptr_ != nullptr; }
		explicit operator bool() const { return ptr_ != nullptr; }
		
		const T* operator->() const { return get(); }
		const T& operator*() const { return *get(); }
		const T* get() const { return static_cast<const T*>(ptr_); }
	private:
		friend class ResourceManager;
		
		Resource* ptr_;
		
		void retain();
		void release();
	};
	
	template <typename T>
	inline ResourcePtr<T>& ResourcePtr<T>::operator=(const ResourcePtr<T>& other) {
		if (this == &other) return *this;
		release();
		ptr_ = other.ptr_;
		retain();
		return *this;
	}
	
	template <typename T>
	inline ResourcePtr<T>& ResourcePtr<T>::operator=(ResourcePtr<T>&& other) {
		if (this == &other) return *this;
		release();
		ptr_ = other.ptr_;
		other.ptr_ = nullptr;
		return *this;
	}
	
	template <typename T>
	inline ResourcePtr<T>& ResourcePtr<T>::operator=(std::nullptr_t) {
		release();
		return *this;
	}
	
	template <typename T>
	inline bool ResourcePtr<T>::operator==(const ResourcePtr<T>& other) const {
		return other.ptr_ == ptr_;
	}
	
	template <typename T>
	inline bool ResourcePtr<T>::operator!=(const ResourcePtr<T>& other) const {
		return other.ptr_ != ptr_;
	}
	
	template <typename T>
	inline void ResourcePtr<T>::retain() {
		if (ptr_ != nullptr)
			ptr_->retain();
	}
	
	template <typename T>
	inline void ResourcePtr<T>::release() {
		if (ptr_ != nullptr) ptr_->release();
		ptr_ = nullptr;
	}
}

#endif
