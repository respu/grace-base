#pragma once
#ifndef DESERIALIZE_OBJECT_HPP_F2934JFR
#define DESERIALIZE_OBJECT_HPP_F2934JFR

#include "object/objectptr.hpp"

namespace falling {
struct IUniverse;
struct ArchiveNode;

ObjectPtr<> deserialize_object(const ArchiveNode& representation, IUniverse& universe);

}

#endif /* end of include guard: DESERIALIZE_OBJECT_HPP_F2934JFR */
