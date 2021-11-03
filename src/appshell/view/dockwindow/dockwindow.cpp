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
    const DockPageView* page = currentPage();
    if (!page) {
        QQuickItem::geometryChanged(newGeometry, oldGeometry);
        return;
    }

    //! NOTE: it is important to reset the current minimum width for all top-level toolbars
    //! Otherwise, the window content can be displaced after LayoutWidget::onResize(QSize newSize)
    //! due to lack of free space
    QList<DockToolBarView*> topToolBars = topLevelToolBars(page);
    for (DockToolBarView* toolBar : topToolBars) {
        toolBar->setMinimumWidth(toolBar->contentWidth());
    }

    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    alignToolBars(page);
}

void DockWindow::onQuit()
{
    TRACEFUNC;

    m_quiting = true;

    saveGeometry();

    const DockPageView* currPage = currentPage();
    IF_ASSERT_FAILED(currPage) {
        return;
    }

    savePageState(currPage->objectName());
}

QString DockWindow::currentPageUri() const
{
    return m_currentPageUri;
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

    if (m_currentPageUri == uri) {
        return;
    }

    bool isFirstOpening = m_currentPageUri.isEmpty();
    if (isFirstOpening) {
        restoreGeometry();
    }

    DockPageView* newPage = pageByUri(uri);
    IF_ASSERT_FAILED(newPage) {
        return;
    }

    DockPageView* currentPage = this->currentPage();
    if (currentPage) {
        savePageState(currentPage->objectName());
        KDDockWidgets::DockRegistry::self()->clear();
    }

    loadPageContent(newPage);
    restorePageState(newPage->objectName());
    initDocks(newPage);

    QStringList allDockNames;

    for (DockBase* dock : newPage->allDocks()) {
        allDockNames << dock->objectName();
    }

    m_currentPageUri = uri;
    emit currentPageUriChanged(uri);

    m_docksOpenStatusChanged.send(allDockNames);
}

bool DockWindow::isDockOpen(const QString& dockName) const
{
    const DockPageView* currPage = currentPage();
    return currPage ? currPage->isDockOpen(dockName) : false;
}

void DockWindow::toggleDock(const QString& dockName)
{
    DockPageView* currPage = currentPage();
    if (currPage) {
        currPage->toggleDock(dockName);
        m_docksOpenStatusChanged.send({ dockName });
    }
}

void DockWindow::setDockOpen(const QString& dockName, bool open)
{
    DockPageView* currPage = currentPage();
    if (currPage) {
        currPage->setDockOpen(dockName, open);
        m_docksOpenStatusChanged.send({ dockName });
    }
}

Channel<QStringList> DockWindow::docksOpenStatusChanged() const
{
    return m_docksOpenStatusChanged;
}

bool DockWindow::isDockFloating(const QString& dockName) const
{
    const DockPageView* currPage = currentPage();
    return currPage ? currPage->isDockFloating(dockName) : false;
}

void DockWindow::toggleDockFloating(const QString& dockName)
{
    DockPageView* currPage = currentPage();
    if (currPage) {
        currPage->toggleDockFloating(dockName);
    }
}

DropDestination DockWindow::hover(const QString& draggedDockName, const QPoint& globalPos)
{
    DockBase* draggedDock = dockByName(draggedDockName);
    if (!draggedDock) {
        return DropDestination();
    }

    QPoint hoveredLocalPos = m_mainWindow->mapFromGlobal(globalPos);

    if (isMouseOverDock(hoveredLocalPos, m_currentDropDestination.dock)) {
        return m_currentDropDestination;
    }

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

    m_currentDropDestination.dock->setSelected(false);

    if (m_currentDropDestination.dock->type() == DockType::DockingHolder) {
        m_currentDropDestination.dock->close();
    }

    m_currentDropDestination.clear();
}

