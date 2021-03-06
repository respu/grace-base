//
//  util.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 25/11/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_util_hpp
#define grace_util_hpp

#include "io/input_stream.hpp"
#include "io/output_stream.hpp"
#include "base/either.hpp"
#include "base/array.hpp"
#include "base/time.hpp"

namespace grace {
	template <typename ContainerType>
	inline size_t read_all(IInputStream& is, ContainerType& buffer) {
		if (is.has_length()) {
			buffer.reserve(is.length());
		}
		size_t n;
		do {
			byte b[1024];
			auto r = is.read(b, 1024);
			r.template when<size_t>([&](size_t n) {
				buffer.insert(b, b + n);
			}).template when<IOEvent>([&](IOEvent ev) {
				n = 0;
			});
		} while (n > 0);
		return buffer.size();
	}
	
	template <typename ContainerType>
	inline ContainerType read_all(IInputStream& is, IAllocator& alloc = default_allocator()) {
		ContainerType buffer(alloc);
		read_all(is, buffer);
		return move(buffer);
	}

	std::tuple<size_t, IOEvent> read_until_event(IInputStream& is, IOutputStream& output);

	String read_string(IInputStream& is, IAllocator& alloc = default_allocator());
	
	bool path_exists(StringRef path);
	bool path_is_file(StringRef path);
	bool path_is_directory(StringRef path);
	String path_join(ArrayRef<const StringRef> components, IAllocator& alloc = default_allocator());
	StringRef path_chomp(StringRef path);
	String path_absolute(StringRef relpath, IAllocator& alloc = default_allocator());
	Array<String> path_glob(StringRef pattern, IAllocator& alloc = default_allocator());
	Maybe<SystemTime> file_modification_time(StringRef path);
}


#endif
