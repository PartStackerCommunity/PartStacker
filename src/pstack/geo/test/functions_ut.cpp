#include "pstack/geo/functions.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>

namespace pstack::geo {
namespace {

template <class T>
T generate() {
    return GENERATE(take(10, random(T{-10}, T{10})));
}

TEST_CASE("pi", "[functions]") {
    STATIC_CHECK(pi == std::numbers::pi);
}

TEST_CASE("sin", "[functions]") {
    const auto num = generate<double>();
    CHECK(sin(num) == Catch::Approx(std::sin(num)));
}

TEST_CASE("cos", "[functions]") {
    const auto num = generate<double>();
    CHECK(cos(num) == Catch::Approx(std::cos(num)));
}

TEST_CASE("ceil fractional", "[functions]") {
    const auto num = generate<double>();
    if (num > 0) {
        CHECK(ceil(num) == ((int)num) + 1);
    } else {
        CHECK(ceil(num) == ((int)num));
    }
}

TEST_CASE("ceil integer", "[functions]") {
    const auto num = (double)generate<int>();
    CHECK(ceil(num) == (int)num);
}

TEMPLATE_TEST_CASE("inverse_sqrt", "[functions]",
                   double, float)
{
    const auto num = std::abs(generate<TestType>());
    CHECK(inverse_sqrt(num) == Catch::Approx(1 / std::sqrt(num)));
}

} // namespace
} // namespace pstack::geo
