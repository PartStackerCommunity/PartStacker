#include "pstack/geo/functions.hpp"
#include "pstack/geo/test/generate.hpp"
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

namespace pstack::geo {
namespace {

constexpr test::generator<-10, 10> g{};
using Catch::Matchers::WithinAbs;
template <std::floating_point T>
constexpr auto epsilon = std::numeric_limits<T>::epsilon() * (std::same_as<T, double> ? 400 : 20);

TEST_CASE("pi", "[functions]") {
    STATIC_CHECK(pi == std::numbers::pi);
}

TEST_CASE("sqrt2", "[functions]") {
    STATIC_CHECK(sqrt2 == std::numbers::sqrt2);
}

TEST_CASE("sin", "[functions]") {
    const auto num = g.generate<double>();
    CHECK_THAT(sin(radians{num}), WithinAbs(std::sin(num), epsilon<double>));
}

TEST_CASE("sin specific radian values", "[functions]") {
    CHECK_THAT(sin(radians{0}), WithinAbs(0, epsilon<double>));
    CHECK_THAT(sin(radians{pi / 4}), WithinAbs(sqrt2 / 2, epsilon<double>));
    CHECK_THAT(sin(radians{pi / 2}), WithinAbs(1, epsilon<double>));
    CHECK_THAT(sin(radians{3 * pi / 4}), WithinAbs(sqrt2 / 2, epsilon<double>));
    CHECK_THAT(sin(radians{pi}), WithinAbs(0, epsilon<double>));
    CHECK_THAT(sin(radians{5 * pi / 4}), WithinAbs(-sqrt2 / 2, epsilon<double>));
    CHECK_THAT(sin(radians{3 * pi / 2}), WithinAbs(-1, epsilon<double>));
    CHECK_THAT(sin(radians{7 * pi / 4}), WithinAbs(-sqrt2 / 2, epsilon<double>));
    CHECK_THAT(sin(radians{2 * pi}), WithinAbs(0, epsilon<double>));
}

TEST_CASE("sin specific degree values", "[functions]") {
    CHECK_THAT(sin(degrees{0}), WithinAbs(0, epsilon<double>));
    CHECK_THAT(sin(degrees{45}), WithinAbs(sqrt2 / 2, epsilon<double>));
    CHECK_THAT(sin(degrees{90}), WithinAbs(1, epsilon<double>));
    CHECK_THAT(sin(degrees{135}), WithinAbs(sqrt2 / 2, epsilon<double>));
    CHECK_THAT(sin(degrees{180}), WithinAbs(0, epsilon<double>));
    CHECK_THAT(sin(degrees{225}), WithinAbs(-sqrt2 / 2, epsilon<double>));
    CHECK_THAT(sin(degrees{270}), WithinAbs(-1, epsilon<double>));
    CHECK_THAT(sin(degrees{315}), WithinAbs(-sqrt2 / 2, epsilon<double>));
    CHECK_THAT(sin(degrees{360}), WithinAbs(0, epsilon<double>));
}

TEST_CASE("cos", "[functions]") {
    const auto num = g.generate<double>();
    CHECK_THAT(cos(radians{num}), WithinAbs(std::cos(num), epsilon<double>));
}

TEST_CASE("cos specific radian values", "[functions]") {
    CHECK_THAT(cos(radians{0}), WithinAbs(1, epsilon<double>));
    CHECK_THAT(cos(radians{pi / 4}), WithinAbs(sqrt2 / 2, epsilon<double>));
    CHECK_THAT(cos(radians{pi / 2}), WithinAbs(0, epsilon<double>));
    CHECK_THAT(cos(radians{3 * pi / 4}), WithinAbs(-sqrt2 / 2, epsilon<double>));
    CHECK_THAT(cos(radians{pi}), WithinAbs(-1, epsilon<double>));
    CHECK_THAT(cos(radians{5 * pi / 4}), WithinAbs(-sqrt2 / 2, epsilon<double>));
    CHECK_THAT(cos(radians{3 * pi / 2}), WithinAbs(0, epsilon<double>));
    CHECK_THAT(cos(radians{7 * pi / 4}), WithinAbs(sqrt2 / 2, epsilon<double>));
    CHECK_THAT(cos(radians{2 * pi}), WithinAbs(1, epsilon<double>));
}

TEST_CASE("cos specific degree values", "[functions]") {
    CHECK_THAT(cos(degrees{0}), WithinAbs(1, epsilon<double>));
    CHECK_THAT(cos(degrees{45}), WithinAbs(sqrt2 / 2, epsilon<double>));
    CHECK_THAT(cos(degrees{90}), WithinAbs(0, epsilon<double>));
    CHECK_THAT(cos(degrees{135}), WithinAbs(-sqrt2 / 2, epsilon<double>));
    CHECK_THAT(cos(degrees{180}), WithinAbs(-1, epsilon<double>));
    CHECK_THAT(cos(degrees{225}), WithinAbs(-sqrt2 / 2, epsilon<double>));
    CHECK_THAT(cos(degrees{270}), WithinAbs(0, epsilon<double>));
    CHECK_THAT(cos(degrees{315}), WithinAbs(sqrt2 / 2, epsilon<double>));
    CHECK_THAT(cos(degrees{360}), WithinAbs(1, epsilon<double>));
}

TEST_CASE("ceil fractional", "[functions]") {
    const auto num = g.generate<double>();
    if (num > 0) {
        CHECK(ceil(num) == ((int)num) + 1);
    } else {
        CHECK(ceil(num) == ((int)num));
    }
}

TEST_CASE("ceil integer", "[functions]") {
    const auto num = (double)g.generate<int>();
    CHECK(ceil(num) == (int)num);
}

TEMPLATE_TEST_CASE("inverse_sqrt", "[functions]",
                   double, float)
{
    using T = TestType;
    const auto num = std::abs(g.generate<T>());
    CHECK_THAT(inverse_sqrt(num), WithinAbs(1 / std::sqrt(num), epsilon<T>));
}

} // namespace
} // namespace pstack::geo
