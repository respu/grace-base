#pragma once
#ifndef COMPOSITE_TYPE_HPP_K5R3HGBW
#define COMPOSITE_TYPE_HPP_K5R3HGBW

#include "type/structured_type.hpp"
#include "serialization/archive.hpp"
#include "base/array.hpp"
#include "object/object_type.hpp"
#include <new>

namespace falling {
	
struct ExposedAttribute : public IAttribute {
	// IAttribute interface
	const Type* type() const final { return attribute_->type(); }
	StringRef name() const final { return attribute_->name(); }
	StringRef description() const final { return attribute_->description(); }
	Any get_any(const Object* object) const final;
	Any get_any(Object* object) const final;
	bool set_any(Object* object, const Any& value) const final;
	void deserialize_attribute(Object* object, const ArchiveNode&, UniverseBase&) const final;
	void serialize_attribute(const Object* object, ArchiveNode&, UniverseBase&) const final;
	
	// ExposedAttribute interface
	size_t aspect() const { return aspect_idx_; }
	const IAttribute* attribute() const { return attribute_; }
private:
	size_t aspect_idx_;
	const IAttribute* attribute_;
	friend class CompositeType;
	ExposedAttribute(size_t aspect_idx, const IAttribute* attribute) : aspect_idx_(aspect_idx), attribute_(attribute) {}
};

void resolve_exposed_attribute(const IAttribute*& inout_attr, ObjectPtr<>& inout_object);

struct CompositeType : StructuredType {
	CompositeType(IAllocator& alloc, StringRef name, const ObjectTypeBase* base_type = nullptr);
	~CompositeType();
	
	// CompositeType interface
	IAllocator& allocator() const;
	const ObjectTypeBase* base_type() const;
	size_t base_size() const;
	void add_aspect(const StructuredType* aspect);
	void freeze() { frozen_ = true; }
	size_t num_aspects() const { return aspects_.size(); }
	Object* get_aspect_in_object(Object* object, size_t idx) const;
	const Object* get_aspect_in_object(const Object* object, size_t idx) const;
	Object* find_aspect_of_type(Object* composite_object, const DerivedType* aspect, const DerivedType* skip_in_search = nullptr) const;
	void expose_attribute(size_t aspect_idx, StringRef attr_name);
	void unexpose_attribute(size_t aspect_idx, StringRef attr_name);
	void expose_slot(size_t aspect_idx, StringRef slot_name);
	void unexpose_slot(size_t aspect_idx, StringRef slot_name);
	ArrayRef<const ExposedAttribute*> exposed_attributes() const;
	const StructuredType* get_aspect(size_t aspect_idx) const;
	
	// Type interface
	void construct(byte* place, UniverseBase&) const override;
	void destruct(byte* place, UniverseBase&) const override;
	void copy_construct(byte* place, const byte* original) const override { ASSERT(false); }
	void move_construct(byte* place, byte* original) const override { ASSERT(false); }
	void deserialize_raw(byte* place, const ArchiveNode& node, UniverseBase&) const override;
	void serialize_raw(const byte* place, ArchiveNode& node, UniverseBase&) const override;
	String name() const override { return name_; }
	size_t size() const override { return size_; }
	
	// DerivedType interface
	size_t num_elements() const { return aspects_.size(); }
	size_t offset_of_element(size_t idx) const;
	const Type* type_of_element(size_t idx) const { return aspects_[idx]; }
	
	// StructuredType interface
	ArrayRef<const IAttribute*> attributes() const override;
	ArrayRef<const SlotBase* const> slots() const override;
private:
	const ObjectTypeBase* base_type_;
	String name_;
	Array<const StructuredType*> aspects_; // TODO: Consider ownership
	bool frozen_;
	size_t size_;
	Array<ExposedAttribute*> exposed_attributes_;
	Array<SlotBase*> exposed_slots_; // TODO!
};

inline size_t CompositeType::offset_of_element(size_t idx) const {
	size_t offset = base_size();
	for (size_t i = 0; i < aspects_.size(); ++i) {
		if (i == idx) return offset;
		offset += aspects_[i]->size();
	}
	ASSERT(false); // unreachable
	return SIZE_T_MAX;
}
	
inline Object* CompositeType::get_aspect_in_object(Object *object, size_t idx) const {
	ASSERT(object->object_type() == this);
	byte* memory = reinterpret_cast<byte*>(object);
	size_t offset = offset_of_element(idx);
	return reinterpret_cast<Object*>(memory + offset);
}
	
	inline const Object* CompositeType::get_aspect_in_object(const Object *object, size_t idx) const {
		ASSERT(object->object_type() == this);
		const byte* memory = reinterpret_cast<const byte*>(object);
		size_t offset = offset_of_element(idx);
		return reinterpret_cast<const Object*>(memory + offset);
	}
	
	template <typename To, typename From>
	typename std::enable_if<HasReflection<To>::Value && HasReflection<From>::Value, To*>::type
	aspect_composite_cast(From* from) {
		if (from == nullptr) return nullptr;
		// At this point, we should've already checked that we can't directly up- or downcast,
		// so let's see if we're a composite.
		
		const DerivedType* from_type = from->object_type();
		const ObjectTypeBase* to_type = get_type<To>();
		
		const CompositeType* from_composite_type = dynamic_cast<const CompositeType*>(from_type);
		if (from_composite_type) {
			// from is a composite, so see if it has an aspect matching To.
			Object* found = from_composite_type->find_aspect_of_type(from, to_type);
			if (found != nullptr) {
				return dynamic_cast<To*>(found);
			}
		}
		
		Object* composite_parent = from->find_parent();
		const DerivedType* skip_aspect_search = from_type;
		while (composite_parent != nullptr) {
			// from is an aspect in a composite object.
			const CompositeType* composite_parent_type = dynamic_cast<const CompositeType*>(composite_parent->object_type());
			ASSERT(composite_parent_type != nullptr); // find_parent returned non-NULL, but parent is not a composite!
			Object* found = nullptr;
			
			// Check if the base type of the composite object is what we're looking for:
			To* f = dynamic_cast<To*>(composite_parent);
			if (f != nullptr) {
				return f;
			}
			
			// It wasn't, check if any of the siblings were what we're looking for:
			found = composite_parent_type->find_aspect_of_type(composite_parent, to_type, skip_aspect_search);
			if (found != nullptr) {
				return dynamic_cast<To*>(found);
			} else {
				// None of the siblings matched, so check siblings of parents:
				skip_aspect_search = composite_parent_type;
				composite_parent = composite_parent->find_parent();
			}
		}
		
		return nullptr;
	}
	
	// composite sidecast, potential upcast
	template <typename To, typename From>
	typename std::enable_if<
	!std::is_same<To, From>::value
	&& HasReflection<To>::Value && HasReflection<From>::Value
	&& !std::is_convertible<From*, To*>::value
	&& std::is_convertible<To*, From*>::value
	, To*>::type
	aspect_cast(From* ptr) {
		To* dyn_to = dynamic_cast<To*>(ptr);
		if (dyn_to != nullptr) return dyn_to;
		return aspect_composite_cast<To>(ptr);
	}
	
	// composite sidecast, no upcast
	template <typename To, typename From>
	typename std::enable_if<
	!std::is_same<To, From>::value
	&& HasReflection<To>::Value && HasReflection<From>::Value
	&& !std::is_convertible<From*, To*>::value
	&& !std::is_convertible<To*, From*>::value
	, To*>::type
	aspect_cast(From* ptr) {
		return aspect_composite_cast<To>(ptr);
	}

}

#endif /* end of include guard: COMPOSITE_TYPE_HPP_K5R3HGBW */
