#ifndef PSTACK_CALC_BINPACK_TYPES_HPP
#define PSTACK_CALC_BINPACK_TYPES_HPP

#include <array>
#include <cstdint>
#include <vector>

#include "pstack/geo/vector3.hpp"

namespace pstack::calc::binpack {

struct Item {
  uint32_t id{};
  uint32_t score{};
  geo::vector3<int> size{};
};

struct PackingBox {
  geo::vector3<int> origin{};
  geo::vector3<int> size{};

  template <geo::Axis axis> struct Offset {
    int value;
  };

  // Split the box into 3 smaller boxes along 3 given axes at 3 given offsets
  template <geo::Axis axis1, geo::Axis axis2, geo::Axis axis3>
  std::array<PackingBox, 3> split(Offset<axis1> offset1, Offset<axis2> offset2,
                                  Offset<axis3> offset3) const {
    auto [reduced_1, remaining_1] = this->split(offset1);
    auto [reduced_2, remaining_2] = reduced_1.split(offset2);
    auto [_, remaining_3] = reduced_2.split(offset3);

    return {remaining_1, remaining_2, remaining_3};
  }

private:
  // Split the box into 2 smaller boxes along a given axis at a given offset
  template <geo::Axis axis>
  std::array<PackingBox, 2> split(Offset<axis> offset) const {
    PackingBox piece1{*this};
    PackingBox piece2{*this};

    piece1.size[axis] = offset;
    piece2.size[axis] -= offset;
    piece2.origin[axis] += offset;

    return {piece1, piece2};
  }
};

struct ItemPlacement {
  Item item;
  geo::vector3<int> position{};
};

struct PackingResult {
  uint32_t total_score{};
  std::vector<ItemPlacement> placements;

  void translate(const geo::vector3<int> &offset) {
    for (auto &placement : placements) {
      placement.position += offset;
    }
  }
};

} // namespace pstack::calc::binpack

#endif // PSTACK_CALC_BINPACK_TYPES_HPP
