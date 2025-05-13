#ifndef PSTACK_GEO_VECTOR3_HPP
#define PSTACK_GEO_VECTOR3_HPP

#include "pstack/geo/functions.hpp"
#include <type_traits>

namespace pstack::geo {

template <class T>
struct vector3 {
    T x;
    T y;
    T z;
    friend constexpr bool operator==(const vector3&, const vector3&) = default;
};

template <class T>
inline constexpr vector3<T> unit_x = { 1, 0, 0 };

template <class T>
inline constexpr vector3<T> unit_y = { 0, 1, 0 };

template <class T>
inline constexpr vector3<T> unit_z = { 0, 0, 1 };

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
constexpr vector3<T> operator/(const std::type_identity_t<T>& lhs, const vector3<T>& rhs) {
    return { lhs / rhs.x, lhs / rhs.y, lhs / rhs.z };
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
