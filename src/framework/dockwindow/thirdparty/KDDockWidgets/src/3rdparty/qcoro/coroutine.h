// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <version>
#include <utility>

// __cpp_lib_coroutine is not defined if the compiler doesn't support coroutines
// (__cpp_impl_coroutine), e.g. clang as of 13.0.
#if defined(__cpp_lib_coroutine)
#include <coroutine>
#elif defined(__clang__)
// Implement our own <coroutine> header in a way that is compatible with the standard.
// See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4849.pdf

#include <type_traits> // void_t
#include <cstddef> // size_t

// Intrinsincs for Clang
// https://clang.llvm.org/docs/LanguageExtensions.html#c-coroutines-support-builtins
extern "C" {
void __builtin_coro_destroy(void *addr);
void __builtin_coro_resume(void *addr);
bool __builtin_coro_done(void *addr);
void* __builtin_coro_promise(void *addr, int alignment, bool from_promise);
void *__builtin_coro_noop();
}

// 17.12.1 Header <coroutine> synopsis
namespace std {

// 17.12.2, coroutine traits
// (omitted, because we implement them in std::experimental namespace and import them into the std
// namespace).
// template<class R, class .. ArgTypes>
// struct coroutine_traits;

// 17.12.3, coroutine traits
template<class Promise = void>
struct coroutine_handle;

// 17.12.3.6, comparison operators
constexpr bool operator==(coroutine_handle<> x, coroutine_handle<> y) noexcept;
// constexpr strong_ordering operator<=>(coroutine_handle<> x, coroutine_handle<> y) noexcept;

// 17.12.3.7, hash support
//template<class T> struct hash;
//template<class P> struct hash<coroutine_handle<P>>;

// 17.12.4, n-op- coroutines
struct noop_coroutine_promise;

template<>
struct coroutine_handle<noop_coroutine_promise>;
using noop_coroutine_handle = coroutine_handle<noop_coroutine_promise>;
noop_coroutine_handle noop_coroutine() noexcept;

// 17.12.5, trivial awaitables
struct suspend_never;
struct suspend_always;

} // namespace std


// Implementation
namespace std {

// Clang checks for std::experimental::coroutine_traits explicitly, so we must define the types
// in the experimental namespace.
namespace experimental {

template<class R, class = void>
struct __coroutine_traits_base {};

template<class R>
struct __coroutine_traits_base<R, void_t<typename R::promise_type>> {
    using promise_type = typename R::promise_type;
};


// 17.12.2, coroutine traits

template<class R, class ... ArgTypes>
struct coroutine_traits : __coroutine_traits_base<R> {};


// Clang requires that std::experimental::coroutine_handle is a class template
template<typename Promise>
struct coroutine_handle : public std::coroutine_handle<Promise> {};

} // namespace experimental

// Import std::experimental::coroutine_traits into the std namespace
template<typename R, typename ... ArgTypes>
using coroutine_traits = std::experimental::coroutine_traits<R, ArgTypes ...>;


// 17.12.3, coroutine handle

template<>
struct coroutine_handle<void> {
    // 17.12.3.1, construct/reset
    constexpr coroutine_handle() noexcept {}
    constexpr coroutine_handle(nullptr_t) noexcept {}
    coroutine_handle &operator=(nullptr_t) noexcept {
        m_ptr = nullptr;
        return *this;
    }

    // 17.12.3.2, export/import
    constexpr void *address() const noexcept {
        return m_ptr;
    }

    static constexpr coroutine_handle from_address(void *addr) noexcept {
        coroutine_handle handle;
        handle.m_ptr = addr;
        return handle;
    }

    // 17.12.3.3, observers
    constexpr explicit operator bool() const noexcept {
        return m_ptr != nullptr;
    }
    bool done() const {
        return __builtin_coro_done(m_ptr);
    }

    // 17.12.3.4, resumption
    void operator()() const {
        resume();
    }
    void resume() const {
        __builtin_coro_resume(m_ptr);
    }
    void destroy() const {
        __builtin_coro_destroy(m_ptr);
    }

protected:
    void *m_ptr = nullptr;
};

template<class Promise>
struct coroutine_handle : public coroutine_handle<> {
    // 17.12.3.1, construct, reset
    using coroutine_handle<>::coroutine_handle;
    static coroutine_handle from_promise(Promise &promise) {
        coroutine_handle handle;
        handle.m_ptr = __builtin_coro_promise(&promise, alignof(Promise), /* from-promise=*/ true);
        return handle;
    }
    coroutine_handle &operator=(nullptr_t) noexcept {
        this->m_ptr = nullptr;
        return *this;
    }

    // 17.12.3.2, export/import
    static constexpr coroutine_handle from_address(void *addr) noexcept {
        coroutine_handle handle;
        handle.m_ptr = addr;
        return handle;
    }

