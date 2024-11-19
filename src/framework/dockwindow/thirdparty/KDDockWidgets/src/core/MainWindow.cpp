/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/


/**
 * @file
 * @brief The MainWindow base class that's shared between QtWidgets and QtQuick stack
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#include "MainWindow.h"
#include "MainWindow_p.h"
#include "kddockwidgets/KDDockWidgets.h"
#include "DockRegistry.h"
#include "Layout_p.h"
#include "core/MDILayout.h"
#include "core/DropArea.h"
#include "core/Utils_p.h"
#include "core/Logging_p.h"
#include "core/ScopedValueRollback_p.h"
#include "core/WidgetResizeHandler_p.h"
#include "core/ViewFactory.h"
#include "core/LayoutSaver_p.h"
#include "core/layouting/Item_p.h"
#include "Platform.h"
#include "core/DockWidget_p.h"
#include "core/Group.h"
#include "core/SideBar.h"
#include "kddockwidgets/core/views/MainWindowViewInterface.h"

#include <unordered_map>
#include <algorithm>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

static Layout *createLayout(MainWindow *mainWindow, MainWindowOptions options)
{
    if (options & MainWindowOption_MDI)
        return new MDILayout(mainWindow->view());

    return new DropArea(mainWindow->view(), options);
}

MainWindow::MainWindow(View *view, const QString &uniqueName, MainWindowOptions options)
    : Controller(ViewType::MainWindow, view)
    , d(new Private(this, uniqueName, options))
{
}

void MainWindow::init(const QString &name)
{
    d->init();
    d->m_layout = createLayout(this, d->m_options);

    d->m_persistentCentralDockWidget = d->createPersistentCentralDockWidget(d->name);

    setUniqueName(name);

    d->m_visibleWidgetCountConnection =
        d->m_layout->d_ptr()->visibleWidgetCountChanged.connect([this](int count) { d->groupCountChanged.emit(count); });
    view()->d->closeRequested.connect([this](CloseEvent *ev) { d->m_layout->onCloseEvent(ev); });

    d->m_resizeConnection = view()->d->resized.connect([this](Size size) {
        d->onResized(size);
    });
}

MainWindow::~MainWindow()
{
    DockRegistry::self()->unregisterMainWindow(this);
    delete d;
}

void MainWindow::addDockWidgetAsTab(Core::DockWidget *widget)
{
    assert(widget);
    KDDW_DEBUG("dock={}", ( void * )widget);

    if (!DockRegistry::self()->affinitiesMatch(d->affinities, widget->affinities())) {
        KDDW_ERROR("Refusing to dock widget with incompatible affinity. {} {}", widget->affinities(), affinities());
        return;
    }

    if (widget->options() & DockWidgetOption_NotDockable) {
        KDDW_ERROR("Refusing to dock non-dockable widget {}", ( void * )widget);
        return;
    }

    if (isMDI()) {
        // Not applicable to MDI
        return;
    }

    if (d->supportsPersistentCentralWidget()) {
        KDDW_ERROR("Not supported with MainWindowOption_HasCentralWidget."
                   "MainWindowOption_HasCentralWidget can only have 1 widget in the center.",
                   "Use MainWindowOption_HasCentralFrame instead, which is similar but supports "
                   "tabbing");
    } else if (d->supportsCentralFrame()) {
        dropArea()->centralGroup()->addTab(widget);
    } else {
        KDDW_ERROR("Not supported without MainWindowOption_HasCentralFrame");
    }
}

void MainWindow::addDockWidget(Core::DockWidget *dw, Location location,
                               Core::DockWidget *relativeTo, const InitialOption &option)
{
    if (dw->options() & DockWidgetOption_NotDockable) {
        KDDW_ERROR("Refusing to dock non-dockable widget dw={}", ( void * )dw);
        return;
    }

    if (isMDI()) {
        // Not applicable to MDI
        return;
    }

    dropArea()->addDockWidget(dw, location, relativeTo, option);
}

void MainWindow::addDockWidgetToSide(KDDockWidgets::Core::DockWidget *dockWidget,
                                     KDDockWidgets::Location location, const KDDockWidgets::InitialOption &initialOption)
{
    if (!dockWidget || location == Location_None || isMDI())
        return;

    if (!(d->m_options & MainWindowOption_HasCentralFrame)) {
        KDDW_ERROR("MainWindow::addDockWidgetToSide: A central group is required. Either MainWindowOption_HasCentralFrame or MainWindowOption_HasCentralWidget");
        return;
    }

    Group *group = dropArea()->centralGroup();
    if (!group || !group->layoutItem()) {
        // Doesn't happen
        KDDW_ERROR("MainWindow::addDockWidgetToSide: no group");
        return;
    }

    auto locToUse = [](Location loc) {
        switch (loc) {
        case Location_None:
            return Location_None;
        case Location_OnLeft:
            return Location_OnBottom;
        case Location_OnTop:
            return Location_OnRight;
        case Location_OnRight:
            return Location_OnBottom;
        case Location_OnBottom:
            return Location_OnRight;
        }

        return Location_None;
    };

    Core::Item *neighbor = group->layoutItem()->outermostNeighbor(location, /*visibleOnly=*/false);
    if (neighbor) {
        if (neighbor->isContainer()) {
            auto container = object_cast<ItemBoxContainer *>(neighbor);
            const auto children = container->childItems();
            if (children.isEmpty()) {
                // Doesn't happen
                KDDW_ERROR("MainWindow::addDockWidgetToSide: no children");
            } else {
                // There's an existing container with opposite orientation, add there but to end
                dropArea()->_addDockWidget(dockWidget, locToUse(location), children.last(), initialOption);
            }
        } else {
            dropArea()->_addDockWidget(dockWidget, locToUse(location), neighbor, initialOption);
        }
    } else {
        addDockWidget(dockWidget, location, nullptr, initialOption);
    }
}

