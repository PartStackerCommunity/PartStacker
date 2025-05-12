#ifndef PSTACK_GEO_VECTOR3_HPP
#define PSTACK_GEO_VECTOR3_HPP

#include "pstack/geo/functions.hpp"
#include <algorithm>
#include <type_traits>

namespace pstack::geo {

enum class Axis {
    X = 0,
    Y = 1,
    Z = 2
};

template <class T>
struct vector3 {
    T x;
    T y;
    T z;

    constexpr T& operator[](const Axis axis) {
        auto index = static_cast<std::underlying_type_t<Axis>>(axis);
        return *(&x + index);
    }

    constexpr const T& operator[](const Axis axis) const {
        auto index = static_cast<std::underlying_type_t<Axis>>(axis);
        return *(&x + index);
    }
};

template <class T>
inline constexpr vector3<T> unit_x = { 1, 0, 0 };

template <class T>
inline constexpr vector3<T> unit_y = { 0, 1, 0 };

template <class T>
inline constexpr vector3<T> unit_z = { 0, 0, 1 };

template <class T>
constexpr auto operator<=>(const vector3<T>& lhs, const vector3<T>& rhs) {
    if (auto c = lhs.x <=> rhs.x; c != 0) {
        return c;
    }
    if (auto c = lhs.y <=> rhs.y; c != 0) {
        return c;
    }
    return lhs.z <=> rhs.z;
}

template<class T>
constexpr vector3<T> component_min(const vector3<T>& lhs, const vector3<T>& rhs) {
    return { std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y), std::min(lhs.z, rhs.z) };
}

template<class T>
constexpr vector3<T> component_max(const vector3<T>& lhs, const vector3<T>& rhs) {
    return { std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y), std::max(lhs.z, rhs.z) };
}

template <class T>
constexpr vector3<T> sort(const vector3<T>& v) {
    vector3<T> result = v;
    if (result.x > result.z) std::swap(result.x, result.z);
    if (result.x > result.y) std::swap(result.x, result.y);
    if (result.y > result.z) std::swap(result.y, result.z);
    return result;
}

template <class T>
constexpr bool fits(const vector3<T>& a, const vector3<T>& b) {
    return a.x >= b.x && a.y >= b.y && a.z >= b.z;
}

template <class T>
constexpr vector3<T> operator+(const vector3<T>& lhs, const vector3<T>& rhs) {
    return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

template <class T>
constexpr vector3<T> operator-(const vector3<T>& lhs, const vector3<T>& rhs) {
    return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

template <class T>
constexpr vector3<T> operator-(const vector3<T>& lhs) {
    return { -lhs.x, -lhs.y, -lhs.z };
}

template <class T>
constexpr vector3<T>& operator+=(vector3<T>& lhs, const vector3<T>& rhs) {
    lhs = { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
    return lhs;
}

template <class T>
constexpr vector3<T> operator+(const vector3<T>& lhs, const std::type_identity_t<T>& rhs) {
    return { lhs.x + rhs, lhs.y + rhs, lhs.z + rhs };
}

template <class T>
constexpr vector3<T> operator+(const std::type_identity_t<T>& lhs, const vector3<T>& rhs) {
    return { lhs + rhs.x, lhs + rhs.y, lhs + rhs.z };
}

template <class T>
constexpr vector3<T> operator-(const vector3<T>& lhs, const std::type_identity_t<T>& rhs) {
    return { lhs.x - rhs, lhs.y - rhs, lhs.z - rhs };
}

template <class T>
constexpr vector3<T> operator-(const std::type_identity_t<T>& lhs, const vector3<T>& rhs) {
    return { lhs - rhs.x, lhs - rhs.y, lhs - rhs.z };
}

template <class T>
constexpr vector3<T> operator*(const std::type_identity_t<T>& lhs, const vector3<T>& rhs) {
    return { rhs.x * lhs, rhs.y * lhs, rhs.z * lhs };
}

template <class T>
constexpr vector3<T> operator*(const vector3<T>& lhs, const std::type_identity_t<T>& rhs) {
    return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
}

template <class T>
constexpr vector3<T> operator/(const vector3<T>& lhs, const std::type_identity_t<T>& rhs) {
    return { lhs.x / rhs, lhs.y / rhs, lhs.z / rhs };
}

template <class T>
constexpr T dot(const vector3<T>& lhs, const vector3<T>& rhs) {
    return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
}

template <class T>
constexpr vector3<T> cross(const vector3<T>& lhs, const vector3<T>& rhs) {
    return { lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x };
}

template <class T>
constexpr vector3<T> normalize(const vector3<T>& v) {
    const auto length_squared = (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
    return v * inverse_sqrt(length_squared);
}

} // namespace pstack::geo

#endif // PSTACK_GEO_VECTOR3_HPP
