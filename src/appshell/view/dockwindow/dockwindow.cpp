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
#include "thirdparty/KDDockWidgets/src/private/Frame_p.h"
#include "thirdparty/KDDockWidgets/src/private/quick/MainWindowQuick_p.h"

#include "dockcentral.h"
#include "dockpage.h"
#include "dockpanel.h"
#include "dockpanelholder.h"
#include "dockstatusbar.h"
#include "docktoolbar.h"
#include "docktoolbarholder.h"

#include "log.h"

using namespace mu::dock;

static constexpr double MAX_DISTANCE_TO_HOLDER = 25;

DockWindow::DockWindow(QQuickItem* parent)
    : QQuickItem(parent),
    m_toolBars(this),
    m_pages(this)
{
}

void DockWindow::componentComplete()
{
    TRACEFUNC;

    QQuickItem::componentComplete();

    m_mainWindow = new KDDockWidgets::MainWindowQuick("mainWindow",
                                                      KDDockWidgets::MainWindowOption_None,
                                                      this);

    connect(qApp, &QCoreApplication::aboutToQuit, this, &DockWindow::onQuit);

    configuration()->windowGeometryChanged().onNotify(this, [this]() {
        if (m_quiting) {
            return;
        }

        resetWindowState();
    });

    mainWindow()->changeToolBarOrientationRequested().onReceive(this, [this](QString name, mu::framework::Orientation orientation) {
        const DockPage* page = currentPage();
        DockToolBar* toolBar = page ? dynamic_cast<DockToolBar*>(page->dockByName(name)) : nullptr;

        if (toolBar) {
            toolBar->setOrientation(static_cast<Qt::Orientation>(orientation));
        }
    });

    mainWindow()->hideAllDockingHoldersRequested().onNotify(this, [this]() {
        hideCurrentToolBarDockingHolder();
        hideCurrentPanelDockingHolder();
    });

    mainWindow()->showToolBarDockingHolderRequested().onReceive(this, [this](const QPoint& mouseGlobalPos) {
        QPoint localPos = m_mainWindow->mapFromGlobal(mouseGlobalPos);
        QRect mainFrameGeometry = m_mainWindow->rect();

        if (!mainFrameGeometry.contains(localPos)) {
            return;
        }

        if (isMouseOverCurrentToolBarDockingHolder(localPos)) {
            return;
        }

        DockToolBarHolder* holder = resolveToolbarDockingHolder(localPos);

        if (holder != m_currentToolBarDockingHolder) {
            hideCurrentToolBarDockingHolder();

            if (holder) {
                holder->open();
            }
        }

        m_currentToolBarDockingHolder = holder;
    });

    mainWindow()->showPanelDockingHolderRequested().onReceive(this, [this](const QPoint& mouseGlobalPos) {
        QPoint localPos = m_mainWindow->mapFromGlobal(mouseGlobalPos);
        QRect mainFrameGeometry = m_mainWindow->rect();

        if (!mainFrameGeometry.contains(localPos)) {
            return;
        }

        if (isMouseOverCurrentPanelDockingHolder(localPos)) {
            return;
        }

        DockPanelHolder* holder = resolvePanelDockingHolder(localPos);

        if (holder != m_currentPanelDockingHolder) {
            hideCurrentPanelDockingHolder();

            if (holder) {
                qDebug() << holder->location();
                holder->open();
            }
        }

        m_currentPanelDockingHolder = holder;
    });

    startupScenario()->run();
}

void DockWindow::onQuit()
{
    TRACEFUNC;

    m_quiting = true;

    saveGeometry();

    const DockPage* currPage = currentPage();
    IF_ASSERT_FAILED(currPage) {
        return;
    }

    savePageState(currPage->objectName());
}

QString DockWindow::currentPageUri() const
{
    return m_currentPageUri;
}

QQmlListProperty<mu::dock::DockToolBar> DockWindow::toolBarsProperty()
{
    return m_toolBars.property();
}