QString MainWindow::uniqueName() const
{
    return d->name;
}

MainWindowOptions MainWindow::options() const
{
    return d->m_options;
}

DropArea *MainWindow::dropArea() const
{
    return d->m_layout->asDropArea();
}

DropArea *MainWindow::multiSplitter() const
{
    return dropArea();
}

Layout *MainWindow::layout() const
{
    return d->m_layout;
}

MDILayout *MainWindow::mdiLayout() const
{
    return d->m_layout->asMDILayout();
}

void MainWindow::setAffinities(const Vector<QString> &affinityNames)
{
    Vector<QString> affinities = affinityNames;
    affinities.removeAll(QString());

    if (d->affinities == affinities)
        return;

    if (!d->affinities.isEmpty()) {
        KDDW_ERROR("Affinity is already set, refusing to change."
                   "Submit a feature request with a good justification.");
        return;
    }

    d->affinities = affinities;
}

Vector<QString> MainWindow::affinities() const
{
    return d->affinities;
}

void MainWindow::layoutEqually()
{
    dropArea()->layoutEqually();
}

void MainWindow::layoutParentContainerEqually(Core::DockWidget *dockWidget)
{
    dropArea()->layoutParentContainerEqually(dockWidget);
}

CursorPositions MainWindow::Private::allowedResizeSides(SideBarLocation loc) const
{
    // When a sidebar is on top, you can only resize its bottom.
    // and so forth...

    switch (loc) {
    case SideBarLocation::North:
        return CursorPosition_Bottom;
    case SideBarLocation::East:
        return CursorPosition_Left;
    case SideBarLocation::West:
        return CursorPosition_Right;
    case SideBarLocation::South:
        return CursorPosition_Top;
    case SideBarLocation::None:
    case SideBarLocation::Last:
        return CursorPosition_Undefined;
    }

    return CursorPosition_Undefined;
}

