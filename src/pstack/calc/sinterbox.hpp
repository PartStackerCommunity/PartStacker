#ifndef PSTACK_CALC_SINTERBOX_HPP
#define PSTACK_CALC_SINTERBOX_HPP

#include "pstack/geo/triangle.hpp"
#include <vector>

namespace pstack::calc {

struct sinterbox_settings {
    double clearance;
    double thickness;
    double width;
    double spacing;
};

struct sinterbox_bounding {
    geo::point3<float> min;
    geo::point3<float> max;
};

struct sinterbox_parameters {
    sinterbox_settings settings;
    sinterbox_bounding bounding;
};

void append_sinterbox(std::vector<geo::triangle>& triangles, const sinterbox_parameters& params);

} // namespace pstack::calc

#endif // PSTACK_CALC_SINTERBOX_HPP
