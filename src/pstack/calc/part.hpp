#ifndef PSTACK_CALC_PART_HPP
#define PSTACK_CALC_PART_HPP

#include <optional>
#include <string>
#include "pstack/calc/mesh.hpp"

namespace pstack::calc {

// `part_base` is the part data needed to save and load
struct part_base {
    std::string mesh_file;
    int quantity;
    bool mirrored;
    int min_hole;
    int rotation_index;
    bool rotate_min_box;
};

// `part` is everything that can be derived from `part_base`
struct part : part_base {
    std::string name;
    mesh mesh;

    std::optional<int> base_quantity;

    double volume;
    geo::point3<float> centroid;
    int triangle_count;
};

} // namespace pstack::calc

#endif // PSTACK_CALC_PART_HPP
