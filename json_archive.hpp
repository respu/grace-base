#pragma once
#ifndef JSON_ARCHIVE_HPP_4OX35IUJ
#define JSON_ARCHIVE_HPP_4OX35IUJ

#include "serialization/archive.hpp"
#include "serialization/archive_node.hpp"
#include "base/bag.hpp"
#include "base/map.hpp"
#include "base/string.hpp"

namespace grace {
struct JSONArchive : Archive {
	void write(OutputStream& os) const override;
	size_t read(InputStream& is, String& out_error) override;
private:
	void write_node(const ArchiveNode&, OutputStream& os, bool print_inline, int indent) const;
	bool read_node(ArchiveNode&, const byte*& p, const byte* end, String& out_error);
};
}

#endif /* end of include guard: JSON_ARCHIVE_HPP_4OX35IUJ */
