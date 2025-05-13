#include "pstack/geo/triangle.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>

namespace pstack::geo {
namespace {

TEST_CASE("properties", "[triangle]") {
    STATIC_CHECK(std::is_aggregate_v<triangle>);
    STATIC_CHECK(std::is_trivially_copyable_v<triangle>);
    STATIC_CHECK(sizeof(triangle) == 4 * 3 * sizeof(float));

    triangle t{};
    STATIC_CHECK(std::is_same_v<decltype(t.normal), vector3<float>>);
    STATIC_CHECK(std::is_same_v<decltype(t.v1), point3<float>>);
    STATIC_CHECK(std::is_same_v<decltype(t.v2), point3<float>>);
    STATIC_CHECK(std::is_same_v<decltype(t.v3), point3<float>>);
}

} // namespace
} // namespace pstack::geo
