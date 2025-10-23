#include "pstack/geo/matrix3.hpp"
#include "pstack/geo/test/generate.hpp"
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

namespace pstack::geo {
namespace {

constexpr test::generator<> g{};
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;
template <std::floating_point T, int factor = 1000>
constexpr auto epsilon = std::numeric_limits<T>::epsilon()
    * (std::same_as<T, double> ? (factor * factor) : factor);
template <std::floating_point T>
constexpr auto rel_epsilon = std::numeric_limits<T>::epsilon() * 5000
    * (std::same_as<T, double> ? 100 : 1);

TEMPLATE_TEST_CASE("properties", "[matrix3]",
                   int, long, double, float)
{
    using T = TestType;
    STATIC_CHECK(std::is_aggregate_v<matrix3<T>>);
    STATIC_CHECK(sizeof(matrix3<T>) == 9 * sizeof(T));
}

TEMPLATE_TEST_CASE("operator==()", "[matrix3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = g.generate<matrix3<T>, matrix3<T>>();
    CHECK(lhs == lhs);
    CHECK(rhs == rhs);
    CHECK(lhs != rhs);
    CHECK(rhs != lhs);
}

TEMPLATE_TEST_CASE("eye3", "[matrix3]",
                   int, long, double, float)
{
    using T = TestType;
    STATIC_CHECK(std::is_same_v<decltype(eye3<T>), const matrix3<T>>);
    STATIC_CHECK(eye3<T>.xx == T{1});
    STATIC_CHECK(eye3<T>.xy == T{0});
    STATIC_CHECK(eye3<T>.xz == T{0});
    STATIC_CHECK(eye3<T>.yx == T{0});
    STATIC_CHECK(eye3<T>.yy == T{1});
    STATIC_CHECK(eye3<T>.yz == T{0});
    STATIC_CHECK(eye3<T>.zx == T{0});
    STATIC_CHECK(eye3<T>.zy == T{0});
    STATIC_CHECK(eye3<T>.zz == T{1});
}

TEMPLATE_TEST_CASE("two-matrix operator+()", "[matrix3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = g.generate<matrix3<T>, matrix3<T>>();
    const matrix3<T> expected{
        lhs.xx + rhs.xx, lhs.xy + rhs.xy, lhs.xz + rhs.xz,
        lhs.yx + rhs.yx, lhs.yy + rhs.yy, lhs.yz + rhs.yz,
        lhs.zx + rhs.zx, lhs.zy + rhs.zy, lhs.zz + rhs.zz,
    };
    CHECK(lhs + rhs == expected);
}

TEMPLATE_TEST_CASE("two-matrix operator*()", "[matrix3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = g.generate<matrix3<T>, matrix3<T>>();
    const matrix3<T> expected{
        (lhs.xx * rhs.xx) + (lhs.xy * rhs.yx) + (lhs.xz * rhs.zx),
        (lhs.xx * rhs.xy) + (lhs.xy * rhs.yy) + (lhs.xz * rhs.zy),
        (lhs.xx * rhs.xz) + (lhs.xy * rhs.yz) + (lhs.xz * rhs.zz),
        (lhs.yx * rhs.xx) + (lhs.yy * rhs.yx) + (lhs.yz * rhs.zx),
        (lhs.yx * rhs.xy) + (lhs.yy * rhs.yy) + (lhs.yz * rhs.zy),
        (lhs.yx * rhs.xz) + (lhs.yy * rhs.yz) + (lhs.yz * rhs.zz),
        (lhs.zx * rhs.xx) + (lhs.zy * rhs.yx) + (lhs.zz * rhs.zx),
        (lhs.zx * rhs.xy) + (lhs.zy * rhs.yy) + (lhs.zz * rhs.zy),
        (lhs.zx * rhs.xz) + (lhs.zy * rhs.yz) + (lhs.zz * rhs.zz),
    };
    CHECK(lhs * rhs == expected);
}

TEMPLATE_TEST_CASE("matrix-vector operator*()", "[matrix3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = g.generate<matrix3<T>, vector3<T>>();
    const vector3<T> expected{
        (lhs.xx * rhs.x) + (lhs.xy * rhs.y) + (lhs.xz * rhs.z),
        (lhs.yx * rhs.x) + (lhs.yy * rhs.y) + (lhs.yz * rhs.z),
        (lhs.zx * rhs.x) + (lhs.zy * rhs.y) + (lhs.zz * rhs.z),
    };
    CHECK(lhs * rhs == expected);
}

TEMPLATE_TEST_CASE("matrix-matrix-matrix commutative operator*()", "[matrix3]",
                   double, float)
{
    using T = TestType;
    const auto [a, b, c] = g.generate<matrix3<T>, matrix3<T>, matrix3<T>>();
    const matrix3<T> lhs = (a * b) * c;
    const matrix3<T> rhs = a * (b * c);
    CHECK_THAT(lhs.xx, WithinRel(rhs.xx, rel_epsilon<T>));
    CHECK_THAT(lhs.xy, WithinRel(rhs.xy, rel_epsilon<T>));
    CHECK_THAT(lhs.xz, WithinRel(rhs.xz, rel_epsilon<T>));
    CHECK_THAT(lhs.yx, WithinRel(rhs.yx, rel_epsilon<T>));
    CHECK_THAT(lhs.yy, WithinRel(rhs.yy, rel_epsilon<T>));
    CHECK_THAT(lhs.yz, WithinRel(rhs.yz, rel_epsilon<T>));
    CHECK_THAT(lhs.zx, WithinRel(rhs.zx, rel_epsilon<T>));
    CHECK_THAT(lhs.zy, WithinRel(rhs.zy, rel_epsilon<T>));
    CHECK_THAT(lhs.zz, WithinRel(rhs.zz, rel_epsilon<T>));
}

TEMPLATE_TEST_CASE("matrix-matrix-vector commutative operator*()", "[matrix3]",
                   double, float)
{
    using T = TestType;
    const auto [a, b, c] = g.generate<matrix3<T>, matrix3<T>, vector3<T>>();
    const vector3<T> lhs = (a * b) * c;
    const vector3<T> rhs = a * (b * c);
    CHECK_THAT(lhs.x, WithinRel(rhs.x, rel_epsilon<T>));
    CHECK_THAT(lhs.y, WithinRel(rhs.y, rel_epsilon<T>));
    CHECK_THAT(lhs.z, WithinRel(rhs.z, rel_epsilon<T>));
}

TEMPLATE_TEST_CASE("matrix-scalar operator*()", "[matrix3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = g.generate<matrix3<T>, T>();
    const matrix3<T> expected{
        lhs.xx * rhs, lhs.xy * rhs, lhs.xz * rhs,
        lhs.yx * rhs, lhs.yy * rhs, lhs.yz * rhs,
        lhs.zx * rhs, lhs.zy * rhs, lhs.zz * rhs,
    };
    CHECK(lhs * rhs == expected);
    CHECK(rhs * lhs == expected);
}

TEMPLATE_TEST_CASE("rot3()", "[matrix3]",
                   double, float)
{
    using T = TestType;
    const auto [axis, theta] = g.generate<vector3<T>, T>();
    const auto c = std::cos(theta);
    const auto s = std::sin(theta);
    const matrix3<T> actual = rot3(axis, geo::radians{theta});
    const auto n = normalize(axis);
    const matrix3<T> K = {    0, -n.z,  n.y,
                            n.z,    0, -n.x,
                           -n.y,  n.x,    0 };
    const matrix3<T> expected = eye3<T> + (s * K) + ((1 - c) * K * K);
    CHECK_THAT(actual.xx, WithinAbs(expected.xx, epsilon<T, 500>));
    CHECK_THAT(actual.xy, WithinAbs(expected.xy, epsilon<T, 500>));
    CHECK_THAT(actual.xz, WithinAbs(expected.xz, epsilon<T, 500>));
    CHECK_THAT(actual.yx, WithinAbs(expected.yx, epsilon<T, 500>));
    CHECK_THAT(actual.yy, WithinAbs(expected.yy, epsilon<T, 500>));
    CHECK_THAT(actual.yz, WithinAbs(expected.yz, epsilon<T, 500>));
    CHECK_THAT(actual.zx, WithinAbs(expected.zx, epsilon<T, 500>));
    CHECK_THAT(actual.zy, WithinAbs(expected.zy, epsilon<T, 500>));
    CHECK_THAT(actual.zz, WithinAbs(expected.zz, epsilon<T, 500>));
}

TEMPLATE_TEST_CASE("rot3_x()", "[matrix3]",
                   double, float)
{
    using T = TestType;
    const auto theta = g.generate<T>();
    const auto c = std::cos(theta);
    const auto s = std::sin(theta);
    const matrix3<T> actual = rot3_x<T>(geo::radians{theta});
    CHECK(actual.xx == 1);
    CHECK(actual.xy == 0);
    CHECK(actual.xz == 0);
    CHECK(actual.yx == 0);
    CHECK_THAT(actual.yy, WithinAbs(c, epsilon<T>));
    CHECK_THAT(actual.yz, WithinAbs(-s, epsilon<T>));
    CHECK(actual.zx == 0);
    CHECK_THAT(actual.zy, WithinAbs(s, epsilon<T>));
    CHECK_THAT(actual.zz, WithinAbs(c, epsilon<T>));
}

TEMPLATE_TEST_CASE("rot3_y()", "[matrix3]",
                   double, float)
{
    using T = TestType;
    const auto theta = g.generate<T>();
    const auto c = std::cos(theta);
    const auto s = std::sin(theta);
    const matrix3<T> actual = rot3_y<T>(geo::radians{theta});
    CHECK_THAT(actual.xx, WithinAbs(c, epsilon<T>));
    CHECK(actual.xy == 0);
    CHECK_THAT(actual.xz, WithinAbs(s, epsilon<T>));
    CHECK(actual.yx == 0);
    CHECK(actual.yy == 1);
    CHECK(actual.yz == 0);
    CHECK_THAT(actual.zx, WithinAbs(-s, epsilon<T>));
    CHECK(actual.zy == 0);
    CHECK_THAT(actual.zz, WithinAbs(c, epsilon<T>));
}

TEMPLATE_TEST_CASE("rot3_z()", "[matrix3]",
                   double, float)
{
    using T = TestType;
    const auto theta = g.generate<T>();
    const auto c = std::cos(theta);
    const auto s = std::sin(theta);
    const matrix3<T> actual = rot3_z<T>(geo::radians{theta});
    CHECK_THAT(actual.xx, WithinAbs(c, epsilon<T>));
    CHECK_THAT(actual.xy, WithinAbs(-s, epsilon<T>));
    CHECK(actual.xz == 0);
    CHECK_THAT(actual.yx, WithinAbs(s, epsilon<T>));
    CHECK_THAT(actual.yy, WithinAbs(c, epsilon<T>));
    CHECK(actual.yz == 0);
    CHECK(actual.zx == 0);
    CHECK(actual.zy == 0);
    CHECK(actual.zz == 1);
}

TEMPLATE_TEST_CASE("scale3()", "[matrix3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [x_factor, y_factor, z_factor] = g.generate<T, T, T>();
    const matrix3<T> actual = scale3(x_factor, y_factor, z_factor);
    CHECK(actual.xx == x_factor);
    CHECK(actual.xy == 0);
    CHECK(actual.xz == 0);
    CHECK(actual.yx == 0);
    CHECK(actual.yy == y_factor);
    CHECK(actual.yz == 0);
    CHECK(actual.zx == 0);
    CHECK(actual.zy == 0);
    CHECK(actual.zz == z_factor);
}

} // namespace
} // namespace pstack::geo
