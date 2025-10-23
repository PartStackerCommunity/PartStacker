#ifndef PSTACK_GEO_MATRIX3_HPP
#define PSTACK_GEO_MATRIX3_HPP

#include "pstack/geo/functions.hpp"
#include "pstack/geo/vector3.hpp"

namespace pstack::geo {

template <class T>
struct matrix3 {
    T xx, xy, xz;
    T yx, yy, yz;
    T zx, zy, zz;
    friend constexpr bool operator==(const matrix3&, const matrix3&) = default;
};

template <class T>
inline constexpr matrix3<T> eye3 = { 1,  0,  0,
                                     0,  1,  0,
                                     0,  0,  1 };

template <class T>
constexpr matrix3<T> operator+(const matrix3<T>& lhs, const matrix3<T>& rhs) {
    return { lhs.xx + rhs.xx, lhs.xy + rhs.xy, lhs.xz + rhs.xz,
             lhs.yx + rhs.yx, lhs.yy + rhs.yy, lhs.yz + rhs.yz,
             lhs.zx + rhs.zx, lhs.zy + rhs.zy, lhs.zz + rhs.zz };
}

template <class T>
constexpr matrix3<T> operator*(const matrix3<T>& lhs, const matrix3<T>& rhs) {
    matrix3<T> m;
    m.xx = (lhs.xx * rhs.xx) + (lhs.xy * rhs.yx) + (lhs.xz * rhs.zx);
    m.xy = (lhs.xx * rhs.xy) + (lhs.xy * rhs.yy) + (lhs.xz * rhs.zy);
    m.xz = (lhs.xx * rhs.xz) + (lhs.xy * rhs.yz) + (lhs.xz * rhs.zz);
    m.yx = (lhs.yx * rhs.xx) + (lhs.yy * rhs.yx) + (lhs.yz * rhs.zx);
    m.yy = (lhs.yx * rhs.xy) + (lhs.yy * rhs.yy) + (lhs.yz * rhs.zy);
    m.yz = (lhs.yx * rhs.xz) + (lhs.yy * rhs.yz) + (lhs.yz * rhs.zz);
    m.zx = (lhs.zx * rhs.xx) + (lhs.zy * rhs.yx) + (lhs.zz * rhs.zx);
    m.zy = (lhs.zx * rhs.xy) + (lhs.zy * rhs.yy) + (lhs.zz * rhs.zy);
    m.zz = (lhs.zx * rhs.xz) + (lhs.zy * rhs.yz) + (lhs.zz * rhs.zz);
    return m;
}

template <class T>
constexpr vector3<T> operator*(const matrix3<T>& lhs, const vector3<T>& rhs) {
    vector3<T> v;
    v.x = (lhs.xx * rhs.x) + (lhs.xy * rhs.y) + (lhs.xz * rhs.z);
    v.y = (lhs.yx * rhs.x) + (lhs.yy * rhs.y) + (lhs.yz * rhs.z);
    v.z = (lhs.zx * rhs.x) + (lhs.zy * rhs.y) + (lhs.zz * rhs.z);
    return v;
}

template <class T>
constexpr matrix3<T> operator*(const matrix3<T>& lhs, const std::type_identity_t<T>& rhs) {
    return { lhs.xx * rhs, lhs.xy * rhs, lhs.xz * rhs,
             lhs.yx * rhs, lhs.yy * rhs, lhs.yz * rhs,
             lhs.zx * rhs, lhs.zy * rhs, lhs.zz * rhs };
}

template <class T>
constexpr matrix3<T> operator*(const std::type_identity_t<T>& lhs, const matrix3<T>& rhs) {
    return { lhs * rhs.xx, lhs * rhs.xy, lhs * rhs.xz,
             lhs * rhs.yx, lhs * rhs.yy, lhs * rhs.yz,
             lhs * rhs.zx, lhs * rhs.zy, lhs * rhs.zz };
}

template <class T>
constexpr matrix3<T> rot3(const vector3<T>& axis, const radians theta) {
    const T c = static_cast<T>(cos(theta));
    const T s = static_cast<T>(sin(theta));
    const auto n = normalize(axis);
    const matrix3<T> K = {    0, -n.z,  n.y,
                            n.z,    0, -n.x,
                           -n.y,  n.x,    0 };
    // Rodrigues's rotation formula: R = I + sin(t)*K + (1 - cos(t))*K^2
    return eye3<T> + (s * K) + ((1 - c) * K * K);
}

template <class T>
constexpr matrix3<T> rot3_x(const radians theta) {
    const T c = static_cast<T>(cos(theta));
    const T s = static_cast<T>(sin(theta));
    return { 1,  0,  0,
             0,  c, -s,
             0,  s,  c };
}

template <class T>
constexpr matrix3<T> rot3_y(const radians theta) {
    const T c = static_cast<T>(cos(theta));
    const T s = static_cast<T>(sin(theta));
    return { c,  0,  s,
             0,  1,  0,
            -s,  0,  c };
}

template <class T>
constexpr matrix3<T> rot3_z(const radians theta) {
    const T c = static_cast<T>(cos(theta));
    const T s = static_cast<T>(sin(theta));
    return { c, -s,  0,
             s,  c,  0,
             0,  0,  1 };
}

template <class T>
constexpr matrix3<T> scale3(const T x, const T y, const T z) {
    return { x,  0,  0,
             0,  y,  0,
             0,  0,  z };
}

} // namespace pstack::geo

#endif // PSTACK_GEO_MATRIX3_HPP
