#include "io/network_stream.hpp"
#include "base/raise.hpp"
#include "memory/unique_ptr.hpp"
#include "io/fd.hpp"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

namespace grace {
	struct SocketNetworkStream : NetworkStream {
		int fd = -1;
		String host_;
		String address_;
		uint16 port_ = 0;
		uint16 local_port_ = 0;

		explicit SocketNetworkStream(int fd) : fd(fd) {}

		virtual ~SocketNetworkStream() {
			close();
		}

		// NetworkStream
		bool is_open() const final {
			return fd >= 0;
		}

		void close() final {
			if (fd >= 0) {
				::close(fd);
				fd = -1;
			}
		}

		StringRef host() const final { return host_; }
		StringRef address() const final { return address_; }
		uint16 port() const final { return port_; }
		uint16 local_port() const final { return local_port_; }
		uintptr_t handle() const final { return (uintptr_t)fd; }

		// IInputStream
		bool is_readable() const final { return true; }
		Either<size_t, IOEvent> read(byte* buffer, size_t max) final;
		size_t tell_read() const final { return 0; }
		bool seek_read(size_t position) final { return false; }
		bool has_length() const final { return false; }
		size_t length() const final { return SIZE_T_MAX; }
		
		bool is_read_nonblocking() const final;
		void set_read_nonblocking(bool) final;

		// IOutputStream
		bool is_writable() const final { return true; }
		Either<size_t, IOEvent> write(const byte* buffer, size_t max) final;
		size_t tell_write() const final { return 0; }
		bool seek_write(size_t position) final { return false; }
		void flush() final {}
		
		bool is_write_nonblocking() const final;
		void set_write_nonblocking(bool) final;
	};

	UniquePtr<INetworkStream> NetworkStream::connect(StringRef host, uint16 port, IAllocator& alloc) {
		int fd = ::socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0) {
			raise<NetworkStreamError>("socket: {0}", ::strerror(errno));
		}
		auto stream = make_unique<SocketNetworkStream>(default_allocator(), fd);
		
		COPY_STRING_REF_TO_CSTR_BUFFER(host_cstr, host);
		struct hostent* server = ::gethostbyname2(host_cstr.data(), AF_INET);
		if (server == nullptr) {
			raise<NetworkStreamError>("gethostbyname2: No such host.");
		}
		stream->host_ = server->h_name;
		stream->address_ = server->h_addr_list[0];
		struct sockaddr_in serv_addr;
		serv_addr.sin_family = AF_INET;
		::memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
		serv_addr.sin_port = htons(port);
		stream->port_ = port;
		
		if (::connect(fd, (const sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
			raise<NetworkStreamError>("connect: {0}", ::strerror(errno));
		}
		
		return move(stream);
	}

	Either<size_t, IOEvent> SocketNetworkStream::read(byte *buffer, size_t max) {
		ssize_t n = ::read(fd, buffer, max);
		if (n < 0) {
			if (errno == EWOULDBLOCK) {
				return IOEvent::WouldBlock;
			} else {
				raise<NetworkStreamError>("read: {0}", ::strerror(errno));
			}
		} else if (n == 0 && max != 0) {
			return IOEvent::EndOfStream;
		}
		return (size_t)n;
	}
	
	bool SocketNetworkStream::is_read_nonblocking() const {
		return is_nonblocking(fd);
	}
	
	void SocketNetworkStream::set_read_nonblocking(bool b) {
		set_nonblocking(fd, b);
	}

	Either<size_t, IOEvent> SocketNetworkStream::write(const byte* buffer, size_t max) {
		ssize_t n = ::write(fd, buffer, max);
		if (n < 0) {
			if (errno == EWOULDBLOCK) {
				return IOEvent::WouldBlock;
			} else {
				raise<NetworkStreamError>("write: {0}", ::strerror(errno));
			}
		} else if (n == 0 && max != 0) {
			return IOEvent::EndOfStream;
		}
		return (size_t)n;
	}
	
	bool SocketNetworkStream::is_write_nonblocking() const {
		return is_nonblocking(fd);
	}
	
	void SocketNetworkStream::set_write_nonblocking(bool b) {
		set_nonblocking(fd, b);
	}
}