#include "pstack/geo/matrix4.hpp"
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

TEMPLATE_TEST_CASE("properties", "[matrix4]",
                   int, long, double, float)
{
    using T = TestType;
    STATIC_CHECK(std::is_aggregate_v<matrix4<T>>);
    STATIC_CHECK(sizeof(matrix4<T>) == 16 * sizeof(T));
}

TEMPLATE_TEST_CASE("operator==()", "[matrix4]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = g.generate<matrix4<T>, matrix4<T>>();
    CHECK(lhs == lhs);
    CHECK(rhs == rhs);
    CHECK(lhs != rhs);
    CHECK(rhs != lhs);
}

TEMPLATE_TEST_CASE("two-matrix operator*()", "[matrix4]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = g.generate<matrix4<T>, matrix4<T>>();
    const matrix4<T> expected{
        (lhs.xx * rhs.xx) + (lhs.xy * rhs.yx) + (lhs.xz * rhs.zx) + (lhs.xw * rhs.wx),
        (lhs.xx * rhs.xy) + (lhs.xy * rhs.yy) + (lhs.xz * rhs.zy) + (lhs.xw * rhs.wy),
        (lhs.xx * rhs.xz) + (lhs.xy * rhs.yz) + (lhs.xz * rhs.zz) + (lhs.xw * rhs.wz),
        (lhs.xx * rhs.xw) + (lhs.xy * rhs.yw) + (lhs.xz * rhs.zw) + (lhs.xw * rhs.ww),
        (lhs.yx * rhs.xx) + (lhs.yy * rhs.yx) + (lhs.yz * rhs.zx) + (lhs.yw * rhs.wx),
        (lhs.yx * rhs.xy) + (lhs.yy * rhs.yy) + (lhs.yz * rhs.zy) + (lhs.yw * rhs.wy),
        (lhs.yx * rhs.xz) + (lhs.yy * rhs.yz) + (lhs.yz * rhs.zz) + (lhs.yw * rhs.wz),
        (lhs.yx * rhs.xw) + (lhs.yy * rhs.yw) + (lhs.yz * rhs.zw) + (lhs.yw * rhs.ww),
        (lhs.zx * rhs.xx) + (lhs.zy * rhs.yx) + (lhs.zz * rhs.zx) + (lhs.zw * rhs.wx),
        (lhs.zx * rhs.xy) + (lhs.zy * rhs.yy) + (lhs.zz * rhs.zy) + (lhs.zw * rhs.wy),
        (lhs.zx * rhs.xz) + (lhs.zy * rhs.yz) + (lhs.zz * rhs.zz) + (lhs.zw * rhs.wz),
        (lhs.zx * rhs.xw) + (lhs.zy * rhs.yw) + (lhs.zz * rhs.zw) + (lhs.zw * rhs.ww),
        (lhs.wx * rhs.xx) + (lhs.wy * rhs.yx) + (lhs.wz * rhs.zx) + (lhs.ww * rhs.wx),
        (lhs.wx * rhs.xy) + (lhs.wy * rhs.yy) + (lhs.wz * rhs.zy) + (lhs.ww * rhs.wy),
        (lhs.wx * rhs.xz) + (lhs.wy * rhs.yz) + (lhs.wz * rhs.zz) + (lhs.ww * rhs.wz),
        (lhs.wx * rhs.xw) + (lhs.wy * rhs.yw) + (lhs.wz * rhs.zw) + (lhs.ww * rhs.ww),
    };
    CHECK(lhs * rhs == expected);
}

