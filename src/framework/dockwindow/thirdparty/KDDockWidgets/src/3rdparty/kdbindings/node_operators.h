/*
  This file is part of KDBindings.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <kdbindings/node.h>
#include <kdbindings/make_node.h>

namespace KDBindings {

// Helper macro to declare free standing unary operators for Property and Node

#define KDBINDINGS_DEFINE_UNARY_OP(OP)                                                    \
    template<typename... T>                                                               \
    inline auto operator OP(Property<T...> &arg) noexcept(noexcept(OP arg.get()))         \
            ->Private::Node<std::decay_t<decltype(OP arg.get())>>                         \
    {                                                                                     \
        return Private::makeNode([](auto &&v) { return (OP v); }, arg);                   \
    }                                                                                     \
                                                                                          \
    template<typename T>                                                                  \
    inline auto operator OP(Private::Node<T> &&arg) noexcept(noexcept(OP arg.evaluate())) \
            ->Private::Node<std::decay_t<decltype(OP arg.evaluate())>>                    \
    {                                                                                     \
        return Private::makeNode([](auto &&v) { return (OP v); }, std::move(arg));        \
    }

KDBINDINGS_DEFINE_UNARY_OP(!)
KDBINDINGS_DEFINE_UNARY_OP(~) // Bitwise not
KDBINDINGS_DEFINE_UNARY_OP(+)
KDBINDINGS_DEFINE_UNARY_OP(-)

// Helper macro to declare free standing binary operators for Property and Node.
// The combinations we need are:
//
// operator op (Property<A> &a, B&& b)          [Property, value]
// operator op (A&& a, Property<B> &b)          [value, Property]
// operator op (Property<A> &a, Property<B> &b) [Property, Property]
//
// operator op (Node<A>&& a, B&& b)             [Node value]
// operator op (A&& a, Node<B>&& b)             [value, Node]
// operator op (Node<A>&& a, Node<B>&& b)       [Node, Node]
//
// operator op (Property<A> &a, Node<B>&& b)    [Property, Node]
// operaotr op (Node<A>&& a, Property<B> &b)    [Node, Property]

#define KDBINDINGS_DEFINE_BINARY_OP(OP)                                                                                  \
    template<typename B, typename... A>                                                                                  \
    inline auto operator OP(Property<A...> &a, B &&b) noexcept(noexcept(a.get() OP b))                                   \
            ->std::enable_if_t<!Private::is_bindable<B>::value,                                                          \
                               Private::Node<decltype(a.get() OP b)>>                                                    \
    {                                                                                                                    \
        return Private::makeNode([](auto &&av, auto &&bv) { return (av OP bv); }, a, std::forward<B>(b));                \
    }                                                                                                                    \
                                                                                                                         \
    template<typename A, typename... B>                                                                                  \
    inline auto operator OP(A &&a, Property<B...> &b) noexcept(noexcept(a OP b.get()))                                   \
            ->std::enable_if_t<!Private::is_bindable<A>::value,                                                          \
                               Private::Node<decltype(a OP b.get())>>                                                    \
    {                                                                                                                    \
        return Private::makeNode([](auto &&av, auto &&bv) { return (av OP bv); }, std::forward<A>(a), b);                \
    }                                                                                                                    \
                                                                                                                         \
    template<typename A, typename B>                                                                                     \
    inline auto operator OP(Property<A> &a, Property<B> &b) noexcept(noexcept(a.get() OP b.get()))                       \
            ->Private::Node<decltype(a.get() OP b.get())>                                                                \
    {                                                                                                                    \
        return Private::makeNode([](auto &&av, auto &&bv) { return (av OP bv); }, a, b);                                 \
    }                                                                                                                    \
                                                                                                                         \
    template<typename A, typename B>                                                                                     \
    inline auto operator OP(Private::Node<A> &&a, B &&b) noexcept(noexcept(a.evaluate() OP b))                           \
            ->std::enable_if_t<!Private::is_bindable<B>::value,                                                          \
                               Private::Node<decltype(a.evaluate() OP b)>>                                               \
    {                                                                                                                    \
        return Private::makeNode([](auto &&av, auto &&bv) { return (av OP bv); }, std::move(a), std::forward<B>(b));     \
    }                                                                                                                    \
                                                                                                                         \
    template<typename A, typename B>                                                                                     \
    inline auto operator OP(A &&a, Private::Node<B> &&b) noexcept(noexcept(a OP b.evaluate()))                           \
            ->std::enable_if_t<!Private::is_bindable<A>::value,                                                          \
                               Private::Node<decltype(a OP b.evaluate())>>                                               \
    {                                                                                                                    \
        return Private::makeNode([](auto &&av, auto &&bv) { return (av OP bv); }, std::forward<A>(a), std::move(b));     \
    }                                                                                                                    \
                                                                                                                         \
    template<typename A, typename B>                                                                                     \
    inline auto operator OP(Private::Node<A> &&a, Private::Node<B> &&b) noexcept(noexcept(a.evaluate() OP b.evaluate())) \
            ->Private::Node<decltype(a.evaluate() OP b.evaluate())>                                                      \
    {                                                                                                                    \
        return Private::makeNode([](auto &&av, auto &&bv) { return (av OP bv); }, std::move(a), std::move(b));           \
    }                                                                                                                    \
                                                                                                                         \
    template<typename B, typename A>                                                                                     \
    inline auto operator OP(Property<A> &a, Private::Node<B> &&b) noexcept(noexcept(a.get() OP b.evaluate()))            \
            ->Private::Node<decltype(a.get() OP b.evaluate())>                                                           \
    {                                                                                                                    \
        return Private::makeNode([](auto &&av, auto &&bv) { return (av OP bv); }, a, std::move(b));                      \
    }                                                                                                                    \
                                                                                                                         \
    template<typename A, typename B>                                                                                     \
    inline auto operator OP(Private::Node<A> &&a, Property<B> &b) noexcept(noexcept(a.evaluate() OP b.get()))            \
            ->Private::Node<decltype(a.evaluate() OP b.get())>                                                           \
    {                                                                                                                    \
        return Private::makeNode([](auto &&av, auto &&bv) { return (av OP bv); }, std::move(a), b);                      \
    }

KDBINDINGS_DEFINE_BINARY_OP(*)
KDBINDINGS_DEFINE_BINARY_OP(/)
KDBINDINGS_DEFINE_BINARY_OP(%)
KDBINDINGS_DEFINE_BINARY_OP(+)
KDBINDINGS_DEFINE_BINARY_OP(-)
KDBINDINGS_DEFINE_BINARY_OP(<<)
KDBINDINGS_DEFINE_BINARY_OP(>>)
KDBINDINGS_DEFINE_BINARY_OP(<)
KDBINDINGS_DEFINE_BINARY_OP(<=)
KDBINDINGS_DEFINE_BINARY_OP(>)
KDBINDINGS_DEFINE_BINARY_OP(>=)
KDBINDINGS_DEFINE_BINARY_OP(==)
KDBINDINGS_DEFINE_BINARY_OP(!=)
KDBINDINGS_DEFINE_BINARY_OP(&)
KDBINDINGS_DEFINE_BINARY_OP(^)
KDBINDINGS_DEFINE_BINARY_OP(|)
KDBINDINGS_DEFINE_BINARY_OP(&&)
KDBINDINGS_DEFINE_BINARY_OP(||)

} // namespace KDBindings