QQmlListProperty<mu::dock::DockPage> DockWindow::pagesProperty()
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

    DockPage* newPage = pageByUri(uri);
    IF_ASSERT_FAILED(newPage) {
        return;
    }

    DockPage* currentPage = this->currentPage();
    if (currentPage) {
        savePageState(currentPage->objectName());
        currentPage->close();
    }

    loadPageContent(newPage);
    restorePageState(newPage->objectName());
    initDocks(newPage);

    for (DockBase* dock : newPage->allDocks()) {
        if (!dock->isVisible()) {
            dock->close();
        }
    }

    m_currentPageUri = uri;
    emit currentPageUriChanged(uri);
}

bool DockWindow::isDockOpen(const QString& dockName) const
{
    const DockPage* currPage = currentPage();
    return currPage ? currPage->isDockOpen(dockName) : false;
}

void DockWindow::toggleDock(const QString& dockName)
{
    DockPage* currPage = currentPage();
    if (currPage) {
        currPage->toggleDock(dockName);
    }
}

void DockWindow::setDockOpen(const QString& dockName, bool open)
{
    DockPage* currPage = currentPage();
    if (currPage) {
        currPage->setDockOpen(dockName, open);
    }
}

DockToolBarHolder* DockWindow::mainToolBarDockingHolder() const
{
    return m_mainToolBarDockingHolder;
}

void DockWindow::setMainToolBarDockingHolder(DockToolBarHolder* mainToolBarDockingHolder)
{
    if (m_mainToolBarDockingHolder == mainToolBarDockingHolder) {
        return;
    }

    m_mainToolBarDockingHolder = mainToolBarDockingHolder;
    emit mainToolBarDockingHolderChanged(m_mainToolBarDockingHolder);
}

void DockWindow::loadPageContent(const DockPage* page)
{
    TRACEFUNC;

    addDock(page->centralDock(), KDDockWidgets::Location_OnRight);

    loadPagePanels(page);

    loadPageToolbars(page);

    if (page->statusBar()) {
        addDock(page->statusBar(), KDDockWidgets::Location_OnBottom);
    }

    QList<DockToolBar*> allToolBars = m_toolBars.list();
    allToolBars << page->mainToolBars();

    DockToolBar* prevToolBar = nullptr;

    for (DockToolBar* toolBar : allToolBars) {
        auto location = prevToolBar ? KDDockWidgets::Location_OnRight : KDDockWidgets::Location_OnTop;
        addDock(toolBar, location, prevToolBar);
        prevToolBar = toolBar;
    }

    addDock(m_mainToolBarDockingHolder, KDDockWidgets::Location_OnTop);
    m_mainToolBarDockingHolder->close();

    unitePanelsToTabs(page);
}

void DockWindow::unitePanelsToTabs(const DockPage* page)
{
    for (const DockPanel* panel : page->panels()) {
        const DockPanel* tab = panel->tabifyPanel();
        if (!tab) {
            continue;
        }

        panel->dockWidget()->addDockWidgetAsTab(tab->dockWidget());

        KDDockWidgets::Frame* frame = panel->dockWidget()->frame();
        if (frame) {
            frame->setCurrentTabIndex(0);
        }
    }
}

