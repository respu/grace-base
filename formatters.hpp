//
//  formatters.hpp
//  falling
//
//  Created by Simon Ask Ulsnes on 30/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef falling_formatters_hpp
#define falling_formatters_hpp

#include "io/formatted_stream.hpp"
#include "io/string_stream.hpp"

#include <functional>

namespace falling {
	template <typename T>
	struct PrintfFormatter : Formatter {
		std::string v;
		PrintfFormatter(const char* format_string, T value) {
			char* ret;
			asprintf(&ret, format_string, value);
			v = ret;
			free(ret);
		}
		void write(FormattedStream& stream) const override { stream << v; }
	};
	
	template <typename T>
	PrintfFormatter<T> format(const char* format_string, T value) {
		return PrintfFormatter<T>(format_string, value);
	}
	
	template <typename T>
	struct TruncateFormatter : Formatter {
		const T& value;
		uint32 width;
		std::string ellipsis;
	};
	
	struct ClosureFormatter : Formatter {
		std::function<void(FormattedStream&)> closure;
		void write(FormattedStream& stream) const {
			closure(stream);
		};
	};
	
	inline ClosureFormatter closure_formatter(std::function<void(FormattedStream&)> closure) {
		ClosureFormatter formatter;
		formatter.closure = closure;
		return formatter;
	}
	
	template <typename T>
	ClosureFormatter truncate(const T& value, uint32 width, const std::string& ellipsis = "") {
		return closure_formatter([=](FormattedStream& stream) {
			StringStream ss;
			ss << value;
			auto s = ss.str();
			size_t ellipsis_length = ellipsis.size();
			if (s.size() > width) {
				if (ellipsis.size() && width > ellipsis.size()) {
					stream << s.substr(0, width - ellipsis.size()) << ellipsis;
				} else {
					stream << s.substr(0, width);
				}
			} else {
				stream << s;
			}
		});
	}
	
	template <typename T>
	struct PadOrTruncateFormatter : Formatter {
		const T& value;
		uint32 width;
		char padding;
		bool pad_left;
		PadOrTruncateFormatter(const T& input, uint32 width, char padding, bool pad_left) : value(input), width(width), padding(padding), pad_left(pad_left) {}
		void write(FormattedStream& stream) const {
			StringStream ss;
			ss << value;
			std::string s = ss.str();
			if (s.size() > width) {
				stream << s.substr(0, width);
			} else {
				size_t missing = width - s.size();
				if (pad_left) {
					for (uint32 i = 0; i < missing; ++i) {
						stream << padding;
					}
				}
				stream << s;
				if (!pad_left) {
					for (uint32 i = 0; i < missing; ++i) {
						stream << padding;
					}
				}
			}
		}
	};
	
	template <typename T>
	PadOrTruncateFormatter<T> pad_or_truncate(const T& value, uint32 width, char padding = ' ', bool pad_left = false) {
		return PadOrTruncateFormatter<T>(value, width, padding, pad_left);
	}
}

#endif
