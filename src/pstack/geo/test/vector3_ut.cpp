#include "pstack/geo/vector3.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>

namespace pstack::geo {
namespace {

template <class T>
std::pair<vector3<T>, vector3<T>> generate_vector_pair() {
    const auto gen = GENERATE(take(10, chunk(6, random(T{-1000}, T{1000}))));
    return {
        vector3<T>{ gen[0], gen[1], gen[2] },
        vector3<T>{ gen[3], gen[4], gen[5] }
    };
}

template <class T>
vector3<T> generate_vector() {
    const auto gen = GENERATE(take(10, chunk(3, random(T{-1000}, T{1000}))));
    return vector3<T>{ gen[0], gen[1], gen[2] };
}

template <class T>
std::pair<vector3<T>, T> generate_vector_scalar() {
    // Guard against div by zero
    static constexpr auto nonzero = [](T t) { return t != T{0}; };
    const auto gen = GENERATE(take(10, chunk(4, filter(nonzero, random(T{-1000}, T{1000})))));
    return {
        vector3<T>{ gen[0], gen[1], gen[2] },
        gen[3]
    };
}

TEMPLATE_TEST_CASE("properties", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    STATIC_CHECK(std::is_aggregate_v<vector3<T>>);
    STATIC_CHECK(sizeof(vector3<T>) == 3 * sizeof(T));
}

TEMPLATE_TEST_CASE("operator==()", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = generate_vector_pair<T>();
    CHECK(lhs == lhs);
    CHECK(rhs == rhs);
    CHECK(lhs != rhs);
    CHECK(rhs != lhs);
}

TEMPLATE_TEST_CASE("unit_x", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    STATIC_CHECK(std::is_same_v<decltype(unit_x<T>), const vector3<T>>);
    STATIC_CHECK(unit_x<T>.x == T{1});
    STATIC_CHECK(unit_x<T>.y == T{0});
    STATIC_CHECK(unit_x<T>.z == T{0});
}

TEMPLATE_TEST_CASE("unit_y", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    STATIC_CHECK(std::is_same_v<decltype(unit_y<T>), const vector3<T>>);
    STATIC_CHECK(unit_y<T>.x == T{0});
    STATIC_CHECK(unit_y<T>.y == T{1});
    STATIC_CHECK(unit_y<T>.z == T{0});
}

TEMPLATE_TEST_CASE("unit_z", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    STATIC_CHECK(std::is_same_v<decltype(unit_z<T>), const vector3<T>>);
    STATIC_CHECK(unit_z<T>.x == T{0});
    STATIC_CHECK(unit_z<T>.y == T{0});
    STATIC_CHECK(unit_z<T>.z == T{1});
}

TEMPLATE_TEST_CASE("two-vector operator+()", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = generate_vector_pair<T>();
    const vector3<T> expected{
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
    };
    CHECK(lhs + rhs == expected);
}

TEMPLATE_TEST_CASE("two-vector operator-()", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = generate_vector_pair<T>();
    const vector3<T> expected{
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z,
    };
    CHECK(lhs - rhs == expected);
}

TEMPLATE_TEST_CASE("unary operator-()", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto vec = generate_vector<T>();
    const vector3<T> expected{
        -vec.x,
        -vec.y,
        -vec.z,
    };
    CHECK(-vec == expected);
}

TEMPLATE_TEST_CASE("operator+=()", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    auto [lhs, rhs] = generate_vector_pair<T>();
    const vector3<T> expected = lhs + rhs;
    lhs += rhs;
    CHECK(lhs == expected);
}

TEMPLATE_TEST_CASE("vector-scalar operator+()", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = generate_vector_scalar<T>();
    const vector3<T> expected{
        lhs.x + rhs,
        lhs.y + rhs,
        lhs.z + rhs,
    };
    CHECK(lhs + rhs == expected);
    CHECK(rhs + lhs == expected);
}

TEMPLATE_TEST_CASE("vector-scalar operator-()", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = generate_vector_scalar<T>();
    const vector3<T> expected1{
        lhs.x - rhs,
        lhs.y - rhs,
        lhs.z - rhs,
    };
    CHECK(lhs - rhs == expected1);
    const vector3<T> expected2{
        rhs - lhs.x,
        rhs - lhs.y,
        rhs - lhs.z,
    };
    CHECK(rhs - lhs == expected2);
}

TEMPLATE_TEST_CASE("vector-scalar operator*()", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = generate_vector_scalar<T>();
    const vector3<T> expected{
        lhs.x * rhs,
        lhs.y * rhs,
        lhs.z * rhs,
    };
    CHECK(lhs * rhs == expected);
    CHECK(rhs * lhs == expected);
}

TEMPLATE_TEST_CASE("vector-scalar operator/()", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = generate_vector_scalar<T>();
    const vector3<T> expected1{
        lhs.x / rhs,
        lhs.y / rhs,
        lhs.z / rhs,
    };
    CHECK(lhs / rhs == expected1);
    const vector3<T> expected2{
        rhs / lhs.x,
        rhs / lhs.y,
        rhs / lhs.z,
    };
    CHECK(rhs / lhs == expected2);
}

TEMPLATE_TEST_CASE("dot()", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = generate_vector_pair<T>();
    const T expected =
        (lhs.x * rhs.x) +
        (lhs.y * rhs.y) +
        (lhs.z * rhs.z);
    CHECK(dot(lhs, rhs) == expected);
}

TEMPLATE_TEST_CASE("cross()", "[vector3]",
                   int, long, double, float)
{
    using T = TestType;
    const auto [lhs, rhs] = generate_vector_pair<T>();
    const vector3<T> expected{
        (lhs.y * rhs.z) - (lhs.z * rhs.y),
        (lhs.z * rhs.x) - (lhs.x * rhs.z),
        (lhs.x * rhs.y) - (lhs.y * rhs.x),
    };
    CHECK(cross(lhs, rhs) == expected);
}

TEMPLATE_TEST_CASE("normalize()", "[vector3]",
                   double, float)
{
    using T = TestType;
    const auto vec = generate_vector<T>();
    const auto expected = vec * inverse_sqrt(dot(vec, vec));
    const auto actual = normalize(vec);
    CHECK(actual == expected);
    CHECK(dot(actual, actual) == Catch::Approx(1));
}

} // namespace
} // namespace pstack::geo
