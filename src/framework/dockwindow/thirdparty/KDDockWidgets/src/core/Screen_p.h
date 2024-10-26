/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Waqar Ahmed <waqar.ahmed@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/QtCompat_p.h"

#include <memory>

namespace KDDockWidgets::Core {

/// @brief Represents a Screen
/// In Qt for example, this would be equivalent to QScreen.
class DOCKS_EXPORT Screen
{
public:
    using Ptr = std::shared_ptr<Screen>;
    using List = Vector<Ptr>;

    virtual ~Screen();

    virtual QString name() const = 0;

    /// @brief returns the size of screen in pixels
    virtual Size size() const = 0;

    /// @brief returns the geometry of screen in pixels
    virtual Rect geometry() const = 0;

    /// @brief returns the ratio between physical pixels and
    /// device-independent pixels for the screen.
    virtual double devicePixelRatio() const = 0;

    /// @brief returns the screen's available size in pixels
    /// The available size is the size excluding window manager
    /// reserved areas such as task bars and system menus.
    virtual Size availableSize() const = 0;

    /// @brief returns the screen's available geometry in pixels
    /// The available geometry is the geometry excluding window manager
    /// reserved areas such as task bars and system menus.
    virtual Rect availableGeometry() const = 0;

    /// @brief returns the pixel size of the virtual desktop corresponding to this screen.
    virtual Size virtualSize() const = 0;

    /// @brief returns the pixel geometry of the virtual desktop corresponding to this screen
    virtual Rect virtualGeometry() const = 0;

    /// @brief Returns whether the two Screen instances refer to the same underlying platform Screen
    virtual bool equals(std::shared_ptr<Screen> other) const = 0;

    bool operator==(Screen *) = delete;
    bool operator!=(Screen *) = delete;

    Screen() = default;
    Screen(const Screen &) = delete;
    Screen &operator=(const Screen &) = delete;
};


inline bool operator==(Screen::Ptr s1, Screen::Ptr s2)
{
    if (!s1 && !s2)
        return true;

    if (s1 && s2)
        return s1->equals(s2);

    return false;
}

inline bool operator!=(Screen::Ptr s1, Screen::Ptr s2)
{
    return !operator==(s1, s2);
}

}
