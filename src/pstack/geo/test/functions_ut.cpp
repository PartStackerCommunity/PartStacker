#include "pstack/geo/functions.hpp"
#include "pstack/geo/test/generate.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

namespace pstack::geo {
namespace {

constexpr test::generator<> g{};

TEST_CASE("pi", "[functions]") {
    STATIC_CHECK(pi == std::numbers::pi);
}

TEST_CASE("sin", "[functions]") {
    const auto num = g.generate<double>();
    CHECK(sin(num) == Catch::Approx(std::sin(num)));
}

TEST_CASE("cos", "[functions]") {
    const auto num = g.generate<double>();
    CHECK(cos(num) == Catch::Approx(std::cos(num)));
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
    CHECK(inverse_sqrt(num) == Catch::Approx(1 / std::sqrt(num)));
}

} // namespace
} // namespace pstack::geo