Rect MainWindow::Private::rectForOverlay(Core::Group *group, SideBarLocation location) const
{
    Core::SideBar *sb = q->sideBar(location);
    if (!sb)
        return {};

    const Rect centralAreaGeo = q->centralAreaGeometry();
    const Margins centerWidgetMargins = q->centerWidgetMargins();

    Rect rect;
    const int margin = m_overlayMargin;
    switch (location) {
    case SideBarLocation::North:
    case SideBarLocation::South: {

        Core::SideBar *leftSideBar = q->sideBar(SideBarLocation::West);
        Core::SideBar *rightSideBar = q->sideBar(SideBarLocation::East);
        const int leftSideBarWidth =
            (leftSideBar && leftSideBar->isVisible()) ? leftSideBar->width() : 0;
        const int rightSideBarWidth =
            (rightSideBar && rightSideBar->isVisible()) ? rightSideBar->width() : 0;
        rect.setHeight(std::max(300, group->view()->minSize().height()));
        rect.setWidth(centralAreaGeo.width() - margin * 2 - leftSideBarWidth - rightSideBarWidth);
        rect.moveLeft(margin + leftSideBarWidth);
        if (location == SideBarLocation::South) {
            rect.moveTop(centralAreaGeo.bottom() - centerWidgetMargins.bottom() - rect.height()
                         - sb->height());
        } else {
            rect.moveTop(centralAreaGeo.y() + sb->height() + centerWidgetMargins.top());
        }
        break;
    }
    case SideBarLocation::West:
    case SideBarLocation::East: {
        Core::SideBar *topSideBar = q->sideBar(SideBarLocation::North);
        Core::SideBar *bottomSideBar = q->sideBar(SideBarLocation::South);
        const int topSideBarHeight =
            (topSideBar && topSideBar->isVisible()) ? topSideBar->height() : 0;
        const int bottomSideBarHeight =
            (bottomSideBar && bottomSideBar->isVisible()) ? bottomSideBar->height() : 0;
        rect.setWidth(std::max(300, group->view()->minSize().width()));
        rect.setHeight(centralAreaGeo.height() - topSideBarHeight - bottomSideBarHeight
                       - centerWidgetMargins.top() - centerWidgetMargins.bottom());
        rect.moveTop(sb->view()->mapTo(q->view(), Point(0, 0)).y() + topSideBarHeight - 1);
        if (location == SideBarLocation::East) {
            rect.moveLeft(centralAreaGeo.x() + centralAreaGeo.width() - rect.width() - sb->width()
                          - centerWidgetMargins.right() - margin);
        } else {
            rect.moveLeft(margin + centralAreaGeo.x() + centerWidgetMargins.left() + sb->width());
        }

        break;
    }
    case SideBarLocation::None:
    case SideBarLocation::Last:
        break;
    }

    return rect;
}

static SideBarLocation opposedSideBarLocationForBorder(Core::LayoutBorderLocation loc)
{
    switch (loc) {
    case Core::LayoutBorderLocation_North:
        return SideBarLocation::South;
    case Core::LayoutBorderLocation_East:
        return SideBarLocation::West;
    case Core::LayoutBorderLocation_West:
        return SideBarLocation::East;
    case Core::LayoutBorderLocation_South:
        return SideBarLocation::North;
    case Core::LayoutBorderLocation_All:
    case Core::LayoutBorderLocation_Verticals:
    case Core::LayoutBorderLocation_Horizontals:
    case Core::LayoutBorderLocation_None:
        break;
    }

    KDDW_ERROR("Unknown loc={}", loc);
    return SideBarLocation::None;
}

static SideBarLocation sideBarLocationForBorder(Core::LayoutBorderLocations loc)
{
    switch (loc) {
    case Core::LayoutBorderLocation_North:
        return SideBarLocation::North;
    case Core::LayoutBorderLocation_East:
        return SideBarLocation::East;
    case Core::LayoutBorderLocation_West:
        return SideBarLocation::West;
    case Core::LayoutBorderLocation_South:
        return SideBarLocation::South;
    case Core::LayoutBorderLocation_All:
    case Core::LayoutBorderLocation_Verticals:
    case Core::LayoutBorderLocation_Horizontals:
    case Core::LayoutBorderLocation_None:
        break;
    }

    return SideBarLocation::None;
}

