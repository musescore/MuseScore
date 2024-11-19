/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "core/View.h"
#include "kdbindings/signal.h"
#include "QtCompat_p.h"

#include <vector>
#include <memory>

namespace KDDockWidgets {

namespace Core {

class EventFilterInterface;
class DOCKS_EXPORT_FOR_UNIT_TESTS View::Private
{
public:
    explicit Private(View *qq, const QString &id, ViewType type)
        : q(qq)
        , m_id(id)
        , m_type(type)
    {
    }

    /// @brief signal emitted once ~View starts
    KDBindings::Signal<> beingDestroyed;

    /// @brief signal emitted when something tried to close this view
    KDBindings::Signal<CloseEvent *> closeRequested;

    /// @brief signal emitted when constraints change, for example min/max sizes
    KDBindings::Signal<> layoutInvalidated;

    /// @brief signal emitted when the view is resized
    KDBindings::Signal<Size> resized;

    /// List of event filters
    std::vector<EventFilterInterface *> m_viewEventFilters;

    /// @brief Returns the views's geometry, but always in global space
    Rect globalGeometry() const;

    /// Returns which screen this view is on
    /// In Qt this is QWindow::screen()
    std::shared_ptr<Screen> screen() const;

    /// Called by the framework when the user tries to close the view
    /// The view can accept or ignore this event
    void requestClose(CloseEvent *);

    /// @brief If true, it means destruction hasn't happen yet but is about to happen.
    /// Useful when a controller is under destructions and wants all related views to stop painting
    /// or doing anything that would call back into the controller. If false, it doesn't mean
    /// anything, as not all controllers are using this.
    void setAboutToBeDestroyed();
    bool aboutToBeDestroyed() const;

    /// @brief Convenience. See Window::transientWindow().
    std::shared_ptr<Core::Window> transientWindow() const;

    ///@brief Returns the type of this view
    ViewType type() const;

    ///@brief returns an id for correlation purposes for saving layouts
    QString id() const;

    Controller *firstParentOfType(ViewType) const;

    /// @brief returns whether this view is inside the specified window
    bool isInWindow(std::shared_ptr<Core::Window> window) const;

    /// @brief Deletes this view and marks it as being deleted to avoid controller deleting it
    void free();

    /// @brief Returns whether free() has already been called
    bool freed() const;

    void closeRootView();
    Rect windowGeometry() const;
    Size parentSize() const;

    /// If this view is wrapped in a shared ptr, this weak ptr allows us to promote to shared ptr
    std::weak_ptr<View> m_thisWeakPtr;

    View *const q;
    bool m_freed = false;
    bool m_aboutToBeDestroyed = false;
    const QString m_id;
    const ViewType m_type;
};

}

}
