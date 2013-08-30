#include "type/maybe_type.hpp"

namespace grace {
	
String build_maybe_type_name(const IType* inner_type) {
	StringStream ss;
	ss << "Maybe<" << inner_type->name() << '>';
	return ss.str();
}

}