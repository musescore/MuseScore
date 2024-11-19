/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KDDOCKWIDGETS_ViewFactory_H
#define KDDOCKWIDGETS_ViewFactory_H

#include "View.h"
#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"


QT_BEGIN_NAMESPACE
class QAbstractButton;
QT_END_NAMESPACE

namespace KDDockWidgets {

namespace Core {

class Action;
class ClassicIndicatorWindowViewInterface;
class DropIndicatorOverlay;
class DropArea;
class Separator;
class TabBar;
class SideBar;
class FloatingWindow;
class MainWindow;
class ClassicDropIndicatorOverlay;
class SegmentedDropIndicatorOverlay;
class TitleBar;

/**
 * @brief A factory class for allowing the user to customize some internal views.
 * This is optional, and if not provided, a default one will be used.
 *
 * You should however not derive directly from ViewFactory, and instead, derive from
 * QtWidgets::ViewFactory or QtQuick::ViewFactory.
 *
 * Sub-classing ViewFactory allows for fine-grained customization and
 * styling of some non-public widgets, such as titlebars, dock widget group and
 * tab widgets.
 *
 * To set your own factory see Config::setViewFactory()
 *
 * @sa Config::setViewFactory()
 */
class DOCKS_EXPORT ViewFactory : public Core::Object
{
    Q_OBJECT
public:
    ViewFactory() = default;

    ///@brief Destructor.
    /// Don't delete ViewFactory directly, it's owned by the framework.
    virtual ~ViewFactory();

    /// @brief Creates a dock widget. This is only used by MainWindow's persistent widget feature.
    /// In all other cases users will instantiate DockWidget directly
    virtual View *createDockWidget(const QString &uniqueName, DockWidgetOptions options = {},
                                   LayoutSaverOptions layoutSaverOptions = {},
                                   Qt::WindowFlags windowFlags = {}) const = 0;


    ///@brief Called by the framework to create a Frame view
    ///       Override to provide your own Frame sub-class. A group is the
    ///       widget that holds the titlebar and tab-widget which holds the
    ///       DockWidgets.
    ///@param parent just forward to Frame's constructor
    virtual View *createGroup(Core::Group *, View *parent = nullptr) const = 0;

    ///@brief Called by the framework to create a TitleBar view
    ///       Override to provide your own TitleBar sub-class.
    ///       Just forward the @p controller and @p parent arguments to the TitleBar view ctor
    virtual View *createTitleBar(Core::TitleBar *controller, View *parent) const = 0;

    ///@brief Called by the framework to create a Stack view
    ///       Override to provide your own Stack sub-class.
    ///@param parent Just forward to Stack's constructor.
    virtual View *createStack(Core::Stack *stack, View *parent) const = 0;

    ///@brief Called by the framework to create a TabBar view
    ///       Override to provide your own TabBar sub-class.
    ///@param parent Just forward to TabBar's's constructor.
    virtual View *createTabBar(Core::TabBar *tabBar, View *parent = nullptr) const = 0;

    ///@brief Called by the framework to create a Separator view
    ///       Override to provide your own Separator sub-class. The Separator allows
    ///       the user to resize nested dock widgets.
    ///@param parent Just forward to Separator's constructor.
    virtual View *createSeparator(Core::Separator *, View *parent = nullptr) const = 0;

    ///@brief Called by the framework to create a FloatingWindow view
    ///       Override to provide your own FloatingWindow sub-class.
    ///@param parent Just forward to FloatingWindow's constructor.
    virtual View *createFloatingWindow(Core::FloatingWindow *controller,
                                       Core::MainWindow *parent = nullptr,
                                       Qt::WindowFlags windowFlags = {}) const = 0;


    /// @brief Creates the window that will show the actual drop indicators. They need a higher
    /// z-order, so this is actually a separate window, not parented to the main window
    virtual Core::ClassicIndicatorWindowViewInterface *
    createClassicIndicatorWindow(Core::ClassicDropIndicatorOverlay *, Core::View *parent = nullptr) const = 0;

    /// @brief Creates the view that will parent the segmented drop indicators
    virtual View *
    createSegmentedDropIndicatorOverlayView(Core::SegmentedDropIndicatorOverlay *controller,
                                            View *parent) const = 0;

    /// @brief Called by the framework to create a DropArea view
    virtual View *createDropArea(Core::DropArea *, View *parent) const = 0;

    /// @brief Called by the framework to create a MDI Layout view
    virtual View *createMDILayout(Core::MDILayout *, View *parent) const = 0;

    ///@brief Called by the framework to create a RubberBand view to show as drop zone
    virtual View *createRubberBand(View *parent) const = 0;

    ///@brief Called by the framework to create a SideBar view
    ///@param loc The side-bar location without the main window. Just forward into your SideBar
    /// sub-class ctor.
    ///@param parent The MainWindow. Just forward into your SideBar sub-class ctor.
    virtual View *createSideBar(Core::SideBar *, View *parent) const = 0;

    /// @brief Creates a QAction if QtWidgets, or an equivalent fallback if QtQuick/Flutter
    /// Not needed to be overridden by users.
    virtual KDDockWidgets::Core::Action *createAction(Core::DockWidget *, const char *debugName) const = 0;

    /// @brief Returns the icon to be used with the specified @p type
    /// @param dpr the device pixel ratio of the button
    virtual Icon iconForButtonType(TitleBarButtonType type, double dpr) const = 0;

    /// @brief The path to a folder containing the classic_indicator png files
    virtual QString classicIndicatorsPath() const;

    /// @ The drop indicator type
    static DropIndicatorType s_dropIndicatorType;

private:
    KDDW_DELETE_COPY_CTOR(ViewFactory)
};

}

}

#endif
