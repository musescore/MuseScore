/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "dockwindow.h"

#include "thirdparty/KDDockWidgets/src/DockWidgetQuick.h"
#include "thirdparty/KDDockWidgets/src/LayoutSaver.h"
#include "thirdparty/KDDockWidgets/src/private/quick/MainWindowQuick_p.h"
#include "thirdparty/KDDockWidgets/src/private/DockRegistry_p.h"

#include "dockcentralview.h"
#include "dockpageview.h"
#include "dockpanelview.h"
#include "dockstatusbarview.h"
#include "docktoolbarview.h"
#include "dockingholderview.h"
#include "dockwindow.h"

#include "log.h"

using namespace mu::dock;
using namespace mu::async;

static constexpr double MAX_DISTANCE_TO_HOLDER = 50;

DockWindow::DockWindow(QQuickItem* parent)
    : QQuickItem(parent),
    m_toolBars(this),
    m_pages(this)
{
}

DockWindow::~DockWindow()
{
    dockWindowProvider()->deinit();
}

void DockWindow::componentComplete()
{
    TRACEFUNC;

    QQuickItem::componentComplete();

    m_mainWindow = new KDDockWidgets::MainWindowQuick("mainWindow",
                                                      KDDockWidgets::MainWindowOption_None,
                                                      this);

    connect(qApp, &QCoreApplication::aboutToQuit, this, &DockWindow::onQuit);

    uiConfiguration()->windowGeometryChanged().onNotify(this, [this]() {
        if (!m_quiting) {
            resetWindowState();
        }
    });

    dockWindowProvider()->init(this);

    startupScenario()->run();
}

void DockWindow::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    if (!m_currentPage) {
        QQuickItem::geometryChanged(newGeometry, oldGeometry);
        return;
    }

    //! NOTE: it is important to reset the current minimum width for all top-level toolbars
    //! Otherwise, the window content can be displaced after LayoutWidget::onResize(QSize newSize)
    //! due to lack of free space
    QList<DockToolBarView*> topToolBars = topLevelToolBars(m_currentPage);
    for (DockToolBarView* toolBar : topToolBars) {
        toolBar->setMinimumWidth(toolBar->contentWidth());
    }

    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    alignToolBars(m_currentPage);
}

void DockWindow::onQuit()
{
    TRACEFUNC;

    m_quiting = true;

    saveGeometry();

    IF_ASSERT_FAILED(m_currentPage) {
        return;
    }

    savePageState(m_currentPage->objectName());
}

QString DockWindow::currentPageUri() const
{
    return m_currentPage ? m_currentPage->uri() : QString();
}

QQmlListProperty<mu::dock::DockToolBarView> DockWindow::toolBarsProperty()
{
    return m_toolBars.property();
}

QQmlListProperty<mu::dock::DockPageView> DockWindow::pagesProperty()
{
    return m_pages.property();
}

void DockWindow::loadPage(const QString& uri)
{
    TRACEFUNC;

    if (currentPageUri() == uri) {
        return;
    }

    bool isFirstOpening = (m_currentPage == nullptr);
    if (isFirstOpening) {
        restoreGeometry();
    }

    DockPageView* newPage = pageByUri(uri);
    IF_ASSERT_FAILED(newPage) {
        return;
    }

    if (m_currentPage) {
        savePageState(m_currentPage->objectName());
        KDDockWidgets::DockRegistry::self()->clear();
    }

    loadPageContent(newPage);
    restorePageState(newPage->objectName());
    initDocks(newPage);

    QStringList allDockNames;

    for (DockBase* dock : newPage->allDocks()) {
        allDockNames << dock->objectName();
    }

    m_currentPage = newPage;
    emit currentPageUriChanged(uri);

    m_docksOpenStatusChanged.send(allDockNames);
}

bool DockWindow::isDockOpen(const QString& dockName) const
{
    return m_currentPage && m_currentPage->isDockOpen(dockName);
}

void DockWindow::toggleDock(const QString& dockName)
{
    if (m_currentPage) {
        m_currentPage->toggleDock(dockName);
        m_docksOpenStatusChanged.send({ dockName });
    }
}

void DockWindow::setDockOpen(const QString& dockName, bool open)
{
    if (m_currentPage) {
        m_currentPage->setDockOpen(dockName, open);
        m_docksOpenStatusChanged.send({ dockName });
    }
}

