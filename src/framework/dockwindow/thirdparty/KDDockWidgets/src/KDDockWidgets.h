/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief File with KDDockWidgets namespace-level enums and methods.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KD_KDDOCKWIDGETS_H
#define KD_KDDOCKWIDGETS_H

#include "kddockwidgets/docks_export.h"

#include "QtCompat_p.h"

#ifdef KDDW_FRONTEND_QT
#include "Qt5Qt6Compat_p.h"

#ifdef Q_OS_WIN
#define KDDW_FRONTEND_QT_WINDOWS
#endif

#ifdef KDDW_FRONTEND_QTWIDGETS
#include <QWidget>
#endif

#endif

namespace KDDockWidgets::QtQuick {
}
namespace KDDockWidgets::QtWidgets {
}
namespace KDDockWidgets::QtCommon {
}
namespace KDDockWidgets::Flutter {
}

namespace KDDockWidgets {
QT_DOCKS_EXPORT
Q_NAMESPACE

namespace QtWidgets {
class DockWidget;
}

namespace Core {
class Item;
class ItemBoxContainer;
}

enum Location {
    Location_None,
    Location_OnLeft, ///> Left docking location
    Location_OnTop, ///> Top docking location
    Location_OnRight, ///> Right docking location
    Location_OnBottom ///> Bottom docking location
};
Q_ENUM_NS(Location)

enum MainWindowOption {
    MainWindowOption_None = 0, ///> No option set
    MainWindowOption_HasCentralFrame =
        1, ///> Makes the MainWindow always have a central group, for tabbing documents
    MainWindowOption_MDI = 2, ///> The layout will be MDI. DockWidgets can have arbitrary positions,
                              /// not restricted by any layout
    MainWindowOption_HasCentralWidget =
        4 | MainWindowOption_HasCentralFrame, ///> Similar to MainWindowOption_HasCentralFrame but
    ///> you'll have a central widget which can't be detached (Similar to regular QMainWindow). @sa
    /// MainWindowBase::setPersistentCentralWidget()
    MainWindowOption_QDockWidgets = 8, ///> Allows the user to use QDockWidget instead of KDDW DockWidget, while using the KDDW MainWindow
                                       ///> Useful as a porting aid, where you want to migrate your main windows 1 by 1
    MainWindowOption_ManualInit = 16 ///> For compatibility with setupUi() from UIC. See manualInit() for more details
};
Q_DECLARE_FLAGS(MainWindowOptions, MainWindowOption)
Q_ENUM_NS(MainWindowOptions)

///@brief DockWidget options to pass at construction time
enum DockWidgetOption {
    DockWidgetOption_None = 0, ///< No option, the default
    DockWidgetOption_NotClosable =
        1, ///< The DockWidget can't be closed on the [x], only programmatically
    DockWidgetOption_NotDockable = 2, ///< The DockWidget can't be docked, it's always floating
    DockWidgetOption_DeleteOnClose = 4, ///< Deletes the DockWidget when closed
    DockWidgetOption_MDINestable =
        8 ///< EXPERIMENTAL. When this dock widget is being shown in a MDI area it will also allow
          ///< other dock widgets to be dropped to its sides and tabbed
          /// Usually Each MDI "window" corresponds to one DockWidget, with this option each
          /// "window" will have a layout with 1 or more dock widgets Run
          /// "examples/qtwidgets_mdi_with_docking -n" to see it in action
};
Q_DECLARE_FLAGS(DockWidgetOptions, DockWidgetOption)
Q_ENUM_NS(DockWidgetOptions)

/// @brief Options which will affect LayoutSaver save/restore
enum class LayoutSaverOption {
    None = 0, ///< Just use the defaults
    Skip = 1, ///< The dock widget won't participate in save/restore
    CheckForPreviousRestore = 2, ///< When the DockWidget is created it will check if there was a layout restore
    ///< before, and try to recover its previous main window position
};
Q_DECLARE_FLAGS(LayoutSaverOptions, LayoutSaverOption)

enum class IconPlace {
    TitleBar = 1,
    TabBar = 2,
    ToggleAction = 4,
    All = ToggleAction | TitleBar | TabBar
};
Q_ENUM_NS(IconPlace)
Q_DECLARE_FLAGS(IconPlaces, IconPlace)

enum class FrontendType {
    QtWidgets = 1,
    QtQuick,
    Flutter,
};
Q_ENUM_NS(FrontendType)

///@internal
///@brief Describes some sizing strategies for the layouting engine.
/// This is internal. The public API for dealing with sizing is InitialOption.
///@sa InitialOption
enum class DefaultSizeMode {
    ItemSize, ///< Simply uses the Item::size() of the item being added. Actual used size might be
              ///< smaller if our window isn't big enough.
    Fair, ///< Gives an equal relative size as the items that are already in the layout
    FairButFloor, ///< Equal to fair, but if the item we're adding is smaller than the fair
                  ///< suggestion, then that small size is used.
    NoDefaultSizeMode, ///< Don't do any sizing
};
Q_ENUM_NS(DefaultSizeMode)

///@brief Only here for source-compat with v1.2. Do not use.
/// Use InitialVisibilityOption instead.
enum AddingOption {
    AddingOption_None = 0,
    AddingOption_StartHidden
};
Q_ENUM_NS(AddingOption)

enum class InitialVisibilityOption {
    StartVisible = 0, ///< The dock widget is made visible when docked
    StartHidden, ///< Don't show the dock widget when adding it
    PreserveCurrentTab ///< When adding as tabbed, don't change the current index
};
Q_ENUM_NS(InitialVisibilityOption)

enum class NeighbourSqueezeStrategy {
    AllNeighbours, ///< The squeeze is spread between all neighbours, not just immediate ones first
    ImmediateNeighboursFirst ///< The first neighbour takes as much squeeze as it can, only then the
                             ///< next neighbour is squezed, and so forth
};
Q_ENUM_NS(NeighbourSqueezeStrategy)

/**
 * @brief Struct describing the preferred dock widget size and visibility when adding it to a layout
 *
 * You can pass this to MainWindowBase::addDockWidget() to give an hint of your preferred size
 * and visibility.
 *
 * See below the documentation for InitialOption::visibility and InitialOption::preferredSize.
 *
 * @sa MainWindowBase::addDockWidget()
 */
struct DOCKS_EXPORT InitialOption
{
    // Implicit ctors for convenience:

