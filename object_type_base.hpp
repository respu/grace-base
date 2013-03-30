//
//  object_type_base.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 30/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_object_type_base_hpp
#define falling_object_type_base_hpp

#include "type/structured_type.hpp"
#include "base/string.hpp"
#include "object/slot.hpp"

namespace falling {
	class IAllocator;
	
	struct ObjectTypeBase : StructuredType {
		StringRef name() const override { return name_; }
		StringRef description() const { return description_; }
		const StructuredType* super() const final;
		
		template <typename T, typename R, typename... Args>
		const SlotForTypeWithSignature<T,R,Args...>* find_slot_for_method(typename GetMemberFunctionPointerType<T, R, Args...>::Type method) const {
			for (auto s: slots()) {
				auto slot = dynamic_cast<const SlotForTypeWithSignature<T,R,Args...>*>(s);
				if (slot != nullptr && slot->method() == method) {
					return slot;
				}
			}
			return nullptr;
		}
		
		ObjectTypeBase(IAllocator& alloc, const ObjectTypeBase* super, StringRef name, StringRef description) : super_(super), name_(name, alloc), description_(description, alloc), is_abstract_(false) {}
		
		const ObjectTypeBase* super_;
		String name_;
		String description_;
		bool is_abstract_;
		
		void set_abstract(bool b) { this->is_abstract_ = b; }
		bool is_abstract() const { return this->is_abstract_; }
	};
}

#endif
