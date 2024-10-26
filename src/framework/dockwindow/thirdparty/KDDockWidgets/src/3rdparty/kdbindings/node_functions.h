/*
  This file is part of KDBindings.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <kdbindings/make_node.h>

#include <cmath>

namespace KDBindings {

namespace Private {

template<typename... Ts>
struct any_bindables;

// Check to see if a single type is a bindable (node or property)
template<typename T>
struct any_bindables<T> : is_bindable<T> {
};

// Check the head of the typelist and recurse
template<typename HEAD, typename... Ts>
struct any_bindables<HEAD, Ts...> : std::integral_constant<
                                            bool,
                                            any_bindables<HEAD>::value || any_bindables<Ts...>::value> {
};

} // namespace Private

/**
 * @brief KDBINDINGS_DECLARE_FUNCTION is a helper macro to declare and define functions for use in data binding.
 *
 * This macro can take any callable object or function reference and create a new function that may be used
 * in data binding expressions.
 * The result function that can be called with a Property or the result of a data binding expression
 * to create another data binding expression.
 *
 * Note that if a function is overloaded, it is impossible to reference all of its overloads at once.
 * Therefore we recommend declaring a struct with a templated operator() to use as the function object.
 * See the KDBindings::node_abs struct for an example of how to do this.
 *
 * @param NAME The name of the function to generate.
 * @param FUNC The function to wrap.
 */
#define KDBINDINGS_DECLARE_FUNCTION(NAME, FUNC)                                                                                                                                                       \
    template<typename... Ts>                                                                                                                                                                          \
    inline auto NAME(Ts &&...args)->std::enable_if_t<::KDBindings::Private::any_bindables<Ts...>::value, ::KDBindings::Private::Node<::KDBindings::Private::operator_node_result_t<decltype(FUNC), Ts...>>> \
    {                                                                                                                                                                                                 \
        return ::KDBindings::Private::makeNode(FUNC, std::forward<Ts>(args)...); \
    }

/**
 * @brief An example struct that is used with a call to KDBINDINGS_DECLARE_FUNCTION to declare all overloads
 * of std::abs as usable in data binding.
 *
 * Because of the way node_abs overloads its operator(), it can be used in a call to KDBINDINGS_DECLARE_FUNCTION like this:
 * @code
 * KDBINDINGS_DECLARE_FUNCTION(abs, node_abs{})
 * @endcode
 *
 * To generate such a struct for another function, use the KDBINDINGS_DECLARE_FUNCTION_OBJECT macro.
 */
struct node_abs {
    /**
     * @brief The operator() is overloaded so the struct can be used as a function object.
     *
     * Because this operator is templated, a single instance of node_abs
     * can refer to all overloads of std::abs.
     */
    template<typename... Ts>
    auto operator()(Ts &&...x) const
    {
        return std::abs(std::forward<Ts>(x)...);
    }
};
KDBINDINGS_DECLARE_FUNCTION(abs, node_abs{})

/**
 * @brief This macro declares a callable struct that wraps a function with all
 * its overloads.
 *
 * The declared struct can be used as the FUNCTION argument to
 * KDBINDINGS_DECLARE_FUNCTION(NAME, FUNCTION) to pass a function with
 * all its overloads to the macro.
 *
 * See the KDBindings::node_abs struct for an example of what this macro would generate.
 *
 * @param NAME The name of the resulting struct.
 * @param FUNCTION The function to wrap.
 */
#define KDBINDINGS_DECLARE_FUNCTION_OBJECT(NAME, FUNCTION) \
    struct NAME {                                          \
        template<typename... Ts>                           \
        auto operator()(Ts &&...x) const                   \
        {                                                  \
            return FUNCTION(std::forward<Ts>(x)...);       \
        }                                                  \
    };

/**
 * @brief This macro allows you to declare any function in a non-nested namespace
 * as available in the context of data binding.
 *
 * @param NAMESPACE the name of the namespace the function is in.
 * @param NAME the name of the function to wrap.
 *
 * In comparison to KDBINDINGS_DECLARE_FUNCTION(NAME, FUNC), this macro will generate a
 * helper struct using #KDBINDINGS_DECLARE_FUNCTION_OBJECT, so all overloads of the function are
 * made available at once.
 *
 * #KDBINDINGS_DECLARE_STD_FUNCTION is basically just a call to this macro with
 * the NAMESPACE parameter set to `std`.
 */
#define KDBINDINGS_DECLARE_NAMESPACED_FUNCTION(NAMESPACE, NAME)                  \
    KDBINDINGS_DECLARE_FUNCTION_OBJECT(node_##NAMESPACE_##NAME, NAMESPACE::NAME) \
    KDBINDINGS_DECLARE_FUNCTION(NAME, node_##NAMESPACE_##NAME{})

/**
 * @brief This macro is based on KDBINDINGS_DECLARE_NAMESPACED_FUNCTION(NAMESPACE, FUNC)
 * to make it easier to declare any standard library function as available for data binding.
 *
 * It uses #KDBINDINGS_DECLARE_NAMESPACED_FUNCTION and can therefore make all overloads
 * of the `std::` function available at once.
 *
 * @param NAME The name of the function in the `std::` namespace.
 */
#define KDBINDINGS_DECLARE_STD_FUNCTION(NAME) \
    KDBINDINGS_DECLARE_NAMESPACED_FUNCTION(std, NAME)

// Define some common and useful functions
KDBINDINGS_DECLARE_STD_FUNCTION(floor)
KDBINDINGS_DECLARE_STD_FUNCTION(ceil)
KDBINDINGS_DECLARE_STD_FUNCTION(sin)
KDBINDINGS_DECLARE_STD_FUNCTION(cos)
KDBINDINGS_DECLARE_STD_FUNCTION(tan)
KDBINDINGS_DECLARE_STD_FUNCTION(asin)
KDBINDINGS_DECLARE_STD_FUNCTION(acos)
KDBINDINGS_DECLARE_STD_FUNCTION(atan)

} // namespace KDBindings
