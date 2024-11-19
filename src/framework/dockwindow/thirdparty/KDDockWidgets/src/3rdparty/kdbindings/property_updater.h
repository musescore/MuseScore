/*
  This file is part of KDBindings.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <functional>

namespace KDBindings {

/**
 * @brief A PropertyUpdater defines the interface used to update a Property, e.g. from a binding expression.
 *
 * An instance of this class (wrapped in a std::unique_ptr) can be passed to the Property constructor.
 * The Property will then become read-only, meaning an instance of ReadOnlyProperty will be thrown if the
 * Property's value is updated through any other means than through the PropertyUpdater.
 *
 * The Property constructor will pass a function to setUpdateFunction() for this purpose.
 * This function is then the only way to update the Property without encountering a ReadOnlyProperty error.
 *
 * The most typical use of PropertyUpdater is in instances of Binding, which are created by makeBoundProperty().
 */
template<typename T>
class PropertyUpdater
{
public:
    /** A PropertyUpdater can be default constructed. */
    PropertyUpdater() = default;

    /** A PropertyUpdater has a virtual destructor. */
    virtual ~PropertyUpdater() = default;

    /** A PropertyUpdater can be copy constructed. */
    PropertyUpdater(PropertyUpdater const &other) = default;
    /** A PropertyUpdater can be copy assigned. */
    PropertyUpdater &operator=(PropertyUpdater const &other) = default;

    /** A PropertyUpdater can be move constructed. */
    PropertyUpdater(PropertyUpdater &&other) = default;
    /** A PropertyUpdater can be move assigned. */
    PropertyUpdater &operator=(PropertyUpdater &&other) = default;

    /**
     * The Property will call this function when it constructed and pass a std::function as argument that allows
     * the PropertyUpdater to update the Property value.
     *
     * A PropertyUpdater typically saves this function and calls it once the value it computes changes.
     */
    virtual void setUpdateFunction(std::function<void(T &&)> const &updateFunction) = 0;

    /**
     * The get() function must return the current value the PropertyUpdater wants to assign to the Property.
     *
     * It is called from the Property constructor.
     */
    virtual T get() const = 0;
};

} // namespace KDBindings
