/*
  This file is part of KDBindings.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <kdbindings/property_updater.h>
#include <kdbindings/signal.h>

#include <iostream>
#include <memory>
#include <type_traits>

namespace KDBindings {

/**
 * @brief A namespace containing parts of KDBindings that are not part of the public API.
 *
 * The contents of this namespace may only be accessed by the implementation of KDBindings, they
 * are not part of KDBindings public API and may be altered at any time and provide no guarantees
 * of any kind when used directly.
 **/
namespace Private {

template<typename X, typename Y, typename = void>
struct are_equality_comparable : std::false_type {
};

template<typename X, typename Y>
struct are_equality_comparable<X, Y,
                               std::enable_if_t<
                                       std::is_same<
                                               std::decay_t<
                                                       decltype(std::equal_to<>{}(std::declval<X>(), std::declval<Y>()))>,
                                               bool>::value>> : std::true_type {
};

template<typename X, typename Y>
constexpr bool are_equality_comparable_v = are_equality_comparable<X, Y>::value;

} // namespace Private

/**
 * A ReadOnlyProperty is thrown when trying to set the value of a Property
 * that has a PropertyUpdater associated with it.
 *
 * Most commonly because the property holds the result of a binding expression.
 */
struct ReadOnlyProperty : std::runtime_error {
    ReadOnlyProperty() = delete;

    using std::runtime_error::runtime_error;
};

/**
 * @brief An instance of the KDBindings::equal_to struct is used to decide whether
 * two values of type T are equal in the context of data binding.
 *
 * If a new value is assigned to a Property and the existing value is equal_to
 * the existing value, the Property will not emit the Property::valueChanged or
 * Property::valueAboutToChange signals and not change the stored value.
 *
 * By default, all classes T that are equality comparable using std::equal_to
 * delegate to std::equal_to for equality comparison. All other instances are
 * assumed to never be equal.
 * Therefore, to change the equality behavior of a Property<T>, either:
 * - Implement operator== for T (std::equal_to uses operator== for equality comparison)
 * - Provide a template spezialization of KDBindings::equal_to and implement operator()()
 */
template<typename T>
struct equal_to {
    /**
     * This implementation of operator()() is only enabled if std::equal_to can be
     * used to compare values of type T.
     * In this case, std::equal_to is used to decide whether values of type T are equal.
     *
     * @return bool - Whether the values are equal.
     */
    auto operator()(const T &x, const T &y) const noexcept
            -> std::enable_if_t<Private::are_equality_comparable_v<T, T>, bool>
    {
        return std::equal_to<>{}(x, y);
    }

    /**
     * The fallback implementation of operator()() if the types are not equality comparable
     * using std::equal_to (i.e. no operator== implementation exists for this type).
     * In this case, two values of type T are assumed to never be equal.
     *
     * @return bool - Whether the values are equal - always false for this default implementation
     */
    template<typename X, typename Y>
    auto operator()(const X &, const Y &) const noexcept
            -> std::enable_if_t<!Private::are_equality_comparable_v<X, Y>, bool>
    {
        return false;
    }
};

// This forwrad declaration is required so that
// Property can declare PropertyNode as a friend
// class.
namespace Private {
    template<typename PropertyType>
    class PropertyNode;
}

/**
 * @brief A property represents a value that can be part of or the result of data binding.
 *
 * Properties are at the basis of data binding.
 * They can contain a value of any type T.
 * The value can either represent the result of a data binding or a value that is used
 * in the calculation of a binding expression.
 *
 * If the value of a property is changed, either manually or because it is the result of a
 * binding expression, the Property will emit the valueAboutToChange(), and valueChanged() Signal.
 * If it is used as part of a binding expression, the expression will be marked
 * as dirty and (unless a custom BindingEvaluator is used) updated immediately.
 *
 * To create a property from a data binding expression, use the @ref makeBoundProperty or @ref makeBinding
 * functions in the @ref KDBindings namespace.
 *
 * Examples:
 * - @ref 04-simple-property/main.cpp
 * - @ref 05-property-bindings/main.cpp
 * - @ref 06-lazy-property-bindings/main.cpp
 */
template<typename T>
class Property
{
public:
    typedef T valuetype;

    /**
     * Properties are default constructable.
     *
     * The value of a default constructed property is then also default constructed.
     */
    Property() = default;

