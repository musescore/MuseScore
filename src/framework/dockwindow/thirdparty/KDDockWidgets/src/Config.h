/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief Application-wide config to tune certain behaviours of the framework.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KD_DOCKWIDGETS_CONFIG_H
#define KD_DOCKWIDGETS_CONFIG_H

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"

namespace KDDockWidgets {

namespace Core {
class DockWidget;
class MainWindow;
class DropArea;
class Draggable;
class ViewFactory;
}

typedef KDDockWidgets::Core::DockWidget *(*DockWidgetFactoryFunc)(const QString &name);
typedef KDDockWidgets::Core::MainWindow *(*MainWindowFactoryFunc)(const QString &name, KDDockWidgets::MainWindowOptions);
typedef bool (*DragAboutToStartFunc)(Core::Draggable *draggable);
typedef void (*DragEndedFunc)();

/// @brief Function to allow more granularity to disallow where widgets are dropped
///
/// By default, widgets can be dropped to the outer and inner left/right/top/bottom
/// and center. The client app can however provide a lambda via setDropIndicatorAllowedFunc
/// to block (by returning false) any specific locations they desire.
///
/// @param location The drop indicator location to allow or disallow
/// @param source The dock widgets being dragged
/// @param target The dock widgets within an existing docked tab group
/// @param dropArea The target drop area. Can belong to a MainWindow or a FloatingWindow.
/// @return true if the docking is allowed.
/// @sa setDropIndicatorAllowedFunc
typedef bool (*DropIndicatorAllowedFunc)(DropLocation location,
                                         const Vector<Core::DockWidget *> &source,
                                         const Vector<Core::DockWidget *> &target,
                                         Core::DropArea *dropArea);

/**
 * @brief Singleton to allow to choose certain behaviours of the framework.
 *
 * The setters should only be used before creating any DockWidget or MainWindow,
 * preferably right after creating the QApplication.
 */
class DOCKS_EXPORT Config
{
public:
    ///@brief returns the singleton Config instance
    static Config &self();

    ///@brief destructor, called at shutdown
    ~Config();

