#include "object/object.hpp"
#include "object/reflect.hpp"
#include "object/universe.hpp"

namespace falling {

Object* Object::find_parent() {
	Object* object = this;
	if (object->offset_ != 0) {
		ssize_t offs = object->offset_;
		return reinterpret_cast<Object*>(reinterpret_cast<byte*>(object) - offs);
	}
	return nullptr;
}

const Object* Object::find_parent() const {
	const Object* object = this;
	if (object->offset_ != 0) {
		ssize_t offs = object->offset_;
		return reinterpret_cast<const Object*>(reinterpret_cast<const byte*>(object) - offs);
	}
	return nullptr;
}

Object* Object::find_topmost_object() {
	Object* object = this;
	Object* p;
	while ((p = object->find_parent()) != nullptr) {
		object = p;
	}
	return object;
}

const Object* Object::find_topmost_object() const {
	const Object* object = this;
	const Object* p;
	while ((p = object->find_parent()) != nullptr) {
		object = p;
	}
	return object;
}

bool Object::set_object_id(StringRef new_id) {
	return universe_->rename_object(this, new_id);
}

StringRef Object::object_id() const {
	return universe_->get_id(this);
}

StringRef Object::object_type_name() const {
	return object_type()->name();
}

BEGIN_TYPE_INFO(Object)
	property(&Object::object_id, &Object::set_object_id, "id", "The unique ID for this object.");
	property(&Object::object_type_name, nullptr, "class", "The class name for this object.");
	// property(&Object::id_, "ID", "The unique ID for this Object.");
END_TYPE_INFO()

}