    /**
     * If a Property is destroyed, it emits the destroyed() Signal.
     */
    ~Property()
    {
        m_destroyed.emit();
    }

    /**
     * Constructs a Property from the provided value.
     */
    explicit Property(T value) noexcept(std::is_nothrow_move_constructible<T>::value)
        : m_value{ std::move(value) }
    {
    }

    /**
     * Properties are not copyable.
     */
    Property(Property<T> const &other) = delete;
    Property &operator=(Property<T> const &other) = delete;

    /**
     * @brief Properties are movable.
     *
     * All connections that were made to the signals of the original Property
     * are moved over to the newly-constructed Property.
     *
     * All data bindings that depend on the moved-from Property will update their references
     * to the newly move-constructed Property.
     */
    Property(Property<T> &&other) noexcept(std::is_nothrow_move_constructible<T>::value)
        : m_value(std::move(other.m_value))
        , m_valueAboutToChange(std::move(other.m_valueAboutToChange))
        , m_valueChanged(std::move(other.m_valueChanged))
        , m_destroyed(std::move(other.m_destroyed))
        , m_updater(std::move(other.m_updater))
    {
        // We do not move the m_moved signal yet so that objects interested in the moved-into
        // property can recreate any connections they need.

        // If we have an updater, let it know how to update our internal value
        if (m_updater) {
            using namespace std::placeholders;
            m_updater->setUpdateFunction(
                    std::bind(&Property<T>::setHelper, this, _1));
        }

        // Emit the moved signals for the moved from and moved to properties
        m_moved.emit(*this);
        other.m_moved.emit(*this);
        m_moved = std::move(other.m_moved);
    }

    /**
     * See: Property(Property<T> &&other)
     */
    Property &operator=(Property<T> &&other) noexcept(std::is_nothrow_move_assignable<T>::value)
    {
        // We do not move the m_moved signal yet so that objects interested in the moved-into
        // property can recreate any connections they need.
        m_value = std::move(other.m_value);
        m_valueAboutToChange = std::move(other.m_valueAboutToChange);
        m_valueChanged = std::move(other.m_valueChanged);
        m_destroyed = std::move(other.m_destroyed);
        m_updater = std::move(other.m_updater);

        // If we have an updater, let it know how to update our internal value
        if (m_updater) {
            using namespace std::placeholders;
            m_updater->setUpdateFunction(
                    std::bind(&Property<T>::setHelper, this, _1));
        }

        // Emit the moved signals for the moved from and moved to properties
        m_moved.emit(*this);
        other.m_moved.emit(*this);
        m_moved = std::move(other.m_moved);

        return *this;
    }

    /**
     * Construct a property that will be updated by the specified PropertyUpdater.
     *
     * This constructor is usually called by the creation of a data binding
     * and usually doesn't need to be called manually.
     */
    template<typename UpdaterT>
    explicit Property(std::unique_ptr<UpdaterT> &&updater)
    {
        *this = std::move(updater);
    }

    /**
     * Assigns a Binding or other Updater to this Property.
     *
     * In comparison to the move assignment operator, this does NOT change any
     * of the existing Signal connections. They are all kept as-is.
     * Only the source of the update is changed.
     *
     * This will immediately set the value of this Property to the
     * result of the updater and will call the valueAboutToChange or valueChanged
     * Signals respectively if necessary.
     */
    template<typename UpdaterT>
    Property &operator=(std::unique_ptr<UpdaterT> &&updater)
    {
        m_updater = std::move(updater);

        // Let the updater know how to update our internal value
        using namespace std::placeholders;
        m_updater->setUpdateFunction(
                std::bind(&Property<T>::setHelper, this, _1));

        // Now synchronise our value with whatever the updator has right now.
        setHelper(m_updater->get());

        return *this;
    }

    /**
     * @brief Disconnects the binding from this Property
     *
     * If this Property has a binding, it will no longer update it.
     * Otherwise, this function does nothing.
     *
     * The value of the property does not change when it is reset.
     */
    void reset()
    {
        m_updater.reset();
    }

    /**
     * Returns a Signal that will be emitted before the value is changed.
     *
     * The first emitted value is the current value of the Property.<br>
     * The second emitted value is the new value of the Property.
     */
    Signal<const T &, const T &> &valueAboutToChange() const { return m_valueAboutToChange; }

