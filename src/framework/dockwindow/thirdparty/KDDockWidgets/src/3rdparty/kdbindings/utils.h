/*
  This file is part of KDBindings.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Leon Matthes <leon.matthes@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#pragma once

#include <functional>
#include <type_traits>
#include <utility>

namespace KDBindings {

/**
* The contents of this namespace may only be accessed by the implementation of KDBindings, they
* are not part of KDBindings public API and may be altered at any time and provide no guarantees
* of any kind when used directly.
**/
namespace Private {

// ------------------------ get_arity --------------------------
// get_arity is a template function that returns the number of arguments
// of (almost) any callable object.
// The easiest way is to simply call get_arity<T>() for callable type T.
// It needs to be constexpr in order so it can be used in template arguments.

// To overload get_arity, it needs a marker type, as C++ doesn't allow partial
// function specialization.
template<typename T>
struct TypeMarker {
    constexpr TypeMarker() = default;
};

// base implementation of get_arity refers to specialized implementations for each
// type of callable object by using the overload for its specialized TypeMarker.
template<typename T>
constexpr size_t get_arity()
{
    return get_arity(TypeMarker<std::decay_t<T>>{});
}

// Syntactic sugar version of get_arity, allows to pass any callable object
// to get_arity, instead of having to pass its decltype as a template argument.
template<typename T>
constexpr size_t get_arity(const T &)
{
    return get_arity<T>();
}

// The arity of a function pointer is simply its number of arguments.
template<typename Return, typename... Arguments>
constexpr size_t get_arity(TypeMarker<Return (*)(Arguments...)>)
{
    return sizeof...(Arguments);
}

template<typename Return, typename... Arguments>
constexpr size_t get_arity(TypeMarker<Return (*)(Arguments...) noexcept>)
{
    return sizeof...(Arguments);
}

// The arity of a generic callable object is the arity of its operator() - 1, as the this
// pointer is already known for such an object.
template<typename T>
constexpr size_t get_arity(TypeMarker<T>)
{
    return get_arity(TypeMarker<decltype(&T::operator())>{}) - 1;
}

// Macro to help define most combinations of possible member function qualifiers.
// Add + 1 to sizeof...(Arguments) here as the "this" pointer is an implicit argument to any member function.
#define KDBINDINGS_DEFINE_MEMBER_GET_ARITY(MODIFIERS)                                                        \
    template<typename Return, typename Class, typename... Arguments>                                         \
    constexpr size_t get_arity(::KDBindings::Private::TypeMarker<Return (Class::*)(Arguments...) MODIFIERS>) \
    {                                                                                                        \
        return sizeof...(Arguments) + 1;                                                                     \
    }

// Define the get_arity version without modifiers without using the macro.
// MSVC otherwise complains about a call to the macro with too few arguments
template<typename Return, typename Class, typename... Arguments>
constexpr size_t get_arity(::KDBindings::Private::TypeMarker<Return (Class::*)(Arguments...)>)
{
    return sizeof...(Arguments) + 1;
}

KDBINDINGS_DEFINE_MEMBER_GET_ARITY(const)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(&)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(const &)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(&&)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(const &&)

KDBINDINGS_DEFINE_MEMBER_GET_ARITY(volatile)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(volatile const)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(volatile &)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(volatile const &)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(volatile &&)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(volatile const &&)

KDBINDINGS_DEFINE_MEMBER_GET_ARITY(noexcept)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(const noexcept)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(&noexcept)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(const &noexcept)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(&&noexcept)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(const &&noexcept)

KDBINDINGS_DEFINE_MEMBER_GET_ARITY(volatile noexcept)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(volatile const noexcept)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(volatile &noexcept)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(volatile const &noexcept)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(volatile &&noexcept)
KDBINDINGS_DEFINE_MEMBER_GET_ARITY(volatile const &&noexcept)

// -------------------- placeholder and bind_first ---------------------
// Inspired by https://gist.github.com/engelmarkus/fc1678adbed1b630584c90219f77eb48
// A placeholder provides a way to construct something equivalent to a std::placeholders::_N
// with N as a template argument.
//
// Note: As placeholders start at 1, therefore placeholder<0> is NOT a valid placeholder.
template<int>
struct placeholder {
};

template<typename Func, typename... Args, std::size_t... Is>
auto bind_first_helper(std::index_sequence<Is...>, Func &&fun, Args... args)
{
    return std::bind(std::forward<Func>(fun), std::forward<Args>(args)..., placeholder<Is + 1>{}...);
}

// bind_first binds the first arguments to the callable object (i.e. function) to the values provided by args.
// The return value is a new function taking get_arity<Func> - sizeof...(Args) many arguments, with the first
// sizeof...(Args) arguments bound to the values of args.
// This is different to a call with std::bind(fun, args...), as the callable object created by std::bind would
// in this case now take zero arguments, whilst bind_first still expects the remaining arguments to be provided
//
// For now, providing instances of std::placeholders in Args is not allowed, as the implications of this are
// unclear if sizeof...(Args) != get_arity<Func>. The enable_if_t makes sure none of the Args value is a placeholder.
//
// In the future, we could provide another overload of this function that allows placeholders, as long as all arguments
// are bound.
template<
        typename Func,
        typename... Args,
        /*Disallow any placeholder arguments, they would mess with the number and ordering of required and bound arguments, and are, for now, unsupported*/
        typename = std::enable_if_t<std::conjunction_v<std::negation<std::is_placeholder<Args>>...>>>
auto bind_first(Func &&fun, Args &&...args)
{
    return bind_first_helper(std::make_index_sequence<get_arity<Func>() - sizeof...(Args)>{}, std::forward<Func>(fun), std::forward<Args>(args)...);
}

} // namespace Private

} // namespace KDBindings

namespace std {

// This allows a placeholder to be used as a replacement of a std::placeholders.
template<int N>
struct is_placeholder<KDBindings::Private::placeholder<N>>
    : integral_constant<int, N> {
};

} // namespace std
