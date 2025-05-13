#ifndef PSTACK_GEO_TEST_GENERATE_HPP
#define PSTACK_GEO_TEST_GENERATE_HPP

#include "pstack/geo/matrix3.hpp"
#include "pstack/geo/matrix4.hpp"
#include "pstack/geo/point3.hpp"
#include "pstack/geo/vector3.hpp"
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>

namespace pstack::geo::test {

template <class T>
struct generate_trait {
    using underlying = T;
    static constexpr std::size_t count = 1;
    static auto make(typename std::vector<T>::const_iterator& it) {
        return *it++;
    }
};
template <class T>
struct generate_trait<matrix3<T>> {
    using underlying = T;
    static constexpr std::size_t count = 9;
    static auto make(typename std::vector<T>::const_iterator& it) {
        matrix3<T> out;
        out.xx = *it++; out.xy = *it++; out.xz = *it++;
        out.yx = *it++; out.yy = *it++; out.yz = *it++;
        out.zx = *it++; out.zy = *it++; out.zz = *it++;
        return out;
    }
};
template <class T>
struct generate_trait<matrix4<T>> {
    using underlying = T;
    static constexpr std::size_t count = 16;
    static auto make(typename std::vector<T>::const_iterator& it) {
        matrix4<T> out;
        out.xx = *it++; out.xy = *it++; out.xz = *it++; out.xw = *it++;
        out.yx = *it++; out.yy = *it++; out.yz = *it++; out.yw = *it++;
        out.zx = *it++; out.zy = *it++; out.zz = *it++; out.zw = *it++;
        out.wx = *it++; out.wy = *it++; out.wz = *it++; out.ww = *it++;
        return out;
    }
};
template <class T>
struct generate_trait<point3<T>> {
    using underlying = T;
    static constexpr std::size_t count = 3;
    static auto make(typename std::vector<T>::const_iterator& it) {
        point3<T> out;
        out.x = *it++;
        out.y = *it++;
        out.z = *it++;
        return out;
    }
};
template <class T>
struct generate_trait<vector3<T>> {
    using underlying = T;
    static constexpr std::size_t count = 3;
    static auto make(typename std::vector<T>::const_iterator& it) {
        vector3<T> out;
        out.x = *it++;
        out.y = *it++;
        out.z = *it++;
        return out;
    }
};

template <class T>
using underlying = typename generate_trait<T>::underlying;
template <class T>
constexpr std::size_t count = generate_trait<T>::count;
template <class T>
auto make(typename std::vector<underlying<T>>::const_iterator& it) {
    return generate_trait<T>::make(it);
}

template <int Lower = -1000, int Upper = 1000, std::size_t Take = 10>
struct generator {
    template <class T, class... Ts, class ValueFilter = std::identity>
    requires (... and std::same_as<underlying<T>, underlying<Ts>>)
    static auto generate(ValueFilter = {}) ->
        std::conditional_t<sizeof...(Ts) == 0, T, std::tuple<T, Ts...>>
    {
        using U = underlying<T>;
        constexpr auto total = (count<T> + ... + count<Ts>);
        const auto nums = values<U, total, ValueFilter>();
        auto it = nums.begin();
        if constexpr (sizeof...(Ts) == 0) {
            return make<T>(it);
        } else {
            std::tuple<T, Ts...> out;
            [&]<std::size_t I, std::size_t... Is>(std::index_sequence<I, Is...>) {
                std::get<I>(out) = make<T>(it);
                (..., (std::get<Is>(out) = make<Ts>(it)));
            }(std::index_sequence_for<T, Ts...>{});
            return out;
        }
    }

private:
    template <class U, std::size_t Total, class ValueFilter>
    static auto values() {
        if constexpr (std::same_as<ValueFilter, std::identity>) {
            return GENERATE(take(Take, chunk(Total, random(U{Lower}, U{Upper}))));
        } else {
            static constexpr ValueFilter value_filter{};
            return GENERATE(take(Take, chunk(Total, filter(value_filter, random(U{Lower}, U{Upper})))));
        }
    }
};

constexpr auto nonzero_filter = []<class T>(T t) { return t != T{0}; };

} // namespace pstack::geo::test

#endif // PSTACK_GEO_TEST_GENERATE_HPP