DockBase* DockWindow::dockByName(const QString& dockName) const
{
    const DockPageView* page = currentPage();
    return page ? page->dockByName(dockName) : nullptr;
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

    addDock(page->centralDock(), KDDockWidgets::Location_OnRight);

    loadPagePanels(page);

    loadPageToolbars(page);

    if (page->statusBar()) {
        addDock(page->statusBar(), KDDockWidgets::Location_OnBottom);
    }

    QList<DockToolBarView*> allToolBars = m_toolBars.list();
    allToolBars << page->mainToolBars();

    DockToolBarView* prevToolBar = nullptr;

    for (DockToolBarView* toolBar : allToolBars) {
        auto location = prevToolBar ? KDDockWidgets::Location_OnRight : KDDockWidgets::Location_OnTop;
        addDock(toolBar, location, prevToolBar);
        prevToolBar = toolBar;
    }

    addDock(m_mainToolBarDockingHolder, KDDockWidgets::Location_OnTop);

    unitePanelsToTabs(page);
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

void DockWindow::loadPageToolbars(const DockPageView* page)
{
    QList<DockBase*> leftSideToolbars;
    if (auto leftHolder = page->toolBarHolderByLocation(Location::Left)) {
        leftSideToolbars << leftHolder;
    }

    QList<DockBase*> rightSideToolbars;
    if (auto rightHolder = page->toolBarHolderByLocation(Location::Right)) {
        rightSideToolbars << rightHolder;
    }

    QList<DockBase*> topSideToolbars;
    if (auto topHolder = page->toolBarHolderByLocation(Location::Top)) {
        topSideToolbars << topHolder;
    }

    QList<DockBase*> bottomSideToolbars;
    if (auto bottomHolder = page->toolBarHolderByLocation(Location::Bottom)) {
        bottomSideToolbars << bottomHolder;
    }

    QList<DockToolBarView*> pageToolBars = page->toolBars();

    for (DockBase* toolBar : pageToolBars) {
        switch (toolBar->location()) {
        case Location::Left:
            leftSideToolbars << toolBar;
            break;
        case Location::Right:
            rightSideToolbars << toolBar;
            break;
        case Location::Top:
            topSideToolbars << toolBar;
            break;
        case Location::Bottom:
            bottomSideToolbars << toolBar;
            break;
        case Location::Center:
        case Location::Undefined:
            LOGW() << "Error location for toolbar";
            break;
        }
    }

    for (int i = leftSideToolbars.size() - 1; i >= 0; --i) {
        addDock(leftSideToolbars[i], KDDockWidgets::Location_OnLeft);
    }

    for (int i = 0; i < rightSideToolbars.size(); ++i) {
        addDock(rightSideToolbars[i], KDDockWidgets::Location_OnRight);
    }

    for (int i = 0; i < bottomSideToolbars.size(); ++i) {
        addDock(bottomSideToolbars[i], KDDockWidgets::Location_OnBottom);
    }

    for (int i = topSideToolbars.size() - 1; i >= 0; --i) {
        addDock(topSideToolbars[i], KDDockWidgets::Location_OnTop);
    }
}

void DockWindow::loadPagePanels(const DockPageView* page)
{
    QList<DockBase*> leftSidePanels;
    if (auto leftHolder = page->panelHolderByLocation(Location::Left)) {
        leftSidePanels << leftHolder;
    }

    QList<DockBase*> rightSidePanels;
    if (auto rightHolder = page->panelHolderByLocation(Location::Right)) {
        rightSidePanels << rightHolder;
    }

    QList<DockBase*> topSidePanels;
    if (auto topHolder = page->panelHolderByLocation(Location::Top)) {
        topSidePanels << topHolder;
    }

    QList<DockBase*> bottomSidePanels;
    if (auto bottomHolder = page->panelHolderByLocation(Location::Bottom)) {
        bottomSidePanels << bottomHolder;
    }

    QList<DockPanelView*> pagePanels = page->panels();
    for (DockPanelView* panel : pagePanels) {
        switch (panel->location()) {
        case Location::Left:
            leftSidePanels << panel;
            break;
        case Location::Right:
            rightSidePanels << panel;
            break;
        case Location::Top:
            topSidePanels << panel;
            break;
        case Location::Bottom:
            bottomSidePanels << panel;
            break;
        case Location::Center:
        case Location::Undefined:
            break;
        }
    }

    for (int i = leftSidePanels.size() - 1; i >= 0; --i) {
        addDock(leftSidePanels[i], KDDockWidgets::Location_OnLeft);
    }

    for (int i = 0; i < rightSidePanels.size(); ++i) {
        addDock(rightSidePanels[i], KDDockWidgets::Location_OnRight);
    }

    for (int i = 0; i < bottomSidePanels.size(); ++i) {
        addDock(bottomSidePanels[i], KDDockWidgets::Location_OnBottom);
    }

    for (int i = topSidePanels.size() - 1; i >= 0; --i) {
        addDock(topSidePanels[i], KDDockWidgets::Location_OnTop);
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

DockPageView* DockWindow::currentPage() const
{
    return pageByUri(m_currentPageUri);
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
    TRACEFUNC;

    QString currentPageUriBackup = m_currentPageUri;

    /// NOTE: for reset geometry
    m_currentPageUri.clear();

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
    if (!dock || !dock->isVisible() || !m_mainWindow) {
        return false;
    }

    const KDDockWidgets::DockWidgetBase* dockWidget = dock->dockWidget();
    if (!dockWidget) {
        return false;
    }

    QRect frameGeometry = dockWidget->frameGeometry();
    frameGeometry.setTopLeft(m_mainWindow->mapFromGlobal(dockWidget->mapToGlobal({ dockWidget->x(), dockWidget->y() })));

    return frameGeometry.contains(mouseLocalPos);
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

    if (m_currentDropDestination.dock->type() != DockType::DockingHolder) {
        m_currentDropDestination.dock->setSelected(true);
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
    m_currentDropDestination.dock->setSelected(true);
    m_currentDropDestination.dock->init();
}

DropDestination DockWindow::resolveDropDestination(const DockBase* draggedDock, const QPoint& localPos) const
{
    if (draggedDock->type() == DockType::Panel) {
        DropDestination destination;

        destination.dock = findTabifyPanel(dynamic_cast<const DockPanelView*>(draggedDock), localPos);
        destination.dropLocation = destination.dock ? Location::Center : Location::Undefined;

        if (destination.isValid()) {
            return destination;
        }
    }

    const DockBase* holder = resolveDockingHolder(draggedDock->type(), localPos);
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
    switch (draggedDockType) {
    case DockType::ToolBar:
        return resolveToolbarDockingHolder(localPos);
    case DockType::Panel:
        return resolvePanelDockingHolder(localPos);
    case DockType::Central:
    case DockType::StatusBar:
    case DockType::DockingHolder:
    case DockType::Undefined:
        break;
    }

    return nullptr;
}

DockingHolderView* DockWindow::resolveToolbarDockingHolder(const QPoint& localPos) const
{
    const DockPageView* page = currentPage();
    if (!page) {
        return nullptr;
    }

    const KDDockWidgets::DockWidgetBase* centralDock = page->centralDock()->dockWidget();
    if (!centralDock) {
        return nullptr;
    }

    if (!m_mainWindow->contains(localPos)) {
        return nullptr;
    }

    QRect centralFrameGeometry = centralDock->frameGeometry();
    centralFrameGeometry.moveTopLeft(m_mainWindow->mapFromGlobal(centralDock->mapToGlobal({ centralDock->x(), centralDock->y() })));

    // TODO: Need to take any panels docked at top into account
    if (localPos.y() <= centralFrameGeometry.top() + MAX_DISTANCE_TO_HOLDER) {
        return page->toolBarHolderByLocation(Location::Top);
    }

    if (localPos.y() >= centralFrameGeometry.bottom() - MAX_DISTANCE_TO_HOLDER) {
        return page->toolBarHolderByLocation(Location::Bottom);
    }

    if (localPos.x() <= MAX_DISTANCE_TO_HOLDER) {
        return page->toolBarHolderByLocation(Location::Left);
    }

    if (localPos.x() >= m_mainWindow->rect().right() - MAX_DISTANCE_TO_HOLDER) {
        return page->toolBarHolderByLocation(Location::Right);
    }

    return nullptr;
}

DockingHolderView* DockWindow::resolvePanelDockingHolder(const QPoint& localPos) const
{
    const DockPageView* page = currentPage();
    if (!page) {
        return nullptr;
    }

    const KDDockWidgets::DockWidgetBase* centralDock = page->centralDock()->dockWidget();
    if (!centralDock) {
        return nullptr;
    }

    if (!m_mainWindow->contains(localPos)) {
        return nullptr;
    }

    QRect centralFrameGeometry = centralDock->frameGeometry();
    centralFrameGeometry.moveTopLeft(m_mainWindow->mapFromGlobal(centralDock->mapToGlobal({ centralDock->x(), centralDock->y() })));

    if (localPos.y() <= centralFrameGeometry.top() + MAX_DISTANCE_TO_HOLDER) {
        return page->panelHolderByLocation(Location::Top);
    }

    if (localPos.y() >= centralFrameGeometry.bottom() - MAX_DISTANCE_TO_HOLDER) {
        return page->panelHolderByLocation(Location::Bottom);
    }

    if (localPos.x() <= MAX_DISTANCE_TO_HOLDER) {
        return page->panelHolderByLocation(Location::Left);
    }

    if (localPos.x() >= centralFrameGeometry.right() - MAX_DISTANCE_TO_HOLDER) {
        return page->panelHolderByLocation(Location::Right);
    }

    return nullptr;
}

DockPanelView* DockWindow::findTabifyPanel(const DockPanelView* panel, const QPoint& localPos) const
{
    auto hoverOverPanel = [localPos, panel, this](const DockPanelView* p) {
        if (p == panel || p->floating()) {
            return false;
        }

        return isMouseOverDock(localPos, p);
    };

    DockPanelView* rootPanel = findRootPanel(panel);
    if (hoverOverPanel(rootPanel)) {
        return rootPanel;
    }

    DockPanelView* nextPanel = rootPanel ? rootPanel->tabifyPanel() : nullptr;

    while (nextPanel) {
        if (hoverOverPanel(nextPanel)) {
            return nextPanel;
        }

        nextPanel = nextPanel->tabifyPanel();
    }

    return nullptr;
}

DockPanelView* DockWindow::findRootPanel(const DockPanelView* panel) const
{
    for (DockPanelView* panel_ : currentPage()->panels()) {
        DockPanelView* tabifyPanel = panel_->tabifyPanel();

        while (tabifyPanel) {
            if (tabifyPanel == panel) {
                return panel_;
            }

            tabifyPanel = tabifyPanel->tabifyPanel();
        }
    }

    return const_cast<DockPanelView*>(panel);
}