SideBarLocation MainWindow::Private::preferredSideBar(Core::DockWidget *dw) const
{
    Group *group = dw->d->group();
    Core::Item *item = q->layout()->itemForGroup(group);
    if (!item) {
        KDDW_ERROR("No item for dock widget");
        return SideBarLocation::None;
    }

    const Core::LayoutBorderLocations borders = item->adjacentLayoutBorders();
    const double aspectRatio = group->width() / (std::max(1, group->height()) * 1.0);

    /// 1. It's touching all borders
    if (borders == Core::LayoutBorderLocation_All) {
        return aspectRatio > 1.0 ? SideBarLocation::South : SideBarLocation::East;
    }

    /// 2. It's touching 3 borders
    for (auto borderLoc :
         { Core::LayoutBorderLocation_North, Core::LayoutBorderLocation_East,
           Core::LayoutBorderLocation_West, Core::LayoutBorderLocation_South }) {
        if (borders == (Core::LayoutBorderLocation_All & ~borderLoc))
            return opposedSideBarLocationForBorder(borderLoc);
    }

    /// 3. It's touching left and right borders
    if ((borders & Core::LayoutBorderLocation_Verticals)
        == Core::LayoutBorderLocation_Verticals) {

        const int distanceToTop = group->geometry().y();
        const int distanceToBottom = q->layout()->layoutHeight() - group->geometry().bottom();
        return distanceToTop > distanceToBottom ? SideBarLocation::South : SideBarLocation::North;
    }

    /// 4. It's touching top and bottom borders
    if ((borders & Core::LayoutBorderLocation_Horizontals)
        == Core::LayoutBorderLocation_Horizontals) {

        const int distanceToLeft = group->geometry().x();
        const int distanceToRight = q->layout()->layoutWidth() - group->geometry().right();
        return distanceToLeft > distanceToRight ? SideBarLocation::East : SideBarLocation::West;
    }

    // 5. It's in a corner
    if (borders == (Core::LayoutBorderLocation_West | Core::LayoutBorderLocation_South)) {
        return aspectRatio > 1.0 ? SideBarLocation::South : SideBarLocation::West;
    } else if (borders
               == (Core::LayoutBorderLocation_East | Core::LayoutBorderLocation_South)) {
        return aspectRatio > 1.0 ? SideBarLocation::South : SideBarLocation::East;
    } else if (borders
               == (Core::LayoutBorderLocation_West | Core::LayoutBorderLocation_North)) {
        return aspectRatio > 1.0 ? SideBarLocation::North : SideBarLocation::West;
    } else if (borders
               == (Core::LayoutBorderLocation_East | Core::LayoutBorderLocation_North)) {
        return aspectRatio > 1.0 ? SideBarLocation::North : SideBarLocation::East;
    }


    {
        // 6. It's only touching 1 border
        SideBarLocation loc = sideBarLocationForBorder(borders);
        if (loc != SideBarLocation::None)
            return loc;
    }

    // It's not touching any border, use aspect ratio.
    return aspectRatio > 1.0 ? SideBarLocation::South : SideBarLocation::West;
}

void MainWindow::Private::updateOverlayGeometry(Size suggestedSize)
{
    if (!m_overlayedDockWidget)
        return;

    Core::SideBar *sb = q->sideBarForDockWidget(m_overlayedDockWidget);
    if (!sb) {
        KDDW_ERROR("Expected a sidebar");
        return;
    }

    const Rect defaultGeometry = rectForOverlay(m_overlayedDockWidget->d->group(), sb->location());
    Rect newGeometry = defaultGeometry;

    Core::Group *group = m_overlayedDockWidget->d->group();

    if (suggestedSize.isValid() && !suggestedSize.isEmpty()) {
        // Let's try to honour the suggested overlay size
        switch (sb->location()) {
        case SideBarLocation::North: {
            const int maxHeight = q->height() - group->pos().y() - 10; // gap
            newGeometry.setHeight(std::min(suggestedSize.height(), maxHeight));
            break;
        }
        case SideBarLocation::South: {
            const int maxHeight = sb->pos().y() - m_layout->view()->pos().y() - 10; // gap
            const int bottom = newGeometry.bottom();
            newGeometry.setHeight(std::min(suggestedSize.height(), maxHeight));
            newGeometry.moveBottom(bottom);
            break;
        }
        case SideBarLocation::East: {
            const int maxWidth = sb->pos().x() - m_layout->view()->pos().x() - 10; // gap
            const int right = newGeometry.right();
            newGeometry.setWidth(std::min(suggestedSize.width(), maxWidth));
            newGeometry.moveRight(right);
            break;
        }
        case SideBarLocation::West: {
            const int maxWidth = q->width() - group->pos().x() - 10; // gap
            newGeometry.setWidth(std::min(suggestedSize.width(), maxWidth));
            break;
        }
        case SideBarLocation::None:
        case SideBarLocation::Last:
            KDDW_ERROR("Unexpected sidebar value");
            break;
        }
    }

    m_overlayedDockWidget->d->group()->view()->setGeometry(newGeometry);
}

