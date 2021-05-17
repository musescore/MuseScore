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

#include "thirdparty/KDDockWidgets/src/private/quick/MainWindowQuick_p.h"
#include "thirdparty/KDDockWidgets/src/DockWidgetQuick.h"
#include "thirdparty/KDDockWidgets/src/LayoutSaver.h"
#include "thirdparty/KDDockWidgets/src/KDDockWidgets.h"
#include "thirdparty/KDDockWidgets/src/private/DockRegistry_p.h"

#include "log.h"

#include "dockpage.h"
#include "docktoolbar.h"
#include "dockpanel.h"

using namespace mu::dock;

static const double MAX_DISTANCE_TO_HELPER = 25;

DockWindow::DockWindow(QQuickItem* parent)
    : QQuickItem(parent),
    m_toolBars(this),
    m_pages(this)
{
}

void DockWindow::componentComplete()
{
    QQuickItem::componentComplete();

    m_mainWindow = new KDDockWidgets::MainWindowQuick("mainWindow",
                                                      KDDockWidgets::MainWindowOption_None,
                                                      this);

    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        saveGeometry();
    });

    configuration()->windowGeometryChanged().onNotify(this, [this]() {
        resetWindowState();
    });

    mainWindow()->toolBarOrientationChangeRequested().onReceive(this, [this](std::pair<QString, mu::framework::Orientation> orientation) {
        const DockPage* page = pageByUri(m_currentPageUri);
        DockToolBar* toolBar = page ? dynamic_cast<DockToolBar*>(page->dockByName(orientation.first)) : nullptr;

        if (toolBar) {
            toolBar->setOrientation(static_cast<Qt::Orientation>(orientation.second));
        }
    });

    mainWindow()->toolbarsDockingHelpersUpdateRequested().onReceive(this, [this](const QPoint& mouseGlobalPos) {
        if (mouseGlobalPos.isNull()) {
            hideCurrentToolBarDockingHelper();
            return;
        }

        QPoint localPos = m_mainWindow->mapFromGlobal(mouseGlobalPos);
        QRect mainFrameGeometry = m_mainWindow->rect();

        if (!mainFrameGeometry.contains(localPos)) {
            return;
        }

        if (isMouseOverCurrentToolBarDockingHelper(localPos)) {
            return;
        }

        DockToolBar* helper = resolveToolbarDockingHelper(localPos);

        if (helper != m_currentToolBarDockingHelper) {
            hideCurrentToolBarDockingHelper();
            if (helper) {
                helper->show();
            }
        }

        m_currentToolBarDockingHelper = helper;
    });
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

    DockPage* currentPage = pageByUri(m_currentPageUri);
    if (currentPage) {
        saveState(currentPage->objectName());
        currentPage->close();
    }

    loadPageContent(newPage);
    restoreState(newPage->objectName());
    initDocks(newPage);

    for (DockBase* dock : newPage->allDocks()) {
        if (!dock->isVisible()) {
            dock->hide();
        }
    }

    m_currentPageUri = uri;
    emit currentPageUriChanged(uri);
}

DockToolBar* DockWindow::mainToolBarDockingHelper() const
{
    return m_mainToolBarDockingHelper;
}

void DockWindow::setMainToolBarDockingHelper(DockToolBar* mainToolBarDockingHelper)
{
    if (m_mainToolBarDockingHelper == mainToolBarDockingHelper) {
        return;
    }

    m_mainToolBarDockingHelper = mainToolBarDockingHelper;
    emit mainToolBarDockingHelperChanged(m_mainToolBarDockingHelper);
}