void DockWindow::loadPageToolbars(const DockPage* page)
{
    QList<DockToolBar*> leftSideToolbars;
    QList<DockToolBar*> rightSideToolbars;
    QList<DockToolBar*> topSideToolbars;
    QList<DockToolBar*> bottomSideToolbars;

    QList<DockToolBar*> pageToolBars = page->toolBars();
    for (DockToolBar* toolBar : pageToolBars) {
        switch (toolBar->location()) {
        case DockBase::DockLocation::Left:
            leftSideToolbars << toolBar;
            break;
        case DockBase::DockLocation::Right:
            rightSideToolbars << toolBar;
            break;
        case DockBase::DockLocation::Top:
            topSideToolbars << toolBar;
            break;
        case DockBase::DockLocation::Bottom:
            bottomSideToolbars << toolBar;
            break;
        case DockBase::DockLocation::Center:
        case DockBase::DockLocation::Undefined:
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

void DockWindow::loadPagePanels(const DockPage* page)
{
    QList<DockPanel*> leftSidePanels;
    QList<DockPanel*> rightSidePanels;
    QList<DockPanel*> topSidePanels;
    QList<DockPanel*> bottomSidePanels;

    QList<DockPanel*> pagePanels = page->panels();
    for (DockPanel* panel : pagePanels) {
        switch (panel->location()) {
        case DockBase::DockLocation::Left:
            leftSidePanels << panel;
            break;
        case DockBase::DockLocation::Right:
            rightSidePanels << panel;
            break;
        case DockBase::DockLocation::Top:
            topSidePanels << panel;
            break;
        case DockBase::DockLocation::Bottom:
            bottomSidePanels << panel;
            break;
        default:
            if (panel->allowedAreas() & Qt::BottomDockWidgetArea) {
                bottomSidePanels << panel;
            } else {
                leftSidePanels << panel;
            }
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

void DockWindow::addDock(DockBase* dock, KDDockWidgets::Location location, const DockBase* relativeTo)
{
    IF_ASSERT_FAILED(dock) {
        return;
    }

    KDDockWidgets::DockWidgetBase* relativeDock = relativeTo ? relativeTo->dockWidget() : nullptr;
    m_mainWindow->addDockWidget(dock->dockWidget(), location, relativeDock, dock->preferredSize());
}

DockPage* DockWindow::pageByUri(const QString& uri) const
{
    for (DockPage* page : m_pages.list()) {
        if (page->uri() == uri) {
            return page;
        }
    }

    return nullptr;
}

DockPage* DockWindow::currentPage() const
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
    configuration()->setWindowGeometry(windowState());
}

void DockWindow::restoreGeometry()
{
    TRACEFUNC;

    if (!restoreLayout(configuration()->windowGeometry())) {
        LOGE() << "Could not restore the window geometry!";
    }
}

void DockWindow::savePageState(const QString& pageName)
{
    TRACEFUNC;

    configuration()->setPageState(pageName, windowState());
}

void DockWindow::restorePageState(const QString& pageName)
{
    TRACEFUNC;

    /// NOTE: Do not restore geometry
    bool ok = restoreLayout(configuration()->pageState(pageName), KDDockWidgets::RestoreOption::RestoreOption_RelativeToMainWindow);
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

void DockWindow::initDocks(DockPage* page)
{
    for (DockToolBar* toolbar : m_toolBars.list()) {
        toolbar->init();
    }

    m_mainToolBarDockingHolder->init();

    if (page) {
        page->init();
    }
}

DockToolBarHolder* DockWindow::resolveToolbarDockingHolder(const QPoint& localPos) const
{
    const DockPage* page = currentPage();
    if (!page) {
        return nullptr;
    }

    const KDDockWidgets::DockWidgetBase* centralDock = page->centralDock()->dockWidget();
    if (!centralDock) {
        return nullptr;
    }

    QRect centralFrameGeometry = centralDock->frameGeometry();
    centralFrameGeometry.moveTopLeft(m_mainWindow->mapFromGlobal(centralDock->mapToGlobal({ centralDock->x(), centralDock->y() })));

    QRect mainFrameGeometry = m_mainWindow->rect();
    DockToolBarHolder* newHolder = nullptr;

    if (localPos.y() < MAX_DISTANCE_TO_HOLDER) { // main toolbar holder
        newHolder = m_mainToolBarDockingHolder;
    }
    // TODO: Need to take any panels docked at top into account
    else if (localPos.y() > centralFrameGeometry.top()
             && localPos.y() < centralFrameGeometry.top() + MAX_DISTANCE_TO_HOLDER) {   // page top toolbar holder
        newHolder = page->toolBarHolderByLocation(DockBase::DockLocation::Top);
    } else if (localPos.y() < centralFrameGeometry.bottom()) { // page left toolbar holder
        if (localPos.x() < MAX_DISTANCE_TO_HOLDER) {
            newHolder = page->toolBarHolderByLocation(DockBase::DockLocation::Left);
        } else if (localPos.x() > mainFrameGeometry.right() - MAX_DISTANCE_TO_HOLDER) { // page right toolbar holder
            newHolder = page->toolBarHolderByLocation(DockBase::DockLocation::Right);
        }
    } else if (localPos.y() < mainFrameGeometry.bottom()) { // page bottom toolbar holder
        newHolder = page->toolBarHolderByLocation(DockBase::DockLocation::Bottom);
    }

    return newHolder;
}

DockPanelHolder* DockWindow::resolvePanelDockingHolder(const QPoint& localPos) const
{
    const DockPage* page = currentPage();
    if (!page) {
        return nullptr;
    }

    const KDDockWidgets::DockWidgetBase* centralDock = page->centralDock()->dockWidget();
    if (!centralDock) {
        return nullptr;
    }

    QRect centralFrameGeometry = centralDock->frameGeometry();
    centralFrameGeometry.moveTopLeft(m_mainWindow->mapFromGlobal(centralDock->mapToGlobal({ centralDock->x(), centralDock->y() })));
    DockPanelHolder* newHolder = nullptr;

    if (localPos.y() > centralFrameGeometry.top()
        && localPos.y() < centralFrameGeometry.top() + MAX_DISTANCE_TO_HOLDER) { // page top panel holder
        newHolder = page->panelHolderByLocation(DockBase::DockLocation::Top);
    } else if (localPos.y() < centralFrameGeometry.bottom()) { // page left panel holder
        if (localPos.x() < MAX_DISTANCE_TO_HOLDER) {
            newHolder = page->panelHolderByLocation(DockBase::DockLocation::Left);
        } else if (localPos.x() > centralFrameGeometry.right() - MAX_DISTANCE_TO_HOLDER) { // page right panel holder
            newHolder = page->panelHolderByLocation(DockBase::DockLocation::Right);
        }
    } else if (localPos.y() < centralFrameGeometry.bottom()) { // page bottom panel holder
        newHolder = page->panelHolderByLocation(DockBase::DockLocation::Bottom);
    }

    return newHolder;
}

void DockWindow::hideCurrentToolBarDockingHolder()
{
    if (!m_currentToolBarDockingHolder) {
        return;
    }

    m_currentToolBarDockingHolder->close();
    m_currentToolBarDockingHolder = nullptr;
}

void DockWindow::hideCurrentPanelDockingHolder()
{
    if (!m_currentPanelDockingHolder) {
        return;
    }

    m_currentPanelDockingHolder->close();
    m_currentPanelDockingHolder = nullptr;
}

bool DockWindow::isMouseOverCurrentToolBarDockingHolder(const QPoint& mouseLocalPos) const
{
    if (!m_currentToolBarDockingHolder || !m_mainWindow) {
        return false;
    }

    const KDDockWidgets::DockWidgetBase* holderDock = m_currentToolBarDockingHolder->dockWidget();
    if (!holderDock) {
        return false;
    }

    QRect holderFrameGeometry = holderDock->frameGeometry();
    holderFrameGeometry.setTopLeft(m_mainWindow->mapFromGlobal(holderDock->mapToGlobal({ holderDock->x(), holderDock->y() })));
    return holderFrameGeometry.contains(mouseLocalPos);
}

bool DockWindow::isMouseOverCurrentPanelDockingHolder(const QPoint& mouseLocalPos) const
{
    if (!m_currentPanelDockingHolder || !m_mainWindow) {
        return false;
    }

    const KDDockWidgets::DockWidgetBase* holderDock = m_currentPanelDockingHolder->dockWidget();
    if (!holderDock) {
        return false;
    }

    QRect holderFrameGeometry = holderDock->frameGeometry();
    holderFrameGeometry.setTopLeft(m_mainWindow->mapFromGlobal(holderDock->mapToGlobal({ holderDock->x(), holderDock->y() })));
    return holderFrameGeometry.contains(mouseLocalPos);
}
