//
//  arch.h
//  falling
//
//  Created by Simon Ask Ulsnes on 07/03/13.
//  Copyright (c) 2013 Simon Ask Consulting. All rights reserved.
//

#ifndef __falling__arch__
#define __falling__arch__

#include "base/basic.hpp"

namespace falling {
	class String;

	void get_backtrace(void** out_instruction_pointers, size_t num_steps, size_t offset = 0);
	void resolve_symbol(void* ip, String& out_module_name, String& out_demangled_function_name, uint32& out_offset);
}

#endif /* defined(__falling__arch__) */