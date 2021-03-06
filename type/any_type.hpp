//
//  any_type.h
//  grace
//
//  Created by Simon Ask Ulsnes on 30/04/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef __grace__any_type__
#define __grace__any_type__

#include "base/any.hpp"
#include "type/type.hpp"

namespace grace {
	struct AnyType : Type {
		AnyType() : Type(GetTypeInfo<Any>::Value) {}
		void deserialize(Any& place, const DocumentNode& n, IUniverse& u) const;
		void serialize(const Any& place, DocumentNode&, IUniverse&) const;
		StringRef name() const final { return "Any"; }
		size_t size() const final { return sizeof(Any); }
		
		
		// XXX: This padding was added because Clang wrongly generated
		// a class definition that reported sizeof(AnyType) == 1. This caused
		// memory corruption.
		byte padding_because_clang_is_buggy[16];
		
		void deserialize_raw(byte* place, const DocumentNode& n, IUniverse& u) const final {
			deserialize(*reinterpret_cast<Any*>(place), n, u);
		}
		void serialize_raw(const byte* place, DocumentNode& n, IUniverse& u) const final {
			serialize(*reinterpret_cast<const Any*>(place), n, u);
		}
	};
	
	template <>
	struct BuildTypeInfo<Any> {
		static const AnyType* build();
	};
}

#endif /* defined(__grace__any_type__) */