    /**
     * Returns a Signal that will be emitted after the value of the property changed.
     *
     * The emitted value is the current (new) value of the Property.
     */
    Signal<const T &> &valueChanged() const { return m_valueChanged; }

    /**
     * Returns a Signal that will be emitted when this Property is destructed.
     */
    Signal<> &destroyed() const { return m_destroyed; }

    /**
     * Assign a new value to this Property.
     *
     * If the new value is equal_to the existing value, the value will not be
     * changed and no Signal will be emitted.
     *
     * Otherwise, the valueAboutToChange() Signal will be emitted before the value
     * of the Property is changed.
     * Then, the provided value will be assigned, and the valueChanged() Signal
     * will be emitted.
     *
     * @throw ReadOnlyProperty If the Property has a PropertyUpdater associated with it (i.e. it is
     * the result of a binding expression).
     */
    void set(T value)
    {
        if (m_updater) {
            throw ReadOnlyProperty{
                "Cannot set value on a read-only property. This property likely holds the result of a binding expression."
            };
        }
        setHelper(std::move(value));
    }

    /**
     * Returns the value represented by this Property.
     */
    T const &get() const
    {
        return m_value;
    }

    /**
     * Assigns a new value to this Property.
     *
     * See: set().
     */
    Property<T> &operator=(T const &rhs)
    {
        set(std::move(rhs));
        return *this;
    }

    /**
     * Returns the value represented by this Property.
     *
     * See: get().
     */
    T const &operator()() const
    {
        return Property<T>::get();
    }

private:
    void setHelper(T value)
    {
        if (equal_to<T>{}(value, m_value))
            return;

        m_valueAboutToChange.emit(m_value, value);
        m_value = std::move(value);
        m_valueChanged.emit(m_value);
    }

    T m_value;
    // the signals in a property are mutable, as a property
    // being "const" should mean that it's value or binding does
    // not change, not that nobody can listen to it anymore.
    mutable Signal<const T &, const T &> m_valueAboutToChange;
    mutable Signal<const T &> m_valueChanged; // By const ref so we can emit the signal for move-only types of T e.g. std::unique_ptr<int>

    // The PropertyNode needs to be a friend class of the Property, as it needs
    // access to the m_moved Signal.
    // The decision to make this Signal private was made after the suggestion by
    // @jm4R who reported issues with the move constructors noexcept guarantee.
    // (https://github.com/KDAB/KDBindings/issues/24)
    // Ideally we would like to figure out a way to remove the moved signal entirely
    // at some point. However currently it is still needed for Property bindings to
    // keep track of moved Properties.
    template<typename PropertyType>
    friend class Private::PropertyNode;
    Signal<Property<T> &> m_moved;

    mutable Signal<> m_destroyed;
    std::unique_ptr<PropertyUpdater<T>> m_updater;
};

/**
 * Outputs the value of the Property onto an output stream.
 */
template<typename T>
std::ostream &operator<<(std::ostream &stream, Property<T> const &property)
{
    stream << property.get();
    return stream;
}

/**
 * Reads a value of type T from the input stream and assigns it to
 * the Property using set().
 */
template<typename T>
std::istream &operator>>(std::istream &stream, Property<T> &prop)
{
    T temp;
    stream >> temp;
    prop.set(std::move(temp));
    return stream;
}

namespace Private {

template<typename T>
struct is_property_helper : std::false_type {
};

template<typename T>
struct is_property_helper<Property<T>> : std::true_type {
};

template<typename T>
struct is_property : is_property_helper<std::decay_t<T>> {
};

} // namespace Private

/**
 * @example 04-simple-property/main.cpp
 *
 * An example of how to create a KDBindings::Property and use its valueChanged() KDBindings::Signal to receive notifications whenever the value of the KDBindigns::Property changes.
 *
 * The output of this example is:
 * ```
 * The new value is 42
 * The new value is 69
 * Property value is 69
 * ```
 */

/**
 * @example 05-property-bindings/main.cpp
 *
 * An example of how to use makeBoundProperty() to create a KDBindings::Property that is automatically updated once any of its inputs change.
 *
 * The output of this example is:
 * ```
 * The initial size of the image = 1920000 bytes
 * The new size of the image = 4608000 bytes
 * The new size of the image = 8294400 bytes
 * ```
 */

} // namespace KDBindings