Channel<QStringList> DockWindow::docksOpenStatusChanged() const
{
    return m_docksOpenStatusChanged;
}

bool DockWindow::isDockFloating(const QString& dockName) const
{
    return m_currentPage && m_currentPage->isDockFloating(dockName);
}

void DockWindow::toggleDockFloating(const QString& dockName)
{
    if (m_currentPage) {
        m_currentPage->toggleDockFloating(dockName);
    }
}

DropDestination DockWindow::hover(const QString& draggedDockName, const QPoint& globalPos)
{
    DockBase* draggedDock = m_currentPage ? m_currentPage->dockByName(draggedDockName) : nullptr;

    if (!draggedDock) {
        return DropDestination();
    }

    QPoint hoveredLocalPos = m_mainWindow->mapFromGlobal(globalPos);
    DropDestination dropDestination = resolveDropDestination(draggedDock, hoveredLocalPos);

    if (auto toolBar = dynamic_cast<DockToolBarView*>(draggedDock)) {
        updateToolBarOrientation(toolBar, dropDestination);
    }

    setCurrentDropDestination(draggedDock, dropDestination);

    return m_currentDropDestination;
}

void DockWindow::endHover()
{
    if (!m_currentDropDestination.isValid()) {
        return;
    }

    m_currentDropDestination.dock->hideHighlighting();

    if (m_currentDropDestination.dock->type() == DockType::DockingHolder) {
        m_currentDropDestination.dock->close();
    }

    m_currentDropDestination.clear();
}

DockingHolderView* DockWindow::mainToolBarDockingHolder() const
{
    return m_mainToolBarDockingHolder;
}

void DockWindow::setMainToolBarDockingHolder(DockingHolderView* mainToolBarDockingHolder)
{
    if (m_mainToolBarDockingHolder == mainToolBarDockingHolder) {
        return;
    }

    m_mainToolBarDockingHolder = mainToolBarDockingHolder;
    emit mainToolBarDockingHolderChanged(m_mainToolBarDockingHolder);
}

void DockWindow::loadPageContent(const DockPageView* page)
{
    TRACEFUNC;

    addDock(page->centralDock());

    loadSideDocks(page, DockType::Panel);
    loadSideDocks(page, DockType::ToolBar);

    if (page->statusBar()) {
        addDock(page->statusBar(), KDDockWidgets::Location_OnBottom);
    }

    loadTopLevelToolBars(page);

    addDock(m_mainToolBarDockingHolder, KDDockWidgets::Location_OnTop);

    unitePanelsToTabs(page);
}

void DockWindow::loadTopLevelToolBars(const DockPageView* page)
{
    QList<DockToolBarView*> allToolBars = m_toolBars.list();
    allToolBars << page->mainToolBars();

    DockToolBarView* prevToolBar = nullptr;

    for (DockToolBarView* toolBar : allToolBars) {
        auto location = prevToolBar ? KDDockWidgets::Location_OnRight : KDDockWidgets::Location_OnTop;
        addDock(toolBar, location, prevToolBar);
        prevToolBar = toolBar;
    }
}

void DockWindow::unitePanelsToTabs(const DockPageView* page)
{
    for (DockPanelView* panel : page->panels()) {
        DockPanelView* tab = panel->tabifyPanel();

        if (tab && tab->isVisible()) {
            panel->addPanelAsTab(tab);
            panel->setCurrentTabIndex(0);
        }
    }
}

void DockWindow::loadSideDocks(const DockPageView* page, DockType type)
{
    static const QList<Location> possibleLocations {
        Location::Left,
        Location::Right,
        Location::Top,
        Location::Bottom
    };

    QMap<Location, QList<DockBase*> > sideDocks;

    for (Location location : possibleLocations) {
        if (auto holder = page->holder(type, location)) {
            sideDocks[location] << holder;
        }
    }

    if (type == DockType::ToolBar) {
        for (DockToolBarView* toolBar : page->toolBars()) {
            sideDocks[toolBar->location()] << toolBar;
        }
    } else if (type == DockType::Panel) {
        for (DockPanelView* panel : page->panels()) {
            sideDocks[panel->location()] << panel;
        }
    }

    QList<DockBase*> leftSideDocks = sideDocks[Location::Left];
    for (int i = leftSideDocks.size() - 1; i >= 0; --i) {
        addDock(leftSideDocks[i], KDDockWidgets::Location_OnLeft);
    }

    QList<DockBase*> rightSideDocks = sideDocks[Location::Right];
    for (int i = 0; i < rightSideDocks.size(); ++i) {
        addDock(rightSideDocks[i], KDDockWidgets::Location_OnRight);
    }

    QList<DockBase*> bottomSideDocks = sideDocks[Location::Bottom];
    for (int i = 0; i < bottomSideDocks.size(); ++i) {
        addDock(bottomSideDocks[i], KDDockWidgets::Location_OnBottom);
    }

    QList<DockBase*> topSideDocks = sideDocks[Location::Top];
    for (int i = topSideDocks.size() - 1; i >= 0; --i) {
        addDock(topSideDocks[i], KDDockWidgets::Location_OnTop);
    }
}

