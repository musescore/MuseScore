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

#include "dockpage.h"
#include "docktoolbar.h"
#include "dockpanel.h"

#include "log.h"

#include "thirdparty/KDDockWidgets/src/private/quick/MainWindowQuick_p.h"
#include "thirdparty/KDDockWidgets/src/DockWidgetQuick.h"
#include "thirdparty/KDDockWidgets/src/LayoutSaver.h"
#include "thirdparty/KDDockWidgets/src/KDDockWidgets.h"

using namespace mu::dock;

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

    m_currentPageUri = uri;
    emit currentPageUriChanged(uri);
}

void DockWindow::loadPageContent(const DockPage* page)
{
    TRACEFUNC;

    addDock(page->centralDock(), KDDockWidgets::Location_OnRight);

    for (DockPanel* panel : page->panels()) {
        //! TODO: add an ability to change location of panels
        addDock(panel, KDDockWidgets::Location_OnLeft);
    }

    const DockToolBar* prevToolBar = nullptr;
    for (DockToolBar* toolBar : page->toolBars()) {
        //! NOTE: need to add dock to its default position
        //! the following dock will be added on the right side
        //! if there is a dock at the default position
        auto location = prevToolBar ? KDDockWidgets::Location_OnRight : KDDockWidgets::Location_OnTop;
        addDock(toolBar, location, prevToolBar);
        prevToolBar = toolBar;
    }

    const DockStatusBar* prevStatusBar = nullptr;
    for (DockStatusBar* statusBar : page->statusBars()) {
        auto location = prevStatusBar ? KDDockWidgets::Location_OnRight : KDDockWidgets::Location_OnBottom;
        addDock(statusBar, location, prevStatusBar);
        prevStatusBar = statusBar;
    }

    QList<DockToolBar*> allToolBars = m_toolBars.list();
    allToolBars << page->mainToolBars();

    prevToolBar = nullptr;

    for (DockToolBar* toolBar : allToolBars) {
        auto location = prevToolBar ? KDDockWidgets::Location_OnRight : KDDockWidgets::Location_OnTop;
        addDock(toolBar, location, prevToolBar);
        prevToolBar = toolBar;
    }

    unitePanelsToTabs(page);
}

void DockWindow::unitePanelsToTabs(const DockPage* page)
{
    for (const DockPanel* panel : page->panels()) {
        const DockPanel* tab = panel->tabifyPanel();

        if (tab) {
            panel->dockWidget()->addDockWidgetAsTab(tab->dockWidget());

            KDDockWidgets::Frame* frame = panel->dockWidget()->frame();
            if (frame) {
                frame->setCurrentTabIndex(0);
            }
        }
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

    if (page) {
        page->init();
    }
}
