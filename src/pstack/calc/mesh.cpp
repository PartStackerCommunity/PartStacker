#include "pstack/calc/mesh.hpp"

namespace pstack::calc {

void mesh::mirror_x() {
    for (auto& t : _triangles) {
        t.normal.x = -t.normal.x;
        t.v1.x = -t.v1.x;
        t.v2.x = -t.v2.x;
        t.v3.x = -t.v3.x;
        std::swap(t.v2, t.v3);
    }
}

void mesh::set_baseline(const geo::point3<float> baseline) {
    const geo::point3 min = bounding().min;
    const geo::vector3 offset = baseline - min;
    for (auto& t : _triangles) {
        t.v1 += offset;
        t.v2 += offset;
        t.v3 += offset;
    }
}

mesh::bounding_t mesh::bounding() const {
    bounding_t out;
    out.min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
    out.max = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };

    for (const auto& triangle : _triangles) {
        out.min = geo::min(out.min, triangle.v1);
        out.min = geo::min(out.min, triangle.v2);
        out.min = geo::min(out.min, triangle.v3);
        out.max = geo::max(out.max, triangle.v1);
        out.max = geo::max(out.max, triangle.v2);
        out.max = geo::max(out.max, triangle.v3);
    }

    const geo::vector3<float> size = out.max - out.min;
    out.box_size = { geo::ceil(size.x + 2), geo::ceil(size.y + 2), geo::ceil(size.z + 2) };
    return out;
}

mesh::volume_and_centroid_t mesh::volume_and_centroid() const {
    double total_volume = 0;
    geo::vector3<float> total_centroid = { 0, 0, 0 };
    for (const auto& t : _triangles) {
        const auto volume_piece = geo::dot(t.v1.as_vector(), geo::cross(t.v2.as_vector(), t.v3.as_vector()));
        total_volume += volume_piece;
        total_centroid += volume_piece * (t.v1.as_vector() + t.v2.as_vector() + t.v3.as_vector());
    }
    // The `/6` and `/4` should actually go on the two calculation lines above,
    // but they're factored out to the final result for efficiency. 
    return { .volume = total_volume / 6, .centroid = ((total_centroid / 4) / total_volume) + geo::origin3<float> };
}

} // namespace pstack::calc