void MainWindow::Private::clearSideBars()
{
    for (auto loc : { SideBarLocation::North, SideBarLocation::South, SideBarLocation::East,
                      SideBarLocation::West }) {
        if (Core::SideBar *sb = q->sideBar(loc))
            sb->clear();
    }
}

Rect MainWindow::Private::windowGeometry() const
{
    /// @brief Returns the window geometry
    /// This is usually the same as the view's geometry()
    /// But fixes the following special cases:
    /// - QWidgets: Our MainWindow is embedded in another widget
    /// - QtQuick: Our MainWindow is QQuickItem

    if (Core::Window::Ptr window = q->view()->window())
        return window->geometry();

    return q->window()->geometry();
}

void MainWindow::moveToSideBar(Core::DockWidget *dw)
{
    moveToSideBar(dw, d->preferredSideBar(dw));
}

void MainWindow::moveToSideBar(Core::DockWidget *dw, SideBarLocation location)
{
    if (dw->isPersistentCentralDockWidget())
        return;

    if (Core::SideBar *sb = sideBar(location)) {
        ScopedValueRollback rollback(dw->d->m_isMovingToSideBar, true);
        CloseReasonSetter reason(CloseReason::MovedToSideBar);
        dw->forceClose();
        sb->addDockWidget(dw);
    } else {
        // Shouldn't happen
        KDDW_ERROR("Minimization supported, probably disabled in Config::self().flags()");
    }
}

void MainWindow::restoreFromSideBar(Core::DockWidget *dw)
{
    if (!dw)
        return;

    DockWidget::Private::UpdateActions updateActions(dw);

    // First un-overlay it, if it's overlayed
    if (dw == d->m_overlayedDockWidget)
        clearSideBarOverlay();

    Core::SideBar *sb = sideBarForDockWidget(dw);
    if (!sb) {
        // Doesn't happen
        KDDW_ERROR("Dock widget isn't in any sidebar");
        return;
    }

    sb->removeDockWidget(dw);
    dw->setFloating(false); // dock it
}

void MainWindow::overlayOnSideBar(Core::DockWidget *dw)
{
    if (!dw || dw->isPersistentCentralDockWidget())
        return;

    const Core::SideBar *sb = sideBarForDockWidget(dw);
    if (!sb) {
        KDDW_ERROR("You need to add the dock widget to the sidebar before you can overlay it");
        return;
    }

    if (d->m_overlayedDockWidget == dw) {
        // Already overlayed
        return;
    }

    // We only support one overlay at a time, remove any existing overlay
    clearSideBarOverlay();

    auto group = new Core::Group(nullptr, FrameOption_IsOverlayed);
    group->setParentView(view());
    d->m_overlayedDockWidget = dw;
    group->addTab(dw);
    d->updateOverlayGeometry(dw->d->lastPosition()->lastOverlayedGeometry(sb->location()).size());

    group->setAllowedResizeSides(d->allowedResizeSides(sb->location()));
    group->view()->show();

    dw->d->isOverlayedChanged.emit(true);
}

void MainWindow::toggleOverlayOnSideBar(Core::DockWidget *dw)
{
    const bool wasOverlayed = d->m_overlayedDockWidget == dw;
    clearSideBarOverlay(); // Because only 1 dock widget can be overlayed each time
    if (!wasOverlayed) {
        overlayOnSideBar(dw);
    }
}

