#include "type/array_type.hpp"
#include <sstream>

namespace falling {
	
std::string build_variable_length_array_type_name(std::string base_name, const Type* element_type) {
	std::stringstream ss;
	ss << element_type->name() << "[]";
	return ss.str();
}

}