    ///@brief Flag enum to tune certain behaviours, the defaults are Flag_Default
    ///@warning Only the default is supported on all platforms. Not all options work with all window
    /// managers,
    ///         Qt does its best to abstract the differences however that's only a best effort. This
    ///         is true specially for any option that changes window flags.
    enum Flag {
        Flag_None = 0, ///< No option set
        Flag_NativeTitleBar = 1, ///< Enables the Native OS title bar on OSes that support it
                                 ///< (Windows 10, macOS), ignored otherwise.
        Flag_AeroSnapWithClientDecos =
            2, ///< Deprecated. This is now default and cannot be turned off. Moving a window on
               ///< Windows 10 uses native moving, as that works well across screens with different
               ///< HDPI settings. There's no reason to use manual client/Qt window moving.
        Flag_AlwaysTitleBarWhenFloating =
            4, ///< Floating windows will have a title bar even if Flag_HideTitleBarWhenTabsVisible
               ///< is specified. Unneeded if Flag_HideTitleBarWhenTabsVisible isn't specified, as
               ///< that's the default already.
        Flag_HideTitleBarWhenTabsVisible = 8, ///< Hides the title bar if there's tabs visible. The
                                              ///< empty space in the tab bar becomes draggable.
        Flag_AlwaysShowTabs = 16, ///< Always show tabs, even if there's only one,
        Flag_AllowReorderTabs = 32, ///< Allows user to re-order tabs by dragging them
        Flag_TabsHaveCloseButton =
            64, ///< Tabs will have a close button. Equivalent to QTabWidget::setTabsClosable(true).
        Flag_DoubleClickMaximizes = 128, ///< Double clicking the titlebar will maximize a floating
                                         ///< window instead of re-docking it
        Flag_TitleBarHasMaximizeButton =
            256, ///< The title bar will have a maximize/restore button when floating. This is
                 ///< mutually-exclusive with the floating button (since many apps behave that way).
        Flag_TitleBarIsFocusable =
            512, ///< You can click the title bar and it will focus the last focused widget in the
                 ///< focus scope. If no previously focused widget then it focuses the user's dock
                 ///< widget guest, which should accept focus or use a focus proxy.
        Flag_LazyResize = 1024, ///< The dock widgets are resized in a lazy manner. The actual
                                ///< resize only happens when you release the mouse button.
        Flag_DontUseUtilityFloatingWindows = 0x1000,
        Flag_TitleBarHasMinimizeButton =
            0x2000 | Flag_DontUseUtilityFloatingWindows, ///< The title bar will have a minimize
                                                         ///< button when floating. This implies
                                                         ///< Flag_DontUseUtilityFloatingWindows
                                                         ///< too, otherwise they wouldn't appear in
                                                         ///< the task bar.
        Flag_TitleBarNoFloatButton = 0x4000, ///< The TitleBar won't show the float button
        Flag_AutoHideSupport =
            0x8000 | Flag_TitleBarNoFloatButton, ///< Supports minimizing dock widgets to the
                                                 ///< side-bar. By default it also turns off the
                                                 ///< float button, but you can remove
                                                 ///< Flag_TitleBarNoFloatButton to have both.
        Flag_KeepAboveIfNotUtilityWindow =
            0x10000, ///< Only meaningful if Flag_DontUseUtilityFloatingWindows is set. If floating
                     ///< windows are normal windows, you might still want them to keep above and
                     ///< not minimize when you focus the main window.
        Flag_CloseOnlyCurrentTab = 0x20000, ///< The TitleBar's close button will only close the
                                            ///< current tab, instead of all of them
        Flag_ShowButtonsOnTabBarIfTitleBarHidden =
            0x40000, ///< When using Flag_HideTitleBarWhenTabsVisible the close/float buttons
                     ///< disappear with the title bar. With Flag_ShowButtonsOnTabBarIfHidden
                     ///< they'll be shown in the tab bar.
        Flag_AllowSwitchingTabsViaMenu = 0x80000, ///< Allow switching tabs via a context menu when
                                                  ///< right clicking on the tab area
        Flag_AutoHideAsTabGroups = 0x100000, ///< If tabbed dockwidgets are sent to/from sidebar, they're all sent and restored together
        Flag_Default = Flag_AeroSnapWithClientDecos ///< The defaults
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    ///@brief List of customizable widgets
    enum CustomizableWidget {
        CustomizableWidget_None = 0, ///< None
        CustomizableWidget_TitleBar, ///< The title bar
        CustomizableWidget_DockWidget, ///< The dock widget
        CustomizableWidget_Frame, ///< The container for a group of 1 or more dockwidgets which are
                                  ///< tabbed together
        CustomizableWidget_TabBar, ///< The tab bar, child of Frame, which contains 1 or more dock
                                   ///< widgets
        CustomizableWidget_TabWidget, ///< The tab widget which relates to the tab bar
        CustomizableWidget_FloatingWindow, ///< A top-level window. The container for 1 or more
                                           ///< Frame nested side by side
        CustomizableWidget_Separator ///< The draggable separator between dock widgets in a layout
    };
    Q_DECLARE_FLAGS(CustomizableWidgets, CustomizableWidget)

    ///@internal
    /// Internal flags for additional tuning.
    ///@warning Not for public consumption, support will be limited.
    enum InternalFlag {
        InternalFlag_None = 0, ///< The default
        InternalFlag_NoAeroSnap = 1, ///< Only for development. Disables Aero-snap.
        InternalFlag_DontUseParentForFloatingWindows =
            2, ///< FloatingWindows won't have a parent top-level.
        InternalFlag_DontUseQtToolWindowsForFloatingWindows =
            4, ///< FloatingWindows will use Qt::Window instead of Qt::Tool.
        InternalFlag_DontShowWhenUnfloatingHiddenWindow =
            8, ///< DockWidget::setFloating(false) won't do anything if the window is hidden.
        InternalFlag_UseTransparentFloatingWindow =
            16, ///< For QtQuick only. Allows to have round-corners. It's flaky when used with
                ///< native Windows drop-shadow.
        InternalFlag_DisableTranslucency =
            32, ///< KDDW tries to detect if your Window Manager doesn't support transparent
                ///< windows, but the detection might fail
        /// with more exotic setups. This flag can be used to override.
        InternalFlag_TopLevelIndicatorRubberBand =
            64, ///< Makes the rubber band of classic drop indicators to be top-level windows. Helps
                ///< with working around MFC bugs,
        InternalFlag_NoDeleteLaterWorkaround = 128, ///< Disables workaround for QTBUG-83030. Will be the default since Qt 6.7
                                                    /// While the workaround works, it will cause memory leaks at shutdown,
        /// This flag allows to disable the workaround if you think you don't have the complex setup reported in QTBUG-83030
        InternalFlag_DeleteSeparatorsLater = 256 ///< Uses deleteLater() when disposing of separators
    };
    Q_DECLARE_FLAGS(InternalFlags, InternalFlag)

    /// Flags to be used in MDI mode
    enum MDIFlag {
        MDIFlag_None = 0,
        MDIFlag_NoClickToRaise = 1 ///< Clicking on a MDI widget won't raise it
    };
    Q_DECLARE_FLAGS(MDIFlags, MDIFlag)

