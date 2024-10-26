/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "Controller.h"
#include "../core/Draggable_p.h"
#include "kddockwidgets/docks_export.h"
#include "DockWidget.h"

namespace KDDockWidgets {
namespace QtWidgets {
class TitleBar;
}
}

namespace KDDockWidgets::Core {

class Group;
class FloatingWindow;
class TabBar;

class DOCKS_EXPORT TitleBar : public Controller, public Draggable
{
    Q_OBJECT
public:
    /// Creates the TitleBar used by Floating windows
    explicit TitleBar(FloatingWindow *parent);

    /// Creates the TitleBar used by tab groups
    explicit TitleBar(Group *parent);

    /// Creates a standalone TitleBar
    /// That means it's not associated with any docking. To allow users to reuse it.
    /// For example, to add a title bar to a QMessageBox popup on EGLFS
    /// @sa isStandalone()
    explicit TitleBar(Core::View *);

    virtual ~TitleBar() override;

    /// From Draggable interface
    bool isMDI() const override;
    std::unique_ptr<WindowBeingDragged> makeWindow() override;
    bool isWindow() const override;
    Core::DockWidget *singleDockWidget() const override final;

    void setTitle(const QString &title);
    QString title() const;

    ///@brief Returns true if this title-bar is the title bar of a floating window
    bool isFloating() const;

    /// Returns whether this titlebar is standalone. See comment in the ctor.
    bool isStandalone() const;

    ///@brief the list of dockwidgets under this TitleBar.
    /// There should always be at least 1. If more than 1 then they are tabbed.
    DockWidget::List dockWidgets() const;

    ///@brief the icon
    Icon icon() const;
    void setIcon(const Icon &icon);

    ///@brief returns whether this title bar supports a floating/docking button
    bool supportsFloatingButton() const;

    ///@brief returns whether this title bar supports a maximize/restore button
    bool supportsMaximizeButton() const;

    ///@brief returns whether this title bar supports a minimize button
    bool supportsMinimizeButton() const;

    ///@brief returns whether this title bar supports the auto-hide button
    bool supportsAutoHideButton() const;

#ifdef DOCKS_TESTING_METHODS
    /// @brief returns whether the float button is visible
    bool isFloatButtonVisible() const;
    bool isCloseButtonVisible() const;
    bool isCloseButtonEnabled() const;
#endif

    ///@brief returns whether this title bar has an icon
    bool hasIcon() const;

    ///@brief returns whether any of the DockWidgets this TitleBar controls has a child focus
    /// Not to be confused with QWidget::hasFocus(), which just refers to 1 widget. This works more
    /// like QtQuick's FocusScope
    bool isFocused() const;

    bool titleBarIsFocusable() const;

    ///@brief getter for m_group
    Core::Group *group() const;

    ///@brief getter for m_floatingWindow
    FloatingWindow *floatingWindow() const;

    ///@brief If this title bar belongs to a dock widget docked into the main window, returns the
    /// main window
    /// Returns nullptr otherwise
    MainWindow *mainWindow() const;

    ///@brief Returns the tab bar which is under this title bar.
    /// It's only nullptr for the case of having a Floating Window with more than one nested Frame
    TabBar *tabBar() const;

    /// Will focus the last user widget (below this titlebar) that had focus
    /// Requires Config::Flag_TitleBarIsFocusable
    void focus(Qt::FocusReason reason);

    /// @brief updates the close button enabled state
    void updateButtons();
    QString floatButtonToolTip() const;

    bool onDoubleClicked();
    void onCloseClicked();
    void onFloatClicked();
    void onMaximizeClicked();
    void onMinimizeClicked();
    void onAutoHideClicked();
    void toggleMaximized();

    // TODO: Refactor into a QHash<Button,State> ?
    bool closeButtonEnabled() const;
    void setCloseButtonVisible(bool);
    bool floatButtonVisible() const;
    bool maximizeButtonVisible() const;
    void setCloseButtonEnabled(bool);
    void setFloatButtonVisible(bool);

    TitleBarButtonType maximizeButtonType() const;

    /// Allows to override's KDDW's default visibility logic and force hide a button
    void setUserHiddenButtons(TitleBarButtonTypes);
    bool buttonIsUserHidden(TitleBarButtonType) const;

    /// Overload which also consults buttonHidesIfDisabled
    bool buttonIsUserHidden(TitleBarButtonType, bool enabled) const;

    /// The specified buttons, if disabled, will be hidden as well
    /// for example, with non-closable dock widgets we disable the close button
    /// this allows to hide it as well.
    void setHideDisabledButtons(TitleBarButtonTypes);
    bool buttonHidesIfDisabled(TitleBarButtonType) const;

    class Private;
    Private *dptr() const;

protected:
    bool isOverlayed() const;

private:
    friend class ::TestDocks;
    friend class Core::Group;
    friend class KDDockWidgets::QtWidgets::TitleBar;

    void updateAutoHideButton();
    void updateMaximizeButton();

    void updateFloatButton();
    void updateCloseButton();
    bool supportsFloatUnfloat() const;
    void setFloatButtonToolTip(const QString &);
    void init();

    Private *const d;

    Point m_pressPos;
    QString m_title;
    Icon m_icon;

    Core::Group *const m_group;
    FloatingWindow *const m_floatingWindow;
    const bool m_supportsAutoHide;
    const bool m_isStandalone;
    bool m_closeButtonEnabled = true;
    bool m_closeButtonVisible = true;
    bool m_floatButtonVisible = true;
    bool m_maximizeButtonVisible = false;
    TitleBarButtonType m_maximizeButtonType = TitleBarButtonType::Maximize;
    QString m_floatButtonToolTip;
};

}