void MainWindow::clearSideBarOverlay(bool deleteGroup)
{
    if (!d->m_overlayedDockWidget)
        return;

    auto overlayedDockWidget = d->m_overlayedDockWidget;
    d->m_overlayedDockWidget = nullptr;

    Core::Group *group = overlayedDockWidget->d->group();
    if (!group) { // prophylactic check
        return;
    }

    const SideBarLocation loc = overlayedDockWidget->sideBarLocation();
    overlayedDockWidget->d->lastPosition()->setLastOverlayedGeometry(loc, group->geometry());

    CloseReasonSetter reason(CloseReason::OverlayCollapse);
    group->unoverlay();

    if (deleteGroup) {
        // only update actions at the end
        DockWidget::Private::UpdateActions updateActions(overlayedDockWidget);

        overlayedDockWidget->setParent(nullptr);

        {
            ScopedValueRollback guard(overlayedDockWidget->d->m_removingFromOverlay, true);
            overlayedDockWidget->setParentView(nullptr);
            overlayedDockWidget->dptr()->setIsOpen(false);
        }

        overlayedDockWidget->d->isOverlayedChanged.emit(false);
        overlayedDockWidget = nullptr;
        delete group;
    } else {
        // No cleanup, just unset. When we drag the overlay it becomes a normal floating window
        // meaning we reuse Frame. Don't delete it.
        overlayedDockWidget->d->isOverlayedChanged.emit(false);
        overlayedDockWidget = nullptr;
    }
}

Core::SideBar *MainWindow::sideBarForDockWidget(const Core::DockWidget *dw) const
{
    for (auto loc : { SideBarLocation::North, SideBarLocation::South, SideBarLocation::East,
                      SideBarLocation::West }) {

        if (Core::SideBar *sb = sideBar(loc)) {
            if (sb->containsDockWidget(const_cast<Core::DockWidget *>(dw)))
                return sb;
        }
    }

    return nullptr;
}

Core::DockWidget *MainWindow::overlayedDockWidget() const
{
    return d->m_overlayedDockWidget;
}

bool MainWindow::sideBarIsVisible(SideBarLocation loc) const
{
    if (Core::SideBar *sb = sideBar(loc)) {
        return !sb->isEmpty(); // isVisible() is always true, but its height is 0 when empty.
    }

    return false;
}

bool MainWindow::anySideBarIsVisible() const
{
    for (auto loc : { SideBarLocation::North, SideBarLocation::South, SideBarLocation::East,
                      SideBarLocation::West }) {
        if (sideBarIsVisible(loc))
            return true;
    }

    return false;
}

bool MainWindow::isMDI() const
{
    return d->m_options & MainWindowOption_MDI;
}

bool MainWindow::closeDockWidgets(bool force)
{
    bool allClosed = true;

    const auto dockWidgets = d->m_layout->dockWidgets();
    for (Core::DockWidget *dw : dockWidgets) {
        Core::Group *group = dw->d->group();

        if (force) {
            dw->forceClose();
        } else {
            const bool closed = dw->view()->close();
            allClosed = allClosed && closed;
        }

        if (group->beingDeletedLater()) {
            // The dock widget was closed and this group is empty, delete immediately instead of
            // waiting. I'm not a big fan of deleting stuff later, as state becomes inconsistent

            // Empty groups are historically deleted later since they are triggered by mouse click
            // on the title bar, and the title bar is inside the group.
            // When doing it programmatically we can delete immediately.

            delete group;
        }
    }

    return allClosed;
}

void MainWindow::setUniqueName(const QString &uniqueName)
{
    if (uniqueName.isEmpty())
        return;

    if (d->name.isEmpty()) {
        d->name = uniqueName;
        d->uniqueNameChanged.emit();
        DockRegistry::self()->registerMainWindow(this);
    } else {
        KDDW_ERROR("Already has a name. {} {}", this->uniqueName(), uniqueName);
    }
}