    ///@brief returns the chosen flags
    Flags flags() const;

    ///@brief returns the chosen MDI flags
    /// default is MDIFlag_None
    MDIFlags mdiFlags() const;

    ///@brief setter for the flags
    ///@param flags the flags to set
    /// Not all flags are guaranteed to be set, as the OS might not supported them
    /// Call @ref flags() after the setter if you need to know what was really set
    void setFlags(Flags flags);

    /// Setter for the MDI flags
    void setMDIFlags(MDIFlags);

    ///@brief Returns whether the specified flag is set or not
    static bool hasFlag(Flag);

    ///@brief Returns whether the specified MDI flag is set or not
    static bool hasMDIFlag(MDIFlag);

    /**
     * @brief Registers a DockWidgetFactoryFunc.
     *
     * This is optional, the default is nullptr.
     *
     * A DockWidgetFactoryFunc is a function that receives a dock widget name
     * and returns a DockWidget instance.
     *
     * While restoring, @ref LayoutSaver requires all dock widgets to exist.
     * If a DockWidget doesn't exist then a DockWidgetFactoryFunc function is
     * required, so the layout saver can ask to create the DockWidget and then
     * restore it.
     */
    void setDockWidgetFactoryFunc(DockWidgetFactoryFunc);

    ///@brief Returns the DockWidgetFactoryFunc.
    /// nullptr by default
    DockWidgetFactoryFunc dockWidgetFactoryFunc() const;

    ///@brief counter-part of DockWidgetFactoryFunc but for the main window.
    /// Should be rarely used. It's good practice to have the main window before restoring a layout.
    /// It's here so we can use it in the linter executable
    void setMainWindowFactoryFunc(MainWindowFactoryFunc);

    ///@brief Returns the MainWindowFactoryFunc.
    /// nullptr by default
    MainWindowFactoryFunc mainWindowFactoryFunc() const;

    /**
     * @brief Sets the ViewFactory.
     *
     * To draw things on screen, KDDW uses QtWidgets::{Group, TitleBar, TabBar, Separator, DockWidget, etc} (same for QtQuick::*)
     * You can set your own factory and provide classes derived from the above list to override visual behaviour.
     *
     * Ownership is taken.
     */
    void setViewFactory(Core::ViewFactory *);

    ///@brief getter for the framework view factory
    Core::ViewFactory *viewFactory() const;

    /**
     * @brief Returns the thickness of the separator.
     *
     * Default is 5px.
     */
    int separatorThickness() const;

    ///@brief setter for @ref separatorThickness
    /// Note: Only use this function at startup before creating any DockWidget or MainWindow.
    /// Note: For backwards compatibility, setting separatorThickness will set layoutSpacing to the same value.
    void setSeparatorThickness(int value);

    /// Returns the spacing between dock widgets
    /// By default this is the thickness of the separators, as they are between dock widgets.
    int layoutSpacing() const;

    /// Setter for layoutSpacing().
    /// Note: Only call this for the rare case of wanting the spacing to be different than the separator's thickness
    ///       Use setSeparatorThickness() for the more common case.
    /// Note: Only use this function at startup before creating any DockWidget or MainWindow.
    void setLayoutSpacing(int);

    ///@brief sets the dragged window opacity
    /// 1.0 is fully opaque while 0.0 is fully transparent
    void setDraggedWindowOpacity(double opacity);

    /// @brief Sets whether transparency is only set when the dragged window is over a drop indicator
    /// This is only relevant when using setDraggedWindowOpacity()
    /// Default is false
    void setTransparencyOnlyOverDropIndicator(bool only);

    ///@brief returns the opacity to use when dragging dock widgets
    /// By default it's 1.0, fully opaque
    double draggedWindowOpacity() const;

    /// @brief Returns whether transparency is only set when the dragged window is over a drop indicator
    /// This is only relevant when using setDraggedWindowOpacity()
    /// Default is false
    bool transparencyOnlyOverDropIndicator() const;

    /// @brief Allows to disable support for drop indicators while dragging
    /// By default drop indicators will be shown when dragging dock widgets.
    /// This functionality can be toggled whenever you need it (it's not a startup-only setting).
    void setDropIndicatorsInhibited(bool inhibit) const;

    /// @brief Returns whether drop indicators are inhibited.
    /// by default this is false unless you call setDropIndicatorsInhibited(true)
    bool dropIndicatorsInhibited() const;

