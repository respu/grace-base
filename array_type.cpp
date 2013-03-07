#include "type/array_type.hpp"
#include "io/string_stream.hpp"
#include "serialization/archive_node.hpp"

namespace falling {
	
String build_variable_length_array_type_name(IAllocator& alloc, StringRef base_name, const Type* element_type) {
	StringStream ss;
	ss << element_type->name() << "[]";
	return ss.string(alloc);
}

void ArrayType::deserialize_array(IArrayWriter& w, const ArchiveNode& node, IUniverse& universe) const {
	auto t = element_type();
	byte data_storage[t->size()];
	byte* element_data = data_storage;
	t->construct(element_data, universe);
	node.value().when<ArchiveNode::ArrayType>([&](const ArchiveNode::ArrayType& nodes) {
		w.reserve(nodes.size());
		for (auto child: nodes) {
			t->deserialize_raw(element_data, *child, universe);
			w.push_back_move(element_data);
		}
	});
	t->destruct(element_data, universe);
}

void ArrayType::serialize_array(IArrayReader& r, ArchiveNode& node, IUniverse& universe) const {
	while (r.next()) {
		void* p = r.get_current();
		ArchiveNode& element = node.array_push();
		element_type()->serialize_raw((const byte*)p, element, universe);
	}
}

}