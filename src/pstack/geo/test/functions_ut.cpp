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

TEST_CASE("sin", "[functions]") {
    const auto num = g.generate<double>();
    CHECK_THAT(sin(num), WithinAbs(std::sin(num), epsilon<double>));
}

TEST_CASE("cos", "[functions]") {
    const auto num = g.generate<double>();
    CHECK_THAT(cos(num), WithinAbs(std::cos(num), epsilon<double>));
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
