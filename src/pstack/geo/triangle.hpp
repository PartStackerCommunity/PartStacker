#ifndef PSTACK_GEO_TRIANGLE_HPP
#define PSTACK_GEO_TRIANGLE_HPP

#include "pstack/geo/point3.hpp"
#include "pstack/geo/vector3.hpp"

namespace pstack::geo {

struct triangle {
    vector3<float> normal;
    point3<float> v1, v2, v3;
};

} // namespace pstack::geo

#endif // PSTACK_GEO_TRIANGLE_HPP