    /**
     * @brief Allows the client app to disallow certain docking indicators.
     *
     * For example, let's assume the app doesn't want to show outer indicators for a certain
     * dock widget.
     *
     * @code
     * #include <kddockwidgets/Config.h>
     * (...)
     *
     * auto func = [] (KDDockWidgets::DropLocation loc,
     *                 const KDDockWidgets::Core::DockWidget::List &source,
     *                 const KDDockWidgets::Core::DockWidget::List &target,
     *                 KDDockWidgets::Core::DropArea *)
     * {
     *    // disallows dockFoo to be docked to outer areas
     *    return !((loc & KDDockWidgets::DropLocation_Outter) && source.contains(dockFoo));
     * };
     *
     * KDDockWidgets::Config::self().setDropIndicatorAllowedFunc(func);
     *
     * @endcode
     *
     * Run "examples/qtwidgets_dockwidgets --hide-certain-docking-indicators" to see this in action.
     */
    void setDropIndicatorAllowedFunc(DropIndicatorAllowedFunc func);

    /// @brief set a callback to be called once a drag starts
    ///
    /// This function is for advanced usage only. Allows more granularity for
    /// inhibiting DnD.
    ///
    /// Return true/false whether you allow the drag or not.
    ///
    /// @param draggable The thing about to be dragged. Can be a tab, a group of tabs, a floating window or
    ///                  the tabbar's background.
    ///
    /// @sa setDragEndedFunc
    void setDragAboutToStartFunc(DragAboutToStartFunc func);
    DragAboutToStartFunc dragAboutToStartFunc() const;

    /// @brief set a callback to be called once drag ends
    ///
    /// @sa setAboutToStartDragFunc
    void setDragEndedFunc(DragEndedFunc func);
    DragEndedFunc dragEndedFunc() const;

    ///@brief Used internally by the framework. Returns the function which was passed to
    /// setDropIndicatorAllowedFunc()
    /// By default it's nullptr.
    ///@sa setDropIndicatorAllowedFunc().
    DropIndicatorAllowedFunc dropIndicatorAllowedFunc() const;

    ///@brief Sets the minimum size a dock widget can have.
    /// Widgets can still provide their own min-size and it will be respected, however it can never
    /// be smaller than this one.
    void setAbsoluteWidgetMinSize(Size size);
    Size absoluteWidgetMinSize() const;

    ///@brief Sets the maximum size a dock widget can have.
    /// Widgets can still provide their own max-size and it will be respected, however it can never
    /// be bigger than this one.
    void setAbsoluteWidgetMaxSize(Size size);
    Size absoluteWidgetMaxSize() const;

    ///@brief Disables our internal widget's paint events
    /// By default, KDDockWidget's internal widgets reimplement paintEvent(). Disabling them
    /// (which makes the base-class, QWidget::paintEvent() be called instead) can be useful if you
    /// want to style
    // via CSS stylesheets.
    void setDisabledPaintEvents(CustomizableWidgets);
    Config::CustomizableWidgets disabledPaintEvents() const;

    ///@internal
    ///@brief returns the internal flags.
    ///@warning Not for public consumption, support will be limited.
    InternalFlags internalFlags() const;

    ///@internal
    ///@brief setter for the internal flags
    ///@warning Not for public consumption, support will be limited.
    void setInternalFlags(InternalFlags flags);

    /// @brief Sets the MDI popup threshold. When the layout is MDI and you drag a dock widget
    /// X pixels behond the window's edge, it will float the dock widget.
    /// by default this value is 250px. Use -1 to disable
    void setMDIPopupThreshold(int);
    int mdiPopupThreshold() const;

    /// @brief Sets how many pixels the mouse needs to travel before a drag is actually started
    /// Calling this is usually unneeded and just provided as a means to override
    /// Platform::startDragDistance() , which already has a reasonable default 4 pixels
    void setStartDragDistance(int);

    /// @brief Returns the value set by setStartDragDistance()
    /// Returns -1 if setStartDragDistance() wasn't call, in which case the
    /// Platform::startDragDistance() will be used
    int startDragDistance() const;

    /// Prints some debug information
    void printDebug();

    /// Mostly used by the linter so it errors out when finding invalid layouts.
    /// For production, this should be false, as sometimes there's fallbacks and the layout is recoverable.
    /// Default is false.
    void setLayoutSaverStrictMode(bool);
    bool layoutSaverUsesStrictMode() const;

    /// For disallowing DnD to be started by mouse and instead require doing programmatically
    /// Default is false, DnD can be started by mouse, which is the most common use case.
    /// @sa Core::DockWidget::startDragging()
    void setOnlyProgrammaticDrag(bool);
    bool onlyProgrammaticDrag() const;

private:
    KDDW_DELETE_COPY_CTOR(Config)
    Config();
    class Private;
    Private *const d;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KDDockWidgets::Config::Flags)

#endif
