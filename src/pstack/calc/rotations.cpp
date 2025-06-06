#include "pstack/calc/rotations.hpp"
#include <random>

namespace pstack::calc {

const std::array<geo::matrix3<float>, 32> arbitrary_rotations = [] {
    std::array<geo::matrix3<float>, 32> out;
    out[0] = geo::eye3<float>;
    out[1] = geo::rot3<float>({ 1, 1, 1 }, 120);
    out[2] = geo::rot3<float>({ 1, 1, 1 }, 240);
    out[3] = geo::rot3<float>({ 1, 0, 0 }, 180);
    out[4] = geo::rot3<float>({ 0, 1, 0 }, 180);
    out[5] = geo::rot3<float>({ 0, 0, 1 }, 180);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0, 2 * geo::pi);
    for (std::size_t i = 6; i != 32; ++i) {
        out[i] = geo::rot3_z(dis(gen)) * geo::rot3_y(dis(gen)) * geo::rot3_x(dis(gen));
    }
    return out;
}();

} // namespace pstack::calc
