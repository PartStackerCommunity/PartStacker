#ifndef PSTACK_CALC_ROTATIONS_HPP
#define PSTACK_CALC_ROTATIONS_HPP

#include "pstack/geo/matrix3.hpp"
#include <array>
#include <span>

namespace pstack::calc {

inline constexpr std::array no_rotations = {
    geo::eye3<float>,
};

inline PSTACK_BIT_CAST_CONSTEXPR const std::array cubic_rotations = {
    geo::eye3<float>,
    geo::rot3<float>({ 1, 0, 0 }, 90),
    geo::rot3<float>({ 1, 0, 0 }, 180),
    geo::rot3<float>({ 1, 0, 0 }, 270),
    geo::rot3<float>({ 0, 1, 0 }, 90),
    geo::rot3<float>({ 0, 1, 0 }, 180),
    geo::rot3<float>({ 0, 1, 0 }, 270),
    geo::rot3<float>({ 0, 0, 1 }, 90),
    geo::rot3<float>({ 0, 0, 1 }, 180),
    geo::rot3<float>({ 0, 0, 1 }, 270),
    geo::rot3<float>({ 1, 1, 0 }, 180),
    geo::rot3<float>({ 1, -1, 0 }, 180),
    geo::rot3<float>({ 0, 1, 1 }, 180),
    geo::rot3<float>({ 0, -1, 1 }, 180),
    geo::rot3<float>({ 1, 0, 1 }, 180),
    geo::rot3<float>({ 1, 0, -1 }, 180),
    geo::rot3<float>({ 1, 1, 1 }, 120),
    geo::rot3<float>({ 1, 1, 1 }, 240),
    geo::rot3<float>({ -1, 1, 1 }, 120),
    geo::rot3<float>({ -1, 1, 1 }, 240),
    geo::rot3<float>({ 1, -1, 1 }, 120),
    geo::rot3<float>({ 1, -1, 1 }, 240),
    geo::rot3<float>({ 1, 1, -1 }, 120),
    geo::rot3<float>({ 1, 1, -1 }, 240),
};

extern const std::array<geo::matrix3<float>, 32> arbitrary_rotations;

inline constexpr std::array<std::span<const geo::matrix3<float>>, 3> rotation_sets = {
    no_rotations,
    cubic_rotations,
    arbitrary_rotations,
};

} // namespace pstack::calc

#endif // PSTACK_CALC_ROTATIONS_HPP