    InitialOption();
    InitialOption(InitialVisibilityOption v);
    InitialOption(Size size);
    InitialOption(InitialVisibilityOption v, Size size);

    bool startsHidden() const
    {
        return visibility == InitialVisibilityOption::StartHidden;
    }

    bool preservesCurrentTab() const
    {
        return visibility == InitialVisibilityOption::PreserveCurrentTab;
    }

    /// Returns preferred height if the container is vertical, otherwise preferred width
    int preferredLength(Qt::Orientation o) const
    {
        return o == Qt::Horizontal ? preferredSize.width() : preferredSize.height();
    }

    bool hasPreferredLength(Qt::Orientation o) const
    {
        return preferredLength(o) > 0;
    }

    /**
     * @brief Allows a dock widget to be docked as hidden.
     *
     * Next time you call DockWidget::show() it will be shown at that place. This avoids
     * flickering, as no show()/hide() workarounds are needed.
     */
    InitialVisibilityOption visibility = InitialVisibilityOption::StartVisible;

    /**
     * @brief Allows to control the size a dock widget should get when docked.
     *
     * If an invalid or empty size is passed then KDDW's default heuristics are applied.
     *
     * Note that usually only the width or the height will be honoured: For example, when adding a
     * dock widget to the left then only the preferred width will be taken into account, as the
     * height will simply fill the whole layout.
     */
    Size preferredSize;

    static NeighbourSqueezeStrategy s_defaultNeighbourSqueezeStrategy;
    NeighbourSqueezeStrategy neighbourSqueezeStrategy = s_defaultNeighbourSqueezeStrategy;

    /// @internal
    InitialOption(DefaultSizeMode mode);

private:
    friend class Core::Item;
    friend class Core::ItemBoxContainer;