    //17.12.3.5, promise access
    Promise &promise() const {
        return *reinterpret_cast<Promise *>(
                __builtin_coro_promise(m_ptr, alignof(Promise), /*from-promise=*/false));
    }
};

// 17.12.3.6, comparison operators
constexpr bool operator==(coroutine_handle<> x, coroutine_handle<> y) noexcept {
    return x.address() == y.address();
}

//constexpr strong_ordering operator<=>(coroutine_handle<> x, coroutine_handle<> y) noexcept;

// 17.12.4, no-op coroutines
struct noop_coroutine_promise {};

template<>
struct coroutine_handle<noop_coroutine_promise>;
using noop_coroutine_handle = coroutine_handle<noop_coroutine_promise>;

template<>
struct coroutine_handle<noop_coroutine_promise> : public coroutine_handle<> {
    // 17.12.4.2.1, observers
    constexpr explicit operator bool() const noexcept { return true; }
    constexpr bool done() const noexcept { return false; }
    constexpr void operator()() const noexcept {}
    constexpr void resume() const noexcept {}
    constexpr void destroy() const noexcept {}

    noop_coroutine_promise &promise() const noexcept {
        return *reinterpret_cast<noop_coroutine_promise *>(
                    __builtin_coro_promise(__builtin_coro_noop(),
                                           alignof(noop_coroutine_promise), false));
    }

private:
    coroutine_handle() noexcept
        : coroutine_handle<>(from_address(__builtin_coro_noop())) {}

    friend noop_coroutine_handle noop_coroutine() noexcept;
};

inline noop_coroutine_handle noop_coroutine() noexcept {
    return {};
}

// 17.12.5, trivial awaitables

struct suspend_never {
    constexpr bool await_ready() const noexcept { return true; }
    constexpr void await_resume() const noexcept {}
    constexpr void await_suspend(coroutine_handle<>) const noexcept {}
};

struct suspend_always {
    constexpr bool await_ready() const noexcept { return false; }
    constexpr void await_suspend(coroutine_handle<>) const noexcept {}
    constexpr void await_resume() const noexcept {}
};

} // namespace std

#else // defined(__clang__)
#pragma error "Current compiler does not support coroutines, or is not supported by QCoro."
#endif // defined(__cpp_lib_coroutine)

// The QCORO_STD macro is no longer needed (with the code above), but keep it for backwards
// compatibility.
#ifdef QCORO_NO_DEPRECATED_QCOROSTD
#define QCORO_STD std
#else // QCORO_NO_DEPRECATED_QCOROSTD
#ifdef _MSC_VER
    #define _QCORO_STRINGIFY2(x) #x
    #define _QCORO_STRINGIFY(x) _QCORO_STRINGIFY2(x)
    #define QCORO_STD \
        __pragma(message(__FILE__ "(" _QCORO_STRINGIFY(__LINE__) ") QCORO_STD macro is deprecated, use regular 'std' namespace instead, or pass /DQCORO_NO_DEPRECATED_QCOROSTD to suppress this warning.")) \
        std
#else // GCC, clang
    #define QCORO_STD \
        _Pragma("GCC warning \"QCORO_STD macro is deprecated, use regular 'std' namespace instead, or pass -DQCORO_NO_DEPRECATED_QCOROSTD to suppress this warning.\"") \
        std
#endif // _MSC_VER
#endif // QCORO_NO_DEPRECATED_QCOROSTD

// Moc doesn't seem to understand something in the <concepts> header...
#ifndef Q_MOC_RUN

#include "concepts_p.h"

namespace QCoro {

namespace detail {


template<typename T>
concept has_await_methods = requires(T t) {
    { t.await_ready() } -> std::same_as<bool>;
    {t.await_suspend(std::declval<std::coroutine_handle<>>())};
    {t.await_resume()};
};

template<typename T>
concept has_member_operator_coawait = requires(T t) {
    // TODO: Check that result of co_await() satisfies Awaitable again
    { t.operator co_await() };
};

template<typename T>
concept has_nonmember_operator_coawait = requires(T t) {
    // TODO: Check that result of the operator satisfied Awaitable again
#if defined(_MSC_VER) && !defined(__clang__)
    // FIXME: MSVC is unable to perform ADL lookup for operator co_await and just fails to compile
    { ::operator co_await(static_cast<T &&>(t)) };
#else
    { operator co_await(static_cast<T &&>(t)) };
#endif
};

} // namespace detail

//! A concept describing the Awaitable type
/*!
 * Awaitable type is a type that can be passed as an argument to co_await.
 */
template<typename T>
concept Awaitable = detail::has_member_operator_coawait<T> ||
                    detail::has_nonmember_operator_coawait<T> ||
                    detail::has_await_methods<T>;

} // namespace QCoro

#endif // Q_MOC_RUN
