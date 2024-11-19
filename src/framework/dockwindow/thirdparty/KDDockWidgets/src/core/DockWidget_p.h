/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_DOCKWIDGET_P_H
#define KD_DOCKWIDGET_P_H

#include "DockWidget.h"
#include "DockRegistry.h"
#include "core/Position_p.h"
#include "Action.h"
#include "core/View_p.h"
#include "QtCompat_p.h"

namespace KDDockWidgets {

namespace Core {
class SideBar;

class DOCKS_EXPORT_FOR_UNIT_TESTS DockWidget::Private
{
public:
    /// RAII class to help updating actions exactly once, otherwise they can be triggered in the
    /// middle of operations during reparenting
    struct UpdateActions
    {
        explicit UpdateActions(Core::DockWidget *dock)
            : dw(dock)
        {
            dw->d->m_willUpdateActions++;
        }

        ~UpdateActions()
        {
            dw->d->m_willUpdateActions--;

            // only the last one (the outer one, updates actions, in case of nesting)
            if (dw->d->m_willUpdateActions == 0) {
                dw->d->updateFloatAction();
                if (dw->isOpen() != dw->toggleAction()->isChecked())
                    dw->d->updateToggleAction();
            }
        }

    private:
        KDDW_DELETE_COPY_CTOR(UpdateActions)
        Core::DockWidget *const dw;
    };

    Private(const QString &dockName, DockWidgetOptions options_,
            LayoutSaverOptions layoutSaverOptions_, DockWidget *qq);

    ~Private();

    void init()
    {
        updateTitle();
        q->view()->d->closeRequested.connect([this](CloseEvent *ev) { onCloseEvent(ev); });
    }

    /**
     * @brief returns the FloatingWindow this dock widget is in. If nullptr then it's in a
     * MainWindow.
     *
     * Note: Being in a FloatingWindow doesn't necessarily mean @ref isFloating() returns true, as
     * the dock widget might be in a floating window with other dock widgets side by side.
     */
    Core::FloatingWindow *floatingWindow() const
    {
        if (auto fw = q->view()->rootView()->asFloatingWindowController())
            return fw;

        return nullptr;
    }

    MainWindow *mainWindow() const
    {
        if (q->view()->isRootView())
            return nullptr;

        // Note: Don't simply use window(), as the MainWindow might be embedded into something else
        auto p = q->view()->parentView();
        while (p) {
            if (auto mw = p->asMainWindowController())
                return mw;

            if (p->isRootView())
                return nullptr;

            p = p->parentView();
        }

        return nullptr;
    }

    Core::SideBar *sideBar() const
    {
        return DockRegistry::self()->sideBarForDockWidget(q);
    }

    ///@brief adds the current layout item containing this dock widget
    void addPlaceholderItem(Core::Item *);

    ///@brief returns the last position, just for tests.
    Position::Ptr &lastPosition();

    void forceClose();
    Point defaultCenterPosForFloating();

    void onWindowActivated(std::shared_ptr<View> rootView);
    void onWindowDeactivated(std::shared_ptr<View> rootView);

    void updateTitle();
    void toggle(bool enabled);
    void updateToggleAction();
    void updateFloatAction();
    void close();
    bool restoreToPreviousPosition();
    void maybeRestoreToPreviousPosition();
    int currentTabIndex() const;
    void onCloseEvent(CloseEvent *);
    void onParentChanged();

    /**
     * @brief Serializes this dock widget into an intermediate form
     */
    std::shared_ptr<LayoutSaver::DockWidget> serialize() const;

    /**
     * @brief the Group which contains this dock widgets.
     *
     * A group wraps a docked DockWidget, giving it a TabWidget so it can accept other dock widgets.
     * Frame is also the actual class that goes into a Layout.
     *
     * It's nullptr immediately after creation.
     */
    Core::Group *group() const;

    /**
     * Returns the Layout Item this DockWidget is in.
     * Can be nullptr.
     */
    Core::Item *item() const;

    ///@brief If this dock widget is floating, then it saves its geometry
    void saveLastFloatingGeometry();

    /**
     * Before floating a dock widget we save its position. So it can be restored when calling
     * DockWidget::setFloating(false)
     */
    void saveTabIndex();

    /**
     * @brief Creates a FloatingWindow and adds itself into it
     * @return the created FloatingWindow
     */
    Core::FloatingWindow *morphIntoFloatingWindow();

    /// @brief calls morphIntoFloatingWindow() if the dock widget is visible and is a top-level
    /// This is called delayed whenever we show a floating dock widget, so we get a FloatingWindow
    void maybeMorphIntoFloatingWindow();

    /// @brief Returns the mdi layout this dock widget is in, if any.
    MDILayout *mdiLayout() const;

    /// @brief Returns if this is an helper DockWidget created automatically to host a drop area
    /// inside MDI This is only used by the DockWidgetOption_MDINestable feature
    bool isMDIWrapper() const;