void DockWindow::alignToolBars(const DockPageView* page)
{
    QList<DockToolBarView*> topToolBars = topLevelToolBars(page);

    DockToolBarView* lastLeftToolBar = nullptr;
    DockToolBarView* lastCentralToolBar = nullptr;

    int leftToolBarsWidth = 0;
    int centralToolBarsWidth = 0;
    int rightToolBarsWidth = 0;

    for (DockToolBarView* toolBar : topToolBars) {
        if (toolBar->floating() || !toolBar->isVisible()) {
            continue;
        }

        switch (static_cast<DockToolBarAlignment::Type>(toolBar->alignment())) {
        case DockToolBarAlignment::Left:
            lastLeftToolBar = toolBar;
            leftToolBarsWidth += toolBar->contentWidth();
            break;
        case DockToolBarAlignment::Center:
            lastCentralToolBar = toolBar;
            centralToolBarsWidth += toolBar->contentWidth();
            break;
        case DockToolBarAlignment::Right:
            rightToolBarsWidth += toolBar->contentWidth();
            break;
        }
    }

    if (!lastLeftToolBar || !lastCentralToolBar) {
        return;
    }

    int deltaForLastLeftToolbar = (width() - centralToolBarsWidth) / 2 - leftToolBarsWidth;
    int deltaForLastCentralToolBar = (width() - centralToolBarsWidth) / 2 - rightToolBarsWidth;

    deltaForLastLeftToolbar = std::max(deltaForLastLeftToolbar, 0);
    deltaForLastCentralToolBar = std::max(deltaForLastCentralToolBar, 0);

    int freeSpace = width() - (leftToolBarsWidth + centralToolBarsWidth + rightToolBarsWidth);

    if (deltaForLastLeftToolbar + deltaForLastCentralToolBar > freeSpace) {
        deltaForLastLeftToolbar = freeSpace;
        deltaForLastCentralToolBar = 0;
    }

    lastLeftToolBar->setMinimumWidth(lastLeftToolBar->contentWidth() + deltaForLastLeftToolbar);
    lastCentralToolBar->setMinimumWidth(lastCentralToolBar->contentWidth() + deltaForLastCentralToolBar);
}

void DockWindow::addDock(DockBase* dock, KDDockWidgets::Location location, const DockBase* relativeTo)
{
    IF_ASSERT_FAILED(dock) {
        return;
    }

    KDDockWidgets::DockWidgetBase* relativeDock = relativeTo ? relativeTo->dockWidget() : nullptr;

    auto visibilityOption = dock->isVisible() ? KDDockWidgets::InitialVisibilityOption::StartVisible
                            : KDDockWidgets::InitialVisibilityOption::StartHidden;

    KDDockWidgets::InitialOption options(visibilityOption, dock->preferredSize());

    m_mainWindow->addDockWidget(dock->dockWidget(), location, relativeDock, options);
}

DockPageView* DockWindow::pageByUri(const QString& uri) const
{
    for (DockPageView* page : m_pages.list()) {
        if (page->uri() == uri) {
            return page;
        }
    }

    return nullptr;
}

void DockWindow::saveGeometry()
{
    TRACEFUNC;

    /// NOTE: The state of all dock widgets is also saved here,
    /// since the library does not provide the ability to save
    /// and restore only the application geometry.
    /// Therefore, for correct operation after saving or restoring geometry,
    /// it is necessary to apply the appropriate method for the state.
    uiConfiguration()->setWindowGeometry(windowState());
}

void DockWindow::restoreGeometry()
{
    TRACEFUNC;

    if (!restoreLayout(uiConfiguration()->windowGeometry())) {
        LOGE() << "Could not restore the window geometry!";
    }
}

