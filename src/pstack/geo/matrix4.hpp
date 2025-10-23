#ifndef PSTACK_GEO_MATRIX4_HPP
#define PSTACK_GEO_MATRIX4_HPP

#include "pstack/geo/functions.hpp"

namespace pstack::geo {

template <class T>
struct matrix4 {
    T xx, xy, xz, xw;
    T yx, yy, yz, yw;
    T zx, zy, zz, zw;
    T wx, wy, wz, ww;
    friend constexpr bool operator==(const matrix4&, const matrix4&) = default;
};

template <class T>
constexpr matrix4<T> operator*(const matrix4<T>& lhs, const matrix4<T>& rhs) {
    matrix4<T> m;
    m.xx = (lhs.xx * rhs.xx) + (lhs.xy * rhs.yx) + (lhs.xz * rhs.zx) + (lhs.xw * rhs.wx);
    m.xy = (lhs.xx * rhs.xy) + (lhs.xy * rhs.yy) + (lhs.xz * rhs.zy) + (lhs.xw * rhs.wy);
    m.xz = (lhs.xx * rhs.xz) + (lhs.xy * rhs.yz) + (lhs.xz * rhs.zz) + (lhs.xw * rhs.wz);
    m.xw = (lhs.xx * rhs.xw) + (lhs.xy * rhs.yw) + (lhs.xz * rhs.zw) + (lhs.xw * rhs.ww);

    m.yx = (lhs.yx * rhs.xx) + (lhs.yy * rhs.yx) + (lhs.yz * rhs.zx) + (lhs.yw * rhs.wx);
    m.yy = (lhs.yx * rhs.xy) + (lhs.yy * rhs.yy) + (lhs.yz * rhs.zy) + (lhs.yw * rhs.wy);
    m.yz = (lhs.yx * rhs.xz) + (lhs.yy * rhs.yz) + (lhs.yz * rhs.zz) + (lhs.yw * rhs.wz);
    m.yw = (lhs.yx * rhs.xw) + (lhs.yy * rhs.yw) + (lhs.yz * rhs.zw) + (lhs.yw * rhs.ww);

    m.zx = (lhs.zx * rhs.xx) + (lhs.zy * rhs.yx) + (lhs.zz * rhs.zx) + (lhs.zw * rhs.wx);
    m.zy = (lhs.zx * rhs.xy) + (lhs.zy * rhs.yy) + (lhs.zz * rhs.zy) + (lhs.zw * rhs.wy);
    m.zz = (lhs.zx * rhs.xz) + (lhs.zy * rhs.yz) + (lhs.zz * rhs.zz) + (lhs.zw * rhs.wz);
    m.zw = (lhs.zx * rhs.xw) + (lhs.zy * rhs.yw) + (lhs.zz * rhs.zw) + (lhs.zw * rhs.ww);

    m.wx = (lhs.wx * rhs.xx) + (lhs.wy * rhs.yx) + (lhs.wz * rhs.zx) + (lhs.ww * rhs.wx);
    m.wy = (lhs.wx * rhs.xy) + (lhs.wy * rhs.yy) + (lhs.wz * rhs.zy) + (lhs.ww * rhs.wy);
    m.wz = (lhs.wx * rhs.xz) + (lhs.wy * rhs.yz) + (lhs.wz * rhs.zz) + (lhs.ww * rhs.wz);
    m.ww = (lhs.wx * rhs.xw) + (lhs.wy * rhs.yw) + (lhs.wz * rhs.zw) + (lhs.ww * rhs.ww);
    return m;
}

template <class T>
inline constexpr matrix4<T> eye4 = { 1,  0,  0,  0,
                                     0,  1,  0,  0,
                                     0,  0,  1,  0,
                                     0,  0,  0,  1 };

template <class T>
constexpr matrix4<T> rot4_x(const radians theta) {
    const T c = static_cast<T>(cos(theta));
    const T s = static_cast<T>(sin(theta));
    return { 1,  0,  0,  0,
             0,  c, -s,  0,
             0,  s,  c,  0,
             0,  0,  0,  1 };
}

template <class T>
constexpr matrix4<T> rot4_y(const radians theta) {
    const T c = static_cast<T>(cos(theta));
    const T s = static_cast<T>(sin(theta));
    return { c,  0,  s,  0,
             0,  1,  0,  0,
            -s,  0,  c,  0,
             0,  0,  0,  1 };
}

template <class T>
constexpr matrix4<T> rot4_z(const radians theta) {
    const T c = static_cast<T>(cos(theta));
    const T s = static_cast<T>(sin(theta));
    return { c, -s,  0,  0,
             s,  c,  0,  0,
             0,  0,  1,  0,
             0,  0,  0,  1 };
}

template <class T>
constexpr matrix4<T> scale4(const T x, const T y, const T z) {
    return { x,  0,  0,  0,
             0,  y,  0,  0,
             0,  0,  z,  0,
             0,  0,  0,  1 };
}

template <class T>
constexpr matrix4<T> translate4(const T x, const T y, const T z) {
    return { 1,  0,  0,  x,
             0,  1,  0,  y,
             0,  0,  1,  z,
             0,  0,  0,  1 };
}

} // namespace pstack::geo

#endif // PSTACK_GEO_MATRIX4_HPP
