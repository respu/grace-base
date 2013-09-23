//
//  stdio_stream.cpp
//  grace
//
//  Created by Simon Ask Ulsnes on 30/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#include "io/stdio_stream.hpp"
#include "io/file_stream.hpp"
#include <stdio.h>
#include "memory/static_allocator.hpp"

namespace grace {
	ConsoleStream& ConsoleStream::get() {
		static ConsoleStream s;
		return s;
	}

	namespace {
		struct StandardOutputStream : FileStream {
			StandardOutputStream() : FileStream(::stdout, "stdout", FileMode::AppendCreate, false) {}
		};

		struct StandardInputStream : FileStream {
			StandardInputStream() : FileStream(::stdin, "stdin", FileMode::Read, false) {}
		};

		struct StandardErrorStream : FileStream {
			StandardErrorStream() : FileStream(::stderr, "stderr", FileMode::AppendCreate, false) {}
		};

		using StdOutStorage = typename std::aligned_storage<sizeof(StandardOutputStream), alignof(StandardOutputStream)>::type;
		using StdErrStorage = typename std::aligned_storage<sizeof(StandardErrorStream), alignof(StandardErrorStream)>::type;
		using StdInStorage = typename std::aligned_storage<sizeof(StandardInputStream), alignof(StandardInputStream)>::type;
		static StdOutStorage stdout_storage;
		static StdErrStorage stderr_storage;
		static StdInStorage stdin_storage;

		StandardOutputStream& get_stdout_stream() {
			static StandardOutputStream* p = new(&stdout_storage) StandardOutputStream;
			return *p;
		}

		StandardInputStream& get_stdin_stream() {
			static StandardInputStream* p = new(&stdin_storage) StandardInputStream;
			return *p;
		}

		StandardErrorStream& get_stderr_stream() {
			static StandardErrorStream* p = new(&stderr_storage) StandardErrorStream;
			return *p;
		}
	}
	
	struct ConsoleStream::Impl {
		FormattedStream serr_f;
		
		Impl() : serr_f(get_stderr_stream()) {}
	};
	
	namespace {
		using ImplStorage = std::aligned_storage<sizeof(ConsoleStream::Impl), alignof(ConsoleStream::Impl)>::type;
		static ImplStorage impl_storage;
	}
	
	ConsoleStream::ConsoleStream() : FormattedStream(get_stdout_stream()) {
		impl = new(&impl_storage) Impl;
	}

	FormattedStream& ConsoleStream::stderr() {
		return impl->serr_f;
	}
	
	bool ConsoleStream::autoflush() const {
		return get_stdout_stream().autoflush();
	}
	
	void ConsoleStream::set_autoflush(bool b) {
		get_stdout_stream().set_autoflush(b);
		get_stderr_stream().set_autoflush(b);
	}

	bool ConsoleStream::is_readable() const {
		return get_stdin_stream().is_readable();
	}

	size_t ConsoleStream::read(byte* buffer, size_t max) {
		return get_stdin_stream().read(buffer, max);
	}

	size_t ConsoleStream::read_if_available(byte* buffer, size_t max, bool& out_would_block) {
		return get_stdin_stream().read_if_available(buffer, max, out_would_block);
	}

	size_t ConsoleStream::tell_read() const {
		return get_stdin_stream().tell_read();
	}

	bool ConsoleStream::seek_read(size_t position) {
		return get_stdin_stream().seek_read(position);
	}

	bool ConsoleStream::has_length() const {
		return get_stdin_stream().has_length();
	}

	size_t ConsoleStream::length() const {
		return get_stdin_stream().length();
	}
}
