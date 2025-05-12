#ifndef PSTACK_CALC_BINPACK_HPP
#define PSTACK_CALC_BINPACK_HPP

#include "binpack_types.hpp"

namespace pstack::calc::binpack {

PackingResult pack_items(const std::vector<Item>& items, const PackingBox& initial_box);

} // namespace pstack::calc::binpack

#endif // PSTACK_CALC_BINPACK_HPP