TEMPLATE_TEST_CASE("matrix-matrix-matrix commutative operator*()", "[matrix4]",
                   double, float)
{
    using T = TestType;
    const auto [a, b, c] = g.generate<matrix4<T>, matrix4<T>, matrix4<T>>();
    const matrix4<T> lhs = (a * b) * c;
    const matrix4<T> rhs = a * (b * c);
    CHECK_THAT(lhs.xx, WithinRel(rhs.xx, rel_epsilon<T>));
    CHECK_THAT(lhs.xy, WithinRel(rhs.xy, rel_epsilon<T>));
    CHECK_THAT(lhs.xz, WithinRel(rhs.xz, rel_epsilon<T>));
    CHECK_THAT(lhs.xw, WithinRel(rhs.xw, rel_epsilon<T>));
    CHECK_THAT(lhs.yx, WithinRel(rhs.yx, rel_epsilon<T>));
    CHECK_THAT(lhs.yy, WithinRel(rhs.yy, rel_epsilon<T>));
    CHECK_THAT(lhs.yz, WithinRel(rhs.yz, rel_epsilon<T>));
    CHECK_THAT(lhs.yw, WithinRel(rhs.yw, rel_epsilon<T>));
    CHECK_THAT(lhs.zx, WithinRel(rhs.zx, rel_epsilon<T>));
    CHECK_THAT(lhs.zy, WithinRel(rhs.zy, rel_epsilon<T>));
    CHECK_THAT(lhs.zz, WithinRel(rhs.zz, rel_epsilon<T>));
    CHECK_THAT(lhs.zw, WithinRel(rhs.zw, rel_epsilon<T>));
    CHECK_THAT(lhs.wx, WithinRel(rhs.wx, rel_epsilon<T>));
    CHECK_THAT(lhs.wy, WithinRel(rhs.wy, rel_epsilon<T>));
    CHECK_THAT(lhs.wz, WithinRel(rhs.wz, rel_epsilon<T>));
    CHECK_THAT(lhs.ww, WithinRel(rhs.ww, rel_epsilon<T>));
}

TEMPLATE_TEST_CASE("eye4", "[matrix4]",
                   int, long, double, float)
{
    using T = TestType;
    STATIC_CHECK(std::is_same_v<decltype(eye4<T>), const matrix4<T>>);
    STATIC_CHECK(eye4<T>.xx == T{1});
    STATIC_CHECK(eye4<T>.xy == T{0});
    STATIC_CHECK(eye4<T>.xz == T{0});
    STATIC_CHECK(eye4<T>.xw == T{0});
    STATIC_CHECK(eye4<T>.yx == T{0});
    STATIC_CHECK(eye4<T>.yy == T{1});
    STATIC_CHECK(eye4<T>.yz == T{0});
    STATIC_CHECK(eye4<T>.yw == T{0});
    STATIC_CHECK(eye4<T>.zx == T{0});
    STATIC_CHECK(eye4<T>.zy == T{0});
    STATIC_CHECK(eye4<T>.zz == T{1});
    STATIC_CHECK(eye4<T>.zw == T{0});
    STATIC_CHECK(eye4<T>.wx == T{0});
    STATIC_CHECK(eye4<T>.wy == T{0});
    STATIC_CHECK(eye4<T>.wz == T{0});
    STATIC_CHECK(eye4<T>.ww == T{1});
}

TEMPLATE_TEST_CASE("rot4_x()", "[matrix4]",
                   double, float)
{
    using T = TestType;
    const auto theta = g.generate<T>();
    const auto c = std::cos(theta);
    const auto s = std::sin(theta);
    const matrix4<T> actual = rot4_x(theta);
    CHECK(actual.xx == 1);
    CHECK(actual.xy == 0);
    CHECK(actual.xz == 0);
    CHECK(actual.xw == 0);
    CHECK(actual.yx == 0);
    CHECK_THAT(actual.yy, WithinAbs(c, epsilon<T>));
    CHECK_THAT(actual.yz, WithinAbs(-s, epsilon<T>));
    CHECK(actual.yw == 0);
    CHECK(actual.zx == 0);
    CHECK_THAT(actual.zy, WithinAbs(s, epsilon<T>));
    CHECK_THAT(actual.zz, WithinAbs(c, epsilon<T>));
    CHECK(actual.zw == 0);
    CHECK(actual.wx == 0);
    CHECK(actual.wy == 0);
    CHECK(actual.wz == 0);
    CHECK(actual.ww == 1);
}

