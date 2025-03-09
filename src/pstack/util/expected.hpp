#ifndef PSTACK_UTIL_EXPECTED_HPP
#define PSTACK_UTIL_EXPECTED_HPP

// If needed, this file contains a polyfill for the `std::expected`
// functionality used in PartStacker

#if defined(__cpp_lib_expected) and __cpp_lib_expected >= 202202L

#include <expected>

namespace pstack::util {

using std::expected;
using std::unexpected;

} // namespace pstack::util

#else // __cpp_lib_expected

#include <type_traits>
#include <utility>
#include <variant>

namespace pstack::util {

template <class E>
class unexpected {
public:
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) = default;
    
    template <class Err = E>
    requires (not std::is_same_v<std::remove_cvref_t<Err>, unexpected>)
        and (not std::is_same_v<std::remove_cvref_t<Err>, std::in_place_t>)
        and (std::is_constructible_v<E, Err>)
    constexpr explicit unexpected(Err&& e)
        : _e(std::forward<Err>(e))
    {}

    constexpr const E& error() const& noexcept {
        return _e;
    }
    constexpr E& error() & noexcept {
        return _e;
    }
    constexpr const E&& error() const&& noexcept {
        return std::move(_e);
    }
    constexpr E&& error() && noexcept {
        return std::move(_e);
    }

private:
    E _e;
};

template <class E>
unexpected(E) -> unexpected<E>;

template <class T, class E>
class expected {
public:
    constexpr expected() = default;
    constexpr expected(const expected&) = default;
    constexpr expected(expected&&)
        noexcept(std::is_nothrow_move_constructible_v<T> and std::is_nothrow_move_constructible_v<E>) = default;

    template <class U = std::remove_cv_t<T>>
    requires (not std::is_same_v<std::remove_cvref_t<U>, std::in_place_t>)
        and (not std::is_same_v<expected<T, E>, std::remove_cvref_t<U>>)
        and (std::is_constructible_v<T, U>)
    constexpr explicit(not std::is_convertible_v<U, T>)
        expected(U&& v)
        : _data(std::in_place_index<0>, std::forward<U>(v))
    {}

    template <class G>
    constexpr explicit(not std::is_convertible_v<const G&, E>)
        expected(const unexpected<G>& e)
        : _data(std::in_place_index<1>, std::forward<const G&>(e.error()))
    {}
    template <class G>
    constexpr explicit(not std::is_convertible_v<G, E>)
        expected(unexpected<G>&& e)
        : _data(std::in_place_index<1>, std::forward<G>(e.error()))
    {}

    constexpr explicit operator bool() const noexcept {
        return _data.index() == 0;
    }
    constexpr bool has_value() const noexcept {
        return _data.index() == 0;
    }

    constexpr const T* operator->() const noexcept {
        return std::addressof(std::get<0>(_data));
    }
    constexpr T* operator->() noexcept {
        return std::addressof(std::get<0>(_data));
    }
    constexpr const T& operator*() const& noexcept {
        return std::get<0>(_data);
    }
    constexpr T& operator*() & noexcept {
        return std::get<0>(_data);
    }
    constexpr const T&& operator*() const&& noexcept {
        return std::move(std::get<0>(_data));
    }
    constexpr T&& operator*() && noexcept {
        return std::move(std::get<0>(_data));
    }

    constexpr const E& error() const& noexcept {
        return std::get<1>(_data);
    }
    constexpr E& error() & noexcept {
        return std::get<1>(_data);
    }
    constexpr const E&& error() const&& noexcept {
        return std::get<1>(std::move(_data));
    }
    constexpr E&& error() && noexcept {
        return std::get<1>(std::move(_data));
    }

private:
    std::variant<T, E> _data;    
};

template <class T, class E>
requires std::is_void_v<T>
class expected<T, E> {
public:
    constexpr expected() = default;
    constexpr expected(const expected&) = default;
    constexpr expected(expected&&)
        noexcept(std::is_nothrow_move_constructible_v<E>) = default;

    template <class G>
    constexpr explicit(not std::is_convertible_v<const G&, E>)
        expected(const unexpected<G>& e)
        : _data(std::in_place_index<1>, std::forward<const G&>(e.error()))
    {}
    template <class G>
    constexpr explicit(not std::is_convertible_v<G, E>)
        expected(unexpected<G>&& e)
        : _data(std::in_place_index<1>, std::forward<G>(e.error()))
    {}

    constexpr explicit operator bool() const noexcept {
        return _data.index() == 0;
    }
    constexpr bool has_value() const noexcept {
        return _data.index() == 0;
    }

    constexpr const E& error() const& noexcept {
        return std::get<1>(_data);
    }
    constexpr E& error() & noexcept {
        return std::get<1>(_data);
    }
    constexpr const E&& error() const&& noexcept {
        return std::get<1>(std::move(_data));
    }
    constexpr E&& error() && noexcept {
        return std::get<1>(std::move(_data));
    }

private:
    std::variant<std::monostate, E> _data;
};

} // namespace pstack::util

#endif // __cpp_lib_expected

#endif // PSTACK_UTIL_EXPECTED_HPP