void DockWindow::savePageState(const QString& pageName)
{
    TRACEFUNC;

    uiConfiguration()->setPageState(pageName, windowState());
}

void DockWindow::restorePageState(const QString& pageName)
{
    TRACEFUNC;

    /// NOTE: Do not restore geometry
    bool ok = restoreLayout(uiConfiguration()->pageState(pageName), KDDockWidgets::RestoreOption::RestoreOption_RelativeToMainWindow);
    if (!ok) {
        LOGE() << "Could not restore the state of " << pageName << "!";
    }
}

bool DockWindow::restoreLayout(const QByteArray& layout, KDDockWidgets::RestoreOptions)
{
    if (layout.isEmpty()) {
        return true;
    }

    LOGI() << "TODO: restoring of layout is temporary disabled because it troubles";
    //KDDockWidgets::LayoutSaver layoutSaver(option);
    //return layoutSaver.restoreLayout(state);

    return true;
}

QByteArray DockWindow::windowState() const
{
    TRACEFUNC;

    KDDockWidgets::LayoutSaver layoutSaver;
    return layoutSaver.serializeLayout();
}

void DockWindow::resetWindowState()
{
    QString currentPageUriBackup = currentPageUri();

    /// NOTE: for reset geometry
    m_currentPage = nullptr;

    loadPage(currentPageUriBackup);
}

void DockWindow::initDocks(DockPageView* page)
{
    for (DockToolBarView* toolbar : m_toolBars.list()) {
        toolbar->init();
    }

    m_mainToolBarDockingHolder->init();

    if (page) {
        page->init();
    }

    alignToolBars(page);

    for (DockToolBarView* toolbar : page->mainToolBars()) {
        connect(toolbar, &DockToolBarView::floatingChanged, this, [this, page]() {
            alignToolBars(page);
        }, Qt::UniqueConnection);
    }
}

QList<DockToolBarView*> DockWindow::topLevelToolBars(const DockPageView* page) const
{
    QList<DockToolBarView*> toolBars = m_toolBars.list();

    if (page) {
        toolBars << page->mainToolBars();
    }

    return toolBars;
}

bool DockWindow::isMouseOverDock(const QPoint& mouseLocalPos, const DockBase* dock) const
{
    QRect geometry = dock ? dock->frameGeometry() : QRect();
    return geometry.contains(mouseLocalPos);
}

void DockWindow::updateToolBarOrientation(DockToolBarView* draggedToolBar, const DropDestination& dropDestination)
{
    IF_ASSERT_FAILED(draggedToolBar) {
        return;
    }

    framework::Orientation orientation = framework::Orientation::Horizontal;

    if (!dropDestination.isValid()) {
        draggedToolBar->setOrientation(static_cast<Qt::Orientation>(orientation));
        return;
    }

    switch (dropDestination.dock->location()) {
    case Location::Left:
    case Location::Right:
        orientation = framework::Orientation::Vertical;
        break;
    case Location::Top:
    case Location::Bottom:
        orientation = framework::Orientation::Horizontal;
        break;
    case Location::Center:
    case Location::Undefined:
        break;
    }

    draggedToolBar->setOrientation(static_cast<Qt::Orientation>(orientation));
}

void DockWindow::setCurrentDropDestination(const DockBase* draggedDock, const DropDestination& dropDestination)
{
    if (m_currentDropDestination == dropDestination) {
        return;
    }

    endHover();

    m_currentDropDestination = dropDestination;

    if (!m_currentDropDestination.isValid()) {
        return;
    }

    auto showHighlighting = [this]() {
        QRect highlightingRect = resolveHighlightingRect(m_currentDropDestination);
        m_currentDropDestination.dock->showHighlighting(highlightingRect);
    };

    if (m_currentDropDestination.dock->type() != DockType::DockingHolder) {
        showHighlighting();
        return;
    }

    switch (m_currentDropDestination.dock->location()) {
    case Location::Left:
    case Location::Right:
        m_currentDropDestination.dock->setMinimumWidth(draggedDock->minimumWidth());
        break;
    case Location::Top:
    case Location::Bottom:
        m_currentDropDestination.dock->setMinimumHeight(draggedDock->minimumHeight());
        break;
    case Location::Center:
    case Location::Undefined:
        break;
    }

    m_currentDropDestination.dock->open();
    showHighlighting();
    m_currentDropDestination.dock->init();
}