TEMPLATE_TEST_CASE("rot4_y()", "[matrix4]",
                   double, float)
{
    using T = TestType;
    const auto theta = g.generate<T>();
    const auto c = std::cos(theta);
    const auto s = std::sin(theta);
    const matrix4<T> actual = rot4_y(theta);
    CHECK_THAT(actual.xx, WithinAbs(c, epsilon<T>));
    CHECK(actual.xy == 0);
    CHECK_THAT(actual.xz, WithinAbs(s, epsilon<T>));
    CHECK(actual.xw == 0);
    CHECK(actual.yx == 0);
    CHECK(actual.yy == 1);
    CHECK(actual.yz == 0);
    CHECK(actual.yw == 0);
    CHECK_THAT(actual.zx, WithinAbs(-s, epsilon<T>));
    CHECK(actual.zy == 0);
    CHECK_THAT(actual.zz, WithinAbs(c, epsilon<T>));
    CHECK(actual.zw == 0);
    CHECK(actual.wx == 0);
    CHECK(actual.wy == 0);
    CHECK(actual.wz == 0);
    CHECK(actual.ww == 1);
}

TEMPLATE_TEST_CASE("rot4_z()", "[matrix4]",
                   double, float)
{
    using T = TestType;
    const auto theta = g.generate<T>();
    const auto c = std::cos(theta);
    const auto s = std::sin(theta);
    const matrix4<T> actual = rot4_z(theta);
    CHECK_THAT(actual.xx, WithinAbs(c, epsilon<T>));
    CHECK_THAT(actual.xy, WithinAbs(-s, epsilon<T>));
    CHECK(actual.xz == 0);
    CHECK(actual.xw == 0);
    CHECK_THAT(actual.yx, WithinAbs(s, epsilon<T>));
    CHECK_THAT(actual.yy, WithinAbs(c, epsilon<T>));
    CHECK(actual.yz == 0);
    CHECK(actual.yw == 0);
    CHECK(actual.zx == 0);
    CHECK(actual.zy == 0);
    CHECK(actual.zz == 1);
    CHECK(actual.zw == 0);
    CHECK(actual.wx == 0);
    CHECK(actual.wy == 0);
    CHECK(actual.wz == 0);
    CHECK(actual.ww == 1);
}

TEMPLATE_TEST_CASE("scale4()", "[matrix4]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [x_factor, y_factor, z_factor] = g.generate<T, T, T>();
    const matrix4<T> actual = scale4(x_factor, y_factor, z_factor);
    CHECK(actual.xx == x_factor);
    CHECK(actual.xy == 0);
    CHECK(actual.xz == 0);
    CHECK(actual.xw == 0);
    CHECK(actual.xw == 0);
    CHECK(actual.yx == 0);
    CHECK(actual.yy == y_factor);
    CHECK(actual.yz == 0);
    CHECK(actual.yw == 0);
    CHECK(actual.zx == 0);
    CHECK(actual.zy == 0);
    CHECK(actual.zz == z_factor);
    CHECK(actual.zw == 0);
    CHECK(actual.wx == 0);
    CHECK(actual.wy == 0);
    CHECK(actual.wz == 0);
    CHECK(actual.ww == 1);
}

TEMPLATE_TEST_CASE("translate4()", "[matrix4]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [x_factor, y_factor, z_factor] = g.generate<T, T, T>();
    const matrix4<T> actual = translate4(x_factor, y_factor, z_factor);
    CHECK(actual.xx == 1);
    CHECK(actual.xy == 0);
    CHECK(actual.xz == 0);
    CHECK(actual.xw == x_factor);
    CHECK(actual.yx == 0);
    CHECK(actual.yy == 1);
    CHECK(actual.yz == 0);
    CHECK(actual.yw == y_factor);
    CHECK(actual.zx == 0);
    CHECK(actual.zy == 0);
    CHECK(actual.zz == 1);
    CHECK(actual.zw == z_factor);
    CHECK(actual.wx == 0);
    CHECK(actual.wy == 0);
    CHECK(actual.wz == 0);
    CHECK(actual.ww == 1);
}

} // namespace
} // namespace pstack::geo
