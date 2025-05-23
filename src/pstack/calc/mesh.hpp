#ifndef PSTACK_CALC_MESH_HPP
#define PSTACK_CALC_MESH_HPP

#include "pstack/calc/sinterbox.hpp"
#include "pstack/geo/functions.hpp"
#include "pstack/geo/matrix3.hpp"
#include "pstack/geo/triangle.hpp"
#include <vector>

namespace pstack::calc {

class mesh {
private:
    std::vector<geo::triangle> _triangles{};

public:
    mesh() = default;
    mesh(std::vector<geo::triangle> triangles)
        : _triangles(std::move(triangles)) {}

    const std::vector<geo::triangle>& triangles() const& {
        return _triangles;
    }

    void add(const mesh& m, const geo::vector3<float> translation);

    void mirror_x();
    void scale(double factor);
    void rotate(const geo::matrix3<float>& rotation);
    geo::vector3<float> set_baseline(const geo::point3<float> baseline);

    void add_sinterbox(const sinterbox_parameters& params) {
        append_sinterbox(_triangles, params);
    }

    struct bounding_t {
        geo::point3<float> min;
        geo::point3<float> max;
        geo::vector3<int> box_size;
    };

    bounding_t bounding() const;

    struct volume_and_centroid_t {
        double volume;
        geo::point3<float> centroid;
    };

    volume_and_centroid_t volume_and_centroid() const;
};

} // namespace pstack::calc

#endif // PSTACK_CALC_MESH_HPP