DropDestination DockWindow::resolveDropDestination(const DockBase* draggedDock, const QPoint& localPos) const
{
    if (draggedDock->type() == DockType::Panel) {
        DropDestination destination;

        destination.dock = resolveTabifyPanel(dynamic_cast<const DockPanelView*>(draggedDock), localPos);
        destination.dropLocation = resolveDropLocation(destination.dock, localPos);

        if (destination.isValid()) {
            return destination;
        }
    }

    const DockingHolderView* holder = resolveDockingHolder(draggedDock->type(), localPos);
    QList<DropDestination> destinations = draggedDock->dropDestinations();

    for (const DropDestination& destination : destinations) {
        if (holder == destination.dock) {
            return destination;
        }

        if (isMouseOverDock(localPos, destination.dock)) {
            return destination;
        }
    }

    return DropDestination();
}

DockingHolderView* DockWindow::resolveDockingHolder(DockType draggedDockType, const QPoint& localPos) const
{
    if (!m_currentPage) {
        return nullptr;
    }

    if (!m_mainWindow->contains(localPos)) {
        return nullptr;
    }

    QRect centralGeometry = m_currentPage->centralDock()->frameGeometry();

    // TODO: Need to take any panels docked at top into account
    if (localPos.y() <= centralGeometry.top() + MAX_DISTANCE_TO_HOLDER) {
        return m_currentPage->holder(draggedDockType, Location::Top);
    }

    if (localPos.y() >= centralGeometry.bottom() - MAX_DISTANCE_TO_HOLDER) {
        return m_currentPage->holder(draggedDockType, Location::Bottom);
    }

    if (localPos.x() <= MAX_DISTANCE_TO_HOLDER) {
        return m_currentPage->holder(draggedDockType, Location::Left);
    }

    if (localPos.x() >= m_mainWindow->rect().right() - MAX_DISTANCE_TO_HOLDER) {
        return m_currentPage->holder(draggedDockType, Location::Right);
    }

    return nullptr;
}

DockPanelView* DockWindow::resolveTabifyPanel(const DockPanelView* panel, const QPoint& localPos) const
{
    QList<DockPanelView*> tabs = m_currentPage->possibleTabs(panel);

    for (DockPanelView* tab : tabs) {
        if (isMouseOverDock(localPos, tab)) {
            return tab;
        }
    }

    return nullptr;
}

Location DockWindow::resolveDropLocation(const DockBase* hoveredDock, const QPoint& localPos) const
{
    if (!hoveredDock) {
        return Location::Undefined;
    }

    QRect geometry = hoveredDock->frameGeometry();
    Location dockLocation = hoveredDock->location();

    qreal frameEnd = geometry.bottom();
    qreal mousePos = localPos.y();
    Location beginDropLocation = Location::Top;
    Location endDropLocation = Location::Bottom;

    if (dockLocation == Location::Top || dockLocation == Location::Bottom) {
        mousePos = localPos.x();
        frameEnd = geometry.right();
        beginDropLocation = Location::Left;
        endDropLocation = Location::Right;
    }

    if (mousePos <= frameEnd / 3) {
        return beginDropLocation;
    }

    if (mousePos <= frameEnd / 1.5) {
        return Location::Center;
    }

    if (mousePos <= frameEnd) {
        return endDropLocation;
    }

    return Location::Undefined;
}

QRect DockWindow::resolveHighlightingRect(const DropDestination& destination) const
{
    if (!destination.isValid()) {
        return QRect();
    }

    QRect frameGeometry = destination.dock->frameGeometry();
    int frameWidth = frameGeometry.width();
    int frameHeight = frameGeometry.height();
    QRect fullFrameHighlightingRect = QRect(0, 0, frameWidth, frameHeight);

    if (destination.dock->type() == DockType::DockingHolder) {
        return fullFrameHighlightingRect;
    }

    switch (destination.dropLocation) {
    case Location::Top:
        return QRect(0, 0, frameWidth, frameHeight / 2);
    case Location::Bottom:
        return QRect(0, frameHeight / 2, frameWidth, frameHeight / 2);
    case Location::Left:
        return QRect(0, 0, frameWidth / 2, frameHeight);
    case Location::Right:
        return QRect(frameWidth / 2, 0, frameWidth / 2, frameHeight);
    case Location::Center:
        return fullFrameHighlightingRect;
    case Location::Undefined:
        break;
    }

    return QRect();
}
