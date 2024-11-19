/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief Represents a dock widget.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KD_DOCKWIDGET_WIDGETS_H
#define KD_DOCKWIDGET_WIDGETS_H
#pragma once

#include "kddockwidgets/core/views/DockWidgetViewInterface.h"
#include "View.h"

// clazy:excludeall=ctor-missing-parent-argument

namespace KDDockWidgets {

namespace QtWidgets {

/**
 * @brief Represents a dock widget.
 *
 * Most of the interface lives in Core::DockWidget, to facilitate sharing with QtQuick.
 */
#ifdef PYTHON_BINDINGS
class DOCKS_EXPORT DockWidget : public QWidget,
                                public Core::DockWidgetViewInterface
#else
class DOCKS_EXPORT DockWidget : public QtWidgets::View<QWidget>,
                                public Core::DockWidgetViewInterface
#endif
{
    Q_OBJECT
public:
    using QWidget::size;

    /**
     * @brief constructs a new DockWidget
     * @param uniqueName Mandatory name that should be unique between all DockWidget instances.
     *        This name won't be user visible and just used internally for the save/restore.
     *        Use setTitle() for user visible text.
     * @param options optional options controlling behaviour
     *
     * There's no parent argument. The DockWidget is either parented to FloatingWindow or MainWindow
     * when visible, or stays without a parent when hidden. This allows to support docking
     * to different main windows.
     */
    explicit DockWidget(const QString &uniqueName, DockWidgetOptions options = {},
                        LayoutSaverOptions layoutSaverOptions = {},
                        Qt::WindowFlags windowFlags = Qt::Tool);

    ///@brief destructor
    ~DockWidget() override;

    /**
     * @brief sets the widget which this dock widget hosts.
     * @param widget the widget to show inside this dock widget. Must not be null.
     *
     * Ownership for @p widget is transferred to Core::DockWidget.
     * Ownsership for any previously existing widget is transferred back to the user. Meaning if you
     * call setWidget(A) followed by setWidget(B) then A will have to be deleted by you, while B is
     * owned by the dock widget.
     */
    void setWidget(QWidget *widget);

    /// @brief Returns the guest widget
    QWidget *widget() const;

    QAction *toggleAction() const;
    QAction *floatAction() const;

#ifndef PYTHON_BINDINGS
    // Override QWidget::show() as there's more to do.
    void show() override
    {
        Core::DockWidgetViewInterface::open();
    }

    // Override QWidget::raise() as there's more to do, like setting it as current tab
    // if it's tabbed
    void raise() override
    {
        Core::DockWidgetViewInterface::raise();
    }
#endif

Q_SIGNALS:
    void optionsChanged(KDDockWidgets::DockWidgetOptions);
    void guestViewChanged();
    void isFocusedChanged(bool);
    void isFloatingChanged(bool);
    void isOpenChanged(bool);
    void windowActiveAboutToChange(bool);

    /// @brief Emitted when a dock widget becomes current or not in its tab group
    /// @param isCurrent true if it became current
    void isCurrentTabChanged(bool);

protected:
    bool event(QEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *ev) override;
    void resizeEvent(QResizeEvent *) override;
    std::shared_ptr<Core::View> focusCandidate() const override;

private:
    class Private;
    Private *const d;
};

}

}

#endif
