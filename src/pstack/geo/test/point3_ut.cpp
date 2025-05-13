#include "pstack/geo/point3.hpp"
#include "pstack/geo/test/generate.hpp"
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

namespace pstack::geo {
namespace {

constexpr test::generator<> g{};

TEMPLATE_TEST_CASE("properties", "[point3]",
                   int, long, double, float)
{
    using T = TestType;
    STATIC_CHECK(std::is_aggregate_v<point3<T>>);
    STATIC_CHECK(sizeof(point3<T>) == 3 * sizeof(T));
}

TEMPLATE_TEST_CASE("operator==()", "[point3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = g.generate<point3<T>, point3<T>>();
    CHECK(lhs == lhs);
    CHECK(rhs == rhs);
    CHECK(lhs != rhs);
    CHECK(rhs != lhs);
}

TEMPLATE_TEST_CASE("as_vector()", "[point3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto point = g.generate<point3<T>>();
    const vector3<T> expected{ point.x, point.y, point.z };
    CHECK(point.as_vector() == expected);
}

TEMPLATE_TEST_CASE("origin3", "[point3]",
                   int, long, double, float)
{
    using T = TestType;
    STATIC_CHECK(std::is_same_v<decltype(origin3<T>), const point3<T>>);
    STATIC_CHECK(origin3<T>.x == T{0});
    STATIC_CHECK(origin3<T>.y == T{0});
    STATIC_CHECK(origin3<T>.z == T{0});
}

TEMPLATE_TEST_CASE("vector-point operator+()", "[point3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = g.generate<vector3<T>, point3<T>>();
    const point3<T> expected{
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
    };
    CHECK(lhs + rhs == expected);
    CHECK(rhs + lhs == expected);
}

TEMPLATE_TEST_CASE("two-point operator-()", "[point3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = g.generate<point3<T>, point3<T>>();
    const vector3<T> expected{
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z,
    };
    CHECK(lhs - rhs == expected);
}

TEMPLATE_TEST_CASE("operator+=()", "[point3]",
                   int, long, double, float)
{
    using T = TestType;
    auto [lhs, rhs] = g.generate<vector3<T>, point3<T>>();
    const point3<T> expected = rhs + lhs;
    rhs += lhs;
    CHECK(rhs == expected);
}

TEMPLATE_TEST_CASE("point-scalar operator+()", "[point3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = g.generate<point3<T>, T>();
    const point3<T> expected{
        lhs.x + rhs,
        lhs.y + rhs,
        lhs.z + rhs,
    };
    CHECK(lhs + rhs == expected);
    CHECK(rhs + lhs == expected);
}

TEMPLATE_TEST_CASE("point-scalar operator-()", "[point3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = g.generate<point3<T>, T>();
    const point3<T> expected1{
        lhs.x - rhs,
        lhs.y - rhs,
        lhs.z - rhs,
    };
    CHECK(lhs - rhs == expected1);
    const point3<T> expected2{
        rhs - lhs.x,
        rhs - lhs.y,
        rhs - lhs.z,
    };
    CHECK(rhs - lhs == expected2);
}

} // namespace
} // namespace pstack::geo