bool MainWindow::deserialize(const LayoutSaver::MainWindow &mw)
{
    if (mw.options != options()) {
        KDDW_ERROR("Refusing to restore MainWindow with different options ; expected={}, has={}", int(mw.options), int(options()));
        return false;
    }

    if (d->affinities != mw.affinities) {
        KDDW_ERROR("Affinity name changed from {} to {}", d->affinities, mw.affinities);

        d->affinities = mw.affinities;
    }

    // Restore the SideBars
    d->clearSideBars();
    for (SideBarLocation loc : { SideBarLocation::North, SideBarLocation::East,
                                 SideBarLocation::West, SideBarLocation::South }) {
        Core::SideBar *sb = sideBar(loc);
        if (!sb)
            continue;

        const Vector<QString> dockWidgets = mw.dockWidgetsForSideBar(loc);
        for (const QString &uniqueName : dockWidgets) {

            Core::DockWidget *dw = DockRegistry::self()->dockByName(
                uniqueName, DockRegistry::DockByNameFlag::CreateIfNotFound);
            if (!dw) {
                KDDW_ERROR("Could not find dock widget {} . Won't restore it to sidebar", uniqueName);
                continue;
            }

            sb->addDockWidget(dw);
        }
    }

    const bool success = layout()->deserialize(mw.multiSplitterLayout);

    // Commented-out for now, we don't want to restore the popup/overlay. popups are perishable
    // if (!mw.overlayedDockWidget.isEmpty())
    //    overlayOnSideBar(DockRegistry::self()->dockByName(mw.overlayedDockWidget));

    return success;
}

LayoutSaver::MainWindow MainWindow::serialize() const
{
    LayoutSaver::MainWindow m;

    Window::Ptr window = view()->window();

    m.options = options();
    m.geometry = d->windowGeometry();
    m.normalGeometry = view()->normalGeometry();
    m.isVisible = isVisible();
    m.uniqueName = uniqueName();
    m.screenIndex = Platform::instance()->screenNumberForView(view());
    m.screenSize = Platform::instance()->screenSizeFor(view());
    m.multiSplitterLayout = layout()->serialize();
    m.affinities = d->affinities;
    m.windowState = window ? window->windowState() : WindowState::None;

    for (SideBarLocation loc : { SideBarLocation::North, SideBarLocation::East,
                                 SideBarLocation::West, SideBarLocation::South }) {
        if (Core::SideBar *sb = sideBar(loc)) {
            const Vector<QString> dockwidgets = sb->serialize();
            if (!dockwidgets.isEmpty())
                m.dockWidgetsPerSideBar[loc] = dockwidgets;
        }
    }

    return m;
}

void MainWindow::setPersistentCentralView(std::shared_ptr<View> widget)
{
    if (!d->supportsPersistentCentralWidget()) {
        KDDW_ERROR("MainWindow::setPersistentCentralWidget() requires "
                   "MainWindowOption_HasCentralWidget");
        return;
    }

    if (auto dw = d->m_persistentCentralDockWidget) {
        dw->setGuestView(widget);
    } else {
        KDDW_ERROR("Unexpected null central dock widget");
    }
}

std::shared_ptr<View> MainWindow::persistentCentralView() const
{
    if (auto dw = d->m_persistentCentralDockWidget)
        return dw->guestView();

    return {};
}

void MainWindow::setContentsMargins(int left, int top, int right, int bottom)
{
    auto v = dynamic_cast<Core::MainWindowViewInterface *>(view());
    v->setContentsMargins(left, top, right, bottom);
}

Margins MainWindow::centerWidgetMargins() const
{
    auto v = dynamic_cast<Core::MainWindowViewInterface *>(view());
    return v->centerWidgetMargins();
}

Core::SideBar *MainWindow::sideBar(SideBarLocation loc) const
{
    auto it = d->m_sideBars.find(loc);
    return it == d->m_sideBars.cend() ? nullptr : it->second;
}

Rect MainWindow::centralAreaGeometry() const
{
    auto v = dynamic_cast<Core::MainWindowViewInterface *>(view());
    return v->centralAreaGeometry();
}

int MainWindow::overlayMargin() const
{
    return d->m_overlayMargin;
}

void MainWindow::setOverlayMargin(int margin)
{
    if (margin == d->m_overlayMargin)
        return;

    d->m_overlayMargin = margin;
    d->overlayMarginChanged.emit(margin);
}