void DockWindow::loadPageContent(const DockPage* page)
{
    TRACEFUNC;

    addDock(page->centralDock(), KDDockWidgets::Location_OnRight);

    for (DockPanel* panel : page->panels()) {
        //! TODO: add an ability to change location of panels
        addDock(panel, KDDockWidgets::Location_OnLeft);
    }

    loadPageToolbars(page);

    const DockStatusBar* prevStatusBar = nullptr;
    for (DockStatusBar* statusBar : page->statusBars()) {
        auto location = prevStatusBar ? KDDockWidgets::Location_OnRight : KDDockWidgets::Location_OnBottom;
        addDock(statusBar, location, prevStatusBar);
        prevStatusBar = statusBar;
    }

    QList<DockToolBar*> allToolBars = m_toolBars.list();
    allToolBars << page->mainToolBars();

    DockToolBar* prevToolBar = nullptr;

    for (DockToolBar* toolBar : allToolBars) {
        auto location = prevToolBar ? KDDockWidgets::Location_OnRight : KDDockWidgets::Location_OnTop;
        addDock(toolBar, location, prevToolBar);
        prevToolBar = toolBar;
    }

    addDock(m_mainToolBarDockingHelper, KDDockWidgets::Location_OnTop);
    m_mainToolBarDockingHelper->hide();

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
        DockToolBar* toolBar = bottomSideToolbars[i];
        const DockToolBar* tabifyToolBar = toolBar->tabifyToolBar();
        auto location = tabifyToolBar ? KDDockWidgets::Location_OnRight : KDDockWidgets::Location_OnBottom;
        addDock(toolBar, location, tabifyToolBar);
    }

    for (int i = topSideToolbars.size() - 1; i >= 0; --i) {
        DockToolBar* toolBar = topSideToolbars[i];
        const DockToolBar* tabifyToolBar = toolBar->tabifyToolBar();
        auto location = tabifyToolBar ? KDDockWidgets::Location_OnLeft : KDDockWidgets::Location_OnTop;
        addDock(toolBar, location, tabifyToolBar);
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

void DockWindow::saveGeometry()
{
    TRACEFUNC;

    /// NOTE: The state of all dock widgets is also saved here,
    /// since the library does not provide the ability to save
    /// and restore only the application geometry.
    /// Therefore, for correct operation after saving or restoring geometry,
    /// it is necessary to apply the appropriate method for the state.
    KDDockWidgets::LayoutSaver layoutSaver;
    configuration()->setWindowGeometry(layoutSaver.serializeLayout());
}

void DockWindow::restoreGeometry()
{
    TRACEFUNC;

    QByteArray state = configuration()->windowGeometry();
    KDDockWidgets::LayoutSaver layoutSaver;
    layoutSaver.restoreLayout(state);
}

void DockWindow::saveState(const QString& pageName)
{
    TRACEFUNC;

    KDDockWidgets::LayoutSaver layoutSaver;
    configuration()->setPageState(pageName.toStdString(), layoutSaver.serializeLayout());
}

void DockWindow::restoreState(const QString& pageName)
{
    TRACEFUNC;

    QByteArray state = configuration()->pageState(pageName.toStdString());
    if (state.isEmpty()) {
        return;
    }

    /// NOTE: Do not restore geometry
    KDDockWidgets::RestoreOption option = KDDockWidgets::RestoreOption::RestoreOption_RelativeToMainWindow;

    KDDockWidgets::LayoutSaver layoutSaver(option);
    layoutSaver.restoreLayout(state);
}

void DockWindow::resetWindowState()
{
    TRACEFUNC;

    QString currentPageUriBackup = m_currentPageUri;

    /// NOTE: for reset geometry
    m_currentPageUri = "";

    loadPage(currentPageUriBackup);
}

void DockWindow::initDocks(DockPage* page)
{
    for (DockToolBar* toolbar : m_toolBars.list()) {
        toolbar->init();
    }

    m_mainToolBarDockingHelper->init();

    if (page) {
        page->init();
    }
}

DockToolBar* DockWindow::resolveToolbarDockingHelper(const QPoint& localPos) const
{
    DockPage* page = pageByUri(m_currentPageUri);
    if (!page) {
        return nullptr;
    }

    QList<DockToolBar*> helpers = page->toolBarsHelpers();

    KDDockWidgets::DockWidgetBase* centralDock = KDDockWidgets::DockRegistry::self()->dockByName(page->centralDock()->objectName());
    QRect centralFrameGeometry = centralDock->frameGeometry();
    centralFrameGeometry.setTopLeft(m_mainWindow->mapFromGlobal(centralDock->mapToGlobal({ centralDock->x(), centralDock->y() })));

    QRect mainFrameGeometry = m_mainWindow->rect();

    auto helper = [=](DockBase::DockLocation location) -> DockToolBar* {
        for (DockToolBar* helper : helpers) {
            if (helper->location() == location) {
                return helper;
            }
        }

        return nullptr;
    };

    mu::dock::DockToolBar* newHelper = nullptr;

    if (localPos.y() < MAX_DISTANCE_TO_HELPER) { // main toolbar helper
        newHelper = m_mainToolBarDockingHelper;
    } else if (localPos.y() > centralFrameGeometry.top()
               && localPos.y() < centralFrameGeometry.top() + MAX_DISTANCE_TO_HELPER) { // page top toolbar helper
        newHelper = helper(DockBase::DockLocation::Top);
    } else if (localPos.y() < centralFrameGeometry.bottom()) { // page left toolbar helper
        if (localPos.x() < MAX_DISTANCE_TO_HELPER) {
            newHelper = helper(DockBase::DockLocation::Left);
        } else if (localPos.x() > mainFrameGeometry.right() - MAX_DISTANCE_TO_HELPER) { // page right toolbar helper
            newHelper = helper(DockBase::DockLocation::Right);
        }
    } else if (localPos.y() < mainFrameGeometry.bottom()) { // page bottom toolbar helper
        newHelper = helper(DockBase::DockLocation::Bottom);
    }

    return newHelper;
}

void DockWindow::hideCurrentToolBarDockingHelper()
{
    if (!m_currentToolBarDockingHelper) {
        return;
    }

    m_currentToolBarDockingHelper->hide();
    m_currentToolBarDockingHelper = nullptr;
}

bool DockWindow::isMouseOverCurrentToolBarDockingHelper(const QPoint& mouseLocalPos) const
{
    if (!m_currentToolBarDockingHelper || !m_mainWindow) {
        return false;
    }
    KDDockWidgets::DockWidgetBase* helperDock
        = KDDockWidgets::DockRegistry::self()->dockByName(m_currentToolBarDockingHelper->objectName());
    if (!helperDock) {
        return false;
    }

    QRect helperFrameGeometry = helperDock->frameGeometry();
    helperFrameGeometry.setTopLeft(m_mainWindow->mapFromGlobal(helperDock->mapToGlobal({ helperDock->x(), helperDock->y() })));
    return helperFrameGeometry.contains(mouseLocalPos);
}
