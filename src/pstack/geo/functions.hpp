#ifndef PSTACK_GEO_FUNCTIONS_HPP
#define PSTACK_GEO_FUNCTIONS_HPP

#include <bit>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <numbers>

namespace pstack::geo {

static_assert(sizeof(double) == sizeof(std::uint64_t));

using std::numbers::pi;
using std::numbers::sqrt2;

struct radians;
struct degrees;

struct radians {
    explicit radians(double val) : value(val) {}
    radians(degrees d);
    double value;
};
struct degrees {
    explicit degrees(double val) : value(val) {}
    degrees(radians r);
    double value;
};

inline radians::radians(degrees d) : value(d.value * (pi / 180)) {}
inline degrees::degrees(radians r) : value(r.value * (180 / pi)) {}

constexpr double sin(radians r) {
    double x = r.value;
    while (x > pi) {
        x -= 2 * pi;
    }
    while (x < -pi) {
        x += 2 * pi;
    }

    const int sign = (x < 0) * (-1) + (x >= 0) * (1);
    x *= sign;

    if (x > (pi / 2)) {
        x = pi - x;
    }

    constexpr double factors[] = { // Reciprocal of factorials, switching signs
        -0.1666666666666666666666666666667, 0.0083333333333333333333333333333, -0.0001984126984126984126984126984,
        0.0000027557319223985890652557319, -0.0000000250521083854417187750521, 0.0000000001605904383682161459939,
        -0.0000000000007647163731819816476, 0.0000000000000028114572543455208, -0.0000000000000000082206352466243,
    };

    const double x_squared = x * x;

    double result = x;
    for (const double factor : factors) {
        x *= x_squared;
        result += x * factor;
    }
    return result * sign;
}

constexpr double cos(radians r) {
    double x = r.value;
    while (x > pi) {
        x -= 2 * pi;
    }
    while (x < -pi) {
        x += 2 * pi;
    }

    if (x < 0) {
        x = -x;
    }

    const int sign = (x > (pi / 2)) * (-1) + (x <= (pi / 2)) * (1);
    x = (x > (pi / 2)) * (pi - x) + (x <= (pi / 2)) * (x);

    constexpr double factors[] = { // Reciprocal of factorials, switching signs
        -0.5, 0.0416666666666666666666666666667, -0.0013888888888888888888888888889, 0.0000248015873015873015873015873,
        -0.0000002755731922398589065255732, 0.0000000020876756987868098979210, -0.0000000000114707455977297247139,
        0.0000000000000477947733238738530, -0.0000000000000001561920696858623, 0.0000000000000000004110317623312,
    };

    const double x_squared = x * x;
    x = 1;

    double result = x;
    for (const double factor : factors) {
        x *= x_squared;
        result += x * factor;
    }
    return result * sign;
}

// Taken from https://stackoverflow.com/a/66146159
template <std::floating_point T>
constexpr int ceil(const T value) {
    const int i = static_cast<int>(value);
    return value > i ? i + 1 : i;
}

template <std::size_t iterations = 4>
constexpr float inverse_sqrt(const float value) {
    const std::uint32_t i = std::bit_cast<std::uint32_t>(value);
    float d = std::bit_cast<float>(0x5F37642F - (i >> 1));
    for (int iter = 0; iter < iterations; ++iter) {
        d = 0.5 * d * (3 - value * d * d);
    }
    return d;
}

template <std::size_t iterations = 4>
constexpr double inverse_sqrt(const double value) {
    const std::uint64_t i = std::bit_cast<std::uint64_t>(value);
    double d = std::bit_cast<double>(0x5FE6EB50C7B537A9 - (i >> 1));
    for (int iter = 0; iter < iterations; ++iter) {
        d = 0.5 * d * (3 - value * d * d);
    }
    return d;
}

} // namespace pstack::geo

#endif // PSTACK_GEO_FUNCTIONS_HPP