    /// @brief If this dock widget is an MDI wrapper (isMDIWrapper()), then returns the wrapper drop
    /// area
    DropArea *mdiDropAreaWrapper() const;

    /// @brief If this dock widget is inside a drop area nested in MDI then returns the wrapper dock
    /// widget This goes up the hierarchy, while mdiDropAreaWrapper goes down.
    DockWidget *mdiDockWidgetWrapper() const;

    void setIsOpen(bool);

    ///@brief signal emitted when the icon changed
    KDBindings::Signal<> iconChanged;

    ///@brief signal emitted when the title changed
    ///@param title the new title
    KDBindings::Signal<QString> titleChanged;

    /// @brief emitted when the hosted guest widget changed
    KDBindings::Signal<> guestViewChanged;

    ///@brief emitted when the options change
    ///@sa setOptions(), options()
    ///@param options
    KDBindings::Signal<KDDockWidgets::DockWidgetOptions> optionsChanged;

    ///@brief emitted when isFocused changes
    ///@sa isFocused
    ///@param isFocused
    KDBindings::Signal<bool> isFocusedChanged;

    ///@brief emitted when isOverlayed changes
    ///@sa isOverlayed
    ///@param isOverlayed
    KDBindings::Signal<bool> isOverlayedChanged;

    ///@brief emitted when isFloating changes
    ///@param isFloating
    KDBindings::Signal<bool> isFloatingChanged;

    ///@brief emitted when this dock widget is removed from a side-bar.
    /// Only relevant for the auto-hide/sidebar feature
    KDBindings::Signal<> removedFromSideBar;

    ///@brief Emitted when the top-level window this dock widget is in is activated or deactivated
    /// This is convenience to replace tracking dockWidget->window(), since the window changes when
    /// docking and undocking
    ///
    /// It's called 'aboutTo' because it's done in an event filter and the target window doesn't
    /// have it's 'activeWindow' property updated yet at this point.
    /// @param activated
    KDBindings::Signal<bool> windowActiveAboutToChange;

    ///@brief Emitted when the title bar that serves this dock widget changes
    KDBindings::Signal<> actualTitleBarChanged;

    /// @brief Emitted when this dock widget is about to be deleted due to Option_DeleteOnClose
    KDBindings::Signal<> aboutToDeleteOnClose;

    /// @brief Emitted when this dock widget is about to be deleted
    /// @param dockWidget
    KDBindings::Signal<KDDockWidgets::Core::DockWidget *> aboutToDelete;

    /// @brief Emitted when a dock widget is closed
    /// This is equivalent to the openedChanged(false) signal
    KDBindings::Signal<> closed;

    /// @brief Emitted when a dock widget is opened or closed
    /// For the false case, closed() is also emitted
    /// @param isOpen
    KDBindings::Signal<bool> isOpenChanged;

    /// @brief Emitted when a dock widget becomes current or not in its tab group
    /// @param isCurrent true if it became current
    KDBindings::Signal<bool> isCurrentTabChanged;

    /// Each dock widget has a unique name which is used for layout save/restore
    QString uniqueName() const;

    /// Sets the dock widget unique name.
    /// DockWidgets have their name set once (passed in ctor), therefore this method doesn't need
    /// to be called, unless you know what you're doing (like reusing dock widgets during restore)
    void setUniqueName(const QString &);

private:
    // Go through the setter
    QString m_uniqueName;

public:
    Vector<QString> affinities;
    QString title;
    Icon titleBarIcon;
    Icon tabBarIcon;
    std::shared_ptr<View> guest;
    DockWidget *const q;
    DockWidgetOptions options;
    FloatingWindowFlags m_flags = FloatingWindowFlag::FromGlobalConfig;
    const LayoutSaverOptions layoutSaverOptions;
    Action *const toggleAction;
    Action *const floatAction;
    Position::Ptr m_lastPosition = std::make_shared<Position>();
    bool m_isPersistentCentralDockWidget = false;
    bool m_processingToggleAction = false;
    bool m_updatingToggleAction = false;
    bool m_updatingFloatAction = false;
    bool m_isForceClosing = false;
    bool m_isMovingToSideBar = false;
    bool m_isSettingCurrent = false;
    bool m_isOpen = false;
    bool m_inOpenSetter = false;
    bool m_inClose = false;
    bool m_inCloseEvent = false;
    bool m_removingFromOverlay = false;
    bool m_wasRestored = false;
    Size m_lastOverlayedSize = Size(0, 0);
    int m_userType = 0;
    int m_willUpdateActions = 0;
    KDBindings::ScopedConnection m_windowActivatedConnection;
    KDBindings::ScopedConnection m_windowDeactivatedConnection;
    KDBindings::ScopedConnection m_toggleActionConnection;
    KDBindings::ScopedConnection m_floatActionConnection;
    CloseReason m_lastCloseReason = CloseReason::Unspecified;
};

}

}

#endif