    DefaultSizeMode sizeMode = DefaultSizeMode::Fair;
};

enum RestoreOption {
    RestoreOption_None = 0,
    RestoreOption_RelativeToMainWindow =
        1, ///< Skips restoring the main window geometry and the restored dock widgets will use
           ///< relative sizing. Loading layouts won't change the main window geometry and just use
           ///< whatever the user has at the moment.
    RestoreOption_AbsoluteFloatingDockWindows = 2, ///< Skips scaling of floating dock windows relative to the main window.
};
Q_DECLARE_FLAGS(RestoreOptions, RestoreOption)
Q_ENUM_NS(RestoreOptions)

enum class DropIndicatorType {
    Classic, ///< The default
    Segmented, ///< Segmented indicators
    None ///< Don't show any drop indicators while dragging
};
Q_ENUM_NS(DropIndicatorType)

///@internal
enum SuggestedGeometryHint {
    SuggestedGeometryHint_None,
    SuggestedGeometryHint_PreserveCenter = 1,
    SuggestedGeometryHint_GeometryIsFromDocked = 2
};
Q_DECLARE_FLAGS(SuggestedGeometryHints, SuggestedGeometryHint)
Q_ENUM_NS(SuggestedGeometryHint)

/// @brief Each main window supports 4 sidebars
enum class SideBarLocation {
    None = 0,
    North,
    East,
    West,
    South,
    Last
};

///@brief describes a type of button you can have in the title bar
enum class TitleBarButtonType {
    Close = 1,
    Float = 2,
    Minimize = 4,
    Maximize = 8,
    Normal = 16, // Restore from maximized state
    AutoHide = 32,
    UnautoHide = 64,
    AllTitleBarButtonTypes = Close | Float | Minimize | Maximize | Normal | AutoHide | UnautoHide
};
Q_ENUM_NS(TitleBarButtonType)
Q_DECLARE_FLAGS(TitleBarButtonTypes, TitleBarButtonType)

///@brief Enum describing the different drop indicator types
enum DropLocation {
    DropLocation_None = 0,
    DropLocation_Left = 1,
    DropLocation_Top = 2,
    DropLocation_Right = 4,
    DropLocation_Bottom = 8,
    DropLocation_Center = 16,
    DropLocation_OutterLeft = 32,
    DropLocation_OutterTop = 64,
    DropLocation_OutterRight = 128,
    DropLocation_OutterBottom = 256,
    DropLocation_Inner =
        DropLocation_Left | DropLocation_Right | DropLocation_Top | DropLocation_Bottom,
    DropLocation_Outter = DropLocation_OutterLeft | DropLocation_OutterRight
        | DropLocation_OutterTop | DropLocation_OutterBottom,
    DropLocation_Horizontal =
        DropLocation_Left | DropLocation_Right | DropLocation_OutterLeft | DropLocation_OutterRight,
    DropLocation_Vertical =
        DropLocation_Top | DropLocation_Bottom | DropLocation_OutterTop | DropLocation_OutterBottom
};
Q_ENUM_NS(DropLocation)

///@internal
enum CursorPosition {
    CursorPosition_Undefined = 0,
    CursorPosition_Left = 1,
    CursorPosition_Right = 2,
    CursorPosition_Top = 4,
    CursorPosition_Bottom = 8,
    CursorPosition_TopLeft = CursorPosition_Top | CursorPosition_Left,
    CursorPosition_TopRight = CursorPosition_Top | CursorPosition_Right,
    CursorPosition_BottomRight = CursorPosition_Bottom | CursorPosition_Right,
    CursorPosition_BottomLeft = CursorPosition_Bottom | CursorPosition_Left,
    CursorPosition_Horizontal = CursorPosition_Right | CursorPosition_Left,
    CursorPosition_Vertical = CursorPosition_Top | CursorPosition_Bottom,
    CursorPosition_All =
        CursorPosition_Left | CursorPosition_Right | CursorPosition_Top | CursorPosition_Bottom
};
Q_DECLARE_FLAGS(CursorPositions, CursorPosition)
Q_ENUM_NS(CursorPosition)


///@internal
enum FrameOption {
    FrameOption_None = 0,
    FrameOption_AlwaysShowsTabs = 1,
    FrameOption_IsCentralFrame = 2,
    FrameOption_IsOverlayed = 4,
    FrameOption_NonDockable = 8 ///> You can't DND and tab things into this Frame
};
Q_DECLARE_FLAGS(FrameOptions, FrameOption)
Q_ENUM_NS(FrameOptions)

///@internal
enum StackOption {
    StackOption_None = 0,
    StackOption_DocumentMode = 1 ///> Enables QTabWidget::documentMode()
};
Q_DECLARE_FLAGS(StackOptions, StackOption)
Q_ENUM_NS(StackOptions)

/// @internal
enum class FloatingWindowFlag {
    None = 0,
    FromGlobalConfig = 1, // KDDockWidgets::Config is used instead
    TitleBarHasMinimizeButton = 2,
    TitleBarHasMaximizeButton = 4,
    KeepAboveIfNotUtilityWindow = 8,
    NativeTitleBar = 16,
    HideTitleBarWhenTabsVisible = 32,
    AlwaysTitleBarWhenFloating = 64,
    DontUseParentForFloatingWindows = 128,
    UseQtWindow = 256,
    UseQtTool = 512,
    StartsMinimized = 1024
};
Q_DECLARE_FLAGS(FloatingWindowFlags, FloatingWindowFlag)

/// @internal
enum class WindowState {
    None = 0,
    Minimized = 1,
    Maximized = 2,
    FullScreen = 4
};
Q_DECLARE_FLAGS(WindowStates, WindowState)

/// @internal
enum class CloseReason {
    Unspecified = 0, /// probably programmatically
    TitleBarCloseButton = 1, /// User clicked titlebar close button
    Action = 2, /// User clicked menu with QAction
    MovedToSideBar = 4, /// User clicked the pin-button (or programmatically) (auto-hide/sidebar/pin-unpin functionality)
    OverlayCollapse = 8 /// Dock widget went from overlay to sidebar (auto-hide/sidebar/pin-unpin functionality)
};

/// @brief Initializes the desired frontend
/// This function should be called before using any docking.
/// Note that if you only built one frontend (by specifying for example -DKDDockWidgets_FRONTENDS=qtwidgets)
/// then KDDW will call this automatically.
void DOCKS_EXPORT initFrontend(FrontendType);

/// Returns the name of the logger used by KDDW
/// You can pass this name to spdlog::get() and change log level
DOCKS_EXPORT const char *spdlogLoggerName();

#ifdef KDDW_FRONTEND_QTWIDGETS

/// @brief Returns the first ancestor widget of the specified type T for the specified
/// Does not go across QWindow boundaries
/// If the specified widget is itself T, then it's returned
template<typename T>
inline T *findAncestor(QWidget *widget)
{
    QWidget *p = widget;
    while (p) {
        if (auto w = qobject_cast<T *>(p))
            return w;

        if (p->isWindow()) {
            // No need to check across window boundaries.
            // Main window is parent of floating windows.
            return nullptr;
        }

        p = p->parentWidget();
    }

    return nullptr;
}
#endif

template<typename T>
T bound(T minVal, T value, T maxVal)
{
    return std::max(minVal, std::min(value, maxVal));
}

inline bool fuzzyCompare(double a, double b, double epsilon = 0.0001)
{
    return std::abs(a - b) < epsilon;
}

} // end namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(KDDockWidgets::FrameOptions)
Q_DECLARE_METATYPE(KDDockWidgets::InitialVisibilityOption)
Q_DECLARE_METATYPE(KDDockWidgets::Location)

#define KDDW_DELETE_COPY_CTOR(NAME)         \
    NAME(const NAME &) = delete;            \
    NAME(const NAME &&) = delete;           \
    NAME &operator=(const NAME &) = delete; \
    NAME &operator=(const NAME &&) = delete;

#define KDDW_UNUSED(name) (( void )name);


#endif
