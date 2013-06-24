#include "serialization/archive.hpp"
#include "object/objectptr.hpp"
#include "serialization/serialize.hpp"
#include "type/type_registry.hpp"
#include "object/composite_type.hpp"
#include "object/universe.hpp"
#include "serialization/deserialize_object.hpp"

namespace grace {
	ArchiveNode& Archive::operator[](StringRef key) {
		return root()[key];
	}
	
	const ArchiveNode& Archive::operator[](StringRef key) const {
		return root()[key];
	}
	
	ArchiveNode* Archive::make() {
		return nodes_.allocate(*this);
	}
	
	void Archive::clear() {
		nodes_.clear();
		root_.clear();
	}
}