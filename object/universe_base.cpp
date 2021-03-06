//
//  universe_base.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 08/01/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#include "object/universe_base.hpp"
#include "io/resource_manager.hpp"
#include "serialization/deserialize_object.hpp"
#include "serialization/serialize.hpp"
#include "base/parse.hpp"
#include "io/formatters.hpp"
#include "object/composite_type.hpp"

namespace grace {
	void DeferredAttributeDeserialization::perform(IUniverse& universe) const {
		attribute->deserialize_attribute(object.get(), *node, universe);
	}
	
	UniverseBase::~UniverseBase() {
		for (auto p: composite_types_) {
			destroy(p, allocator());
		}
	}
	
	CompositeType* UniverseBase::create_composite_type(const ObjectTypeBase* base, StringRef name) {
		if (name == "") {
			name = base->name();
		}
		CompositeType* p = new(allocator()) CompositeType(allocator(), name, base);
		composite_types_.push_back(p);
		return p;
	}

	bool UniverseBase::instantiate(const DocumentNode &scene_definition, String &out_error) {
		if (!scene_definition.is_map()) {
			out_error = "Invalid scene definition.";
			return false;
		}
		
		int format_version;
		if (scene_definition["format"] >> format_version) {
			// TODO: Check scene version
		}
		
		scene_definition["objects"].array_each([&](const DocumentNode& object_definition) {
			deserialize_object(object_definition, *this);
		});
		
		for (auto& deferred: deferred_) {
			deferred.perform(*this);
		}
		deferred_.clear();
		
		return true;
	}
	
	bool BasicUniverse::serialize_scene(DocumentNode &root_node, grace::String &out_error) {
		root_node["format"] << 1;
		auto& objects = root_node["objects"];
		for (auto ptr: object_map_.values()) {
			grace::serialize(*ptr, objects.array_push(), *this);
		}
		return true; // XXX: Report errors
	}
	
	void UniverseBase::defer_attribute_deserialization(ObjectPtr<> obj, const IAttribute *attr, const DocumentNode *serialized) {
		deferred_.push_back(DeferredAttributeDeserialization{obj, attr, serialized});
	}
	
	bool BasicUniverse::recreate_object_and_initialize(const DocumentNode &node, StringRef object_id) {
		ASSERT(false); // Cannot recreate object in non-editor universe
	}
	void error_category_already_initialized_with_different_type(StringRef name) {
		Error() << "Object category has already been initialized with a different type: " << name;
	}
	
	ObjectPtr<> BasicUniverse::create_object_and_set_as_root(const StructuredType* type, StringRef id) {
		clear();
		root_ = create_object(type, std::move(id));
		return root_;
	}
	
	ObjectPtr<> BasicUniverse::create_object(const StructuredType* type, StringRef id) {
		size_t sz = type->size();
		byte* memory = (byte*)allocator().allocate(sz, type->alignment());
		type->construct(memory, *this);
		Object* object = reinterpret_cast<Object*>(memory);
		memory_map_.push_back(object);
		rename_object(ObjectPtr<>(object), id);
		if (type->wants_game_update()) {
			register_object_for_update(ObjectPtr<>(object));
		}
		return ObjectPtr<>(object);
	}
	
	bool BasicUniverse::rename_object(ObjectPtr<> object, StringRef new_id) {
		ASSERT(object->universe() == this);
		
		// erase old name from database
		auto old_it = reverse_object_map_.find(object);
		if (old_it != reverse_object_map_.end()) {
			object_map_.erase(old_it->second);
		}
		
		// check if new name already exists
		auto it = object_map_.find(String(new_id));
		bool renamed_exact = true;
		String new_name;
		if ((it != object_map_.end()) || (new_id.size() < 2)) {
			// it does, so create a unique name from the requested name
			int n = 1;
			String base_name;
			if (new_id.size() >= 2) {
				Maybe<int> parsed = parse<int>(substr(new_id, -2));
				maybe_if(parsed, [&](int parsed_number) {
					base_name = substr(new_id, 0, -2);
				}).otherwise([&]() {
					base_name = std::move(new_id);
				});
				n += 1; // n is set to 0 if recognition failed, otherwise the existing number. Add one. :)
			} else {
				base_name = object->object_type()->name();
			}
			
			// increment n and try the name until we find one that's available
			do {
				StringStream create_new_name;
				create_new_name << base_name << format("%02d", n);
				new_name = std::move(create_new_name.str());
				++n;
			} while (object_map_.find(new_name) != object_map_.end());
			
			renamed_exact = false;
		} else {
			new_name = std::move(new_id);
		}
		
		object_map_[new_name] = object;
		reverse_object_map_[object] = std::move(new_name);
		return renamed_exact;
	}
	
	StringRef BasicUniverse::get_id(ObjectPtr<const Object> object) const {
		auto it = reverse_object_map_.find(object);
		if (it != reverse_object_map_.end()) {
			return it->second;
		}
		return "";
	}
	
	void BasicUniverse::run_initializers() {
		for (auto pair: object_map_) {
			pair.second->initialize();
		}
	}
	
	void BasicUniverse::clear() {
		UniverseBase::clear();
		for (auto object: memory_map_) {
			const StructuredType* type = object->object_type();
			type->destruct(reinterpret_cast<byte*>(object), *this);
			allocator().free(object, type->size());
		}
		// TODO: Test for references?
		object_map_.clear();
		reverse_object_map_.clear();
		memory_map_.clear();
		
	}
	
	void UniverseBase::clear() {
		update_objects_.clear();
	}
	
	void UniverseBase::update(GameTimeDelta delta) {
		for (auto p: update_objects_) {
			p->update(delta);
		}
	}
	
	void UniverseBase::register_object_for_update(ObjectPtr<> ptr) {
		update_objects_.insert(ptr);
	}
	
	void UniverseBase::unregister_object_for_update(ObjectPtr<> ptr) {
		auto it = update_objects_.find(ptr);
		if (it != update_objects_.end()) {
			update_objects_.erase(it);
		}
	}
}
