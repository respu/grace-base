#pragma once
#ifndef JSON_ARCHIVE_HPP_4OX35IUJ
#define JSON_ARCHIVE_HPP_4OX35IUJ

#include "serialization/archive.hpp"
#include "serialization/archive_node.hpp"
#include "base/bag.hpp"
#include "base/map.hpp"
#include "base/string.hpp"

namespace grace {
struct JSONArchive;

struct JSONArchiveNode : ArchiveNode {
	JSONArchiveNode(JSONArchive& archive);
	void write(OutputStream& os) const { write(os, false, 0); }
	void write(OutputStream& os, bool print_inline, int indent) const;
	bool read(const byte*& p, const byte* end, String& out_error);
};

struct JSONArchive : Archive {
	explicit JSONArchive(IAllocator& alloc = default_allocator());
	ArchiveNode& root() override;
	const ArchiveNode& root() const override;
	void write(OutputStream& os) const override;
	size_t read(InputStream& is, String& out_error) override;
	ArchiveNode* make() override { return make_internal(); }
	
	const ArchiveNode& empty() const { return *empty_; }
private:
	friend struct JSONArchiveNode;
	JSONArchiveNode* empty_;
	JSONArchiveNode* root_;
	ContainedBag<JSONArchiveNode> nodes_;
	JSONArchiveNode* make_internal();
};

inline JSONArchiveNode::JSONArchiveNode(JSONArchive& archive) : ArchiveNode(archive) {}
}

#endif /* end of include guard: JSON_ARCHIVE_HPP_4OX35IUJ */
