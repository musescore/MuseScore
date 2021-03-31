//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "dockwindow.h"

#include <QDebug>
#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QStackedWidget>
#include <QDockWidget>
#include <QStatusBar>
#include <QMenuBar>

#include "log.h"
#include "eventswatcher.h"
#include "modularity/ioc.h"

#include "framework/global/widgetstatestore.h"

using namespace mu::dock;
using namespace mu::uicomponents;

static const QString WINDOW_QSS = QString("QMainWindow { background: %1; } "
                                          "QMainWindow::separator { background: %2; width: 1px; } "
                                          "QTabBar::tab { background: %1; border: 2px solid; padding: 2px; }"
                                          "QTabBar::tab:selected { border-color: #9B9B9B; border-bottom-color: #C2C7CB; }");

static const QString STATUS_QSS = QString("QStatusBar { background: %1; border-top: 1px solid %2; } QStatusBar::item { border: 0 }");

static void addMenu(QMenu* menu, QMenuBar* menuBar)
{
#ifdef Q_OS_MAC
    menu->setParent(menuBar);
#endif
    menuBar->addMenu(menu);
}

DockWindow::DockWindow(QQuickItem* parent)
    : QQuickItem(parent), m_toolbars(this), m_pages(this)
{
    framework::ioc()->registerExportNoDelete<ui::IMainWindow>("dock", this);

    setFlag(QQuickItem::ItemHasContents, true);
    m_window = new QMainWindow();
    m_window->setObjectName("mainWindow");
    m_window->setMinimumSize(800, 600);
    setWidth(1024);
    setHeight(800);

    m_eventsWatcher = new EventsWatcher(this);
    m_window->installEventFilter(m_eventsWatcher);
    connect(m_eventsWatcher, &EventsWatcher::eventReceived, this, &DockWindow::onMainWindowEvent);

    m_central = new QStackedWidget(m_window);
    m_window->setCentralWidget(m_central);

    m_window->setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
    m_window->setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);
    m_window->setAnimated(false);

    m_statusbar = new QStatusBar(m_window);
    m_statusbar->setSizeGripEnabled(false);
    m_window->setStatusBar(m_statusbar);

    connect(m_pages.notifier(), &uicomponents::QmlListPropertyNotifier::appended, this, &DockWindow::onPageAppended);
    connect(this, &DockWindow::colorChanged, this, &DockWindow::updateStyle);
    connect(this, &DockWindow::borderColorChanged, this, &DockWindow::updateStyle);

    configuration()->pageStateChanged().onNotify(nullptr, [this]() {
        togglePage(nullptr, currentPage());
    });

    WidgetStateStore::restoreGeometry(m_window);
}

void DockWindow::componentComplete()
{
    QQuickItem::componentComplete();

    for (DockToolBar* toolbar : m_toolbars.list()) {
        DockToolBar::Widget tw = toolbar->widget();
        tw.bar->setParent(m_window);
        m_window->addToolBar(tw.bar);
    }

    togglePage(nullptr, currentPage());

    m_window->show();

    m_isComponentComplete = true;
}

void DockWindow::onMainWindowEvent(QEvent* event)
{
    switch (event->type()) {
    case QEvent::Paint: {
        configuration()->applyPlatformStyle(m_window);
    } break;
    case QEvent::Resize: {
        QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(event);
        setSize(QSizeF(resizeEvent->size()));
        adjustPanelsSize(currentPage());
    } break;
    case QEvent::Close: {
        WidgetStateStore::saveGeometry(m_window);
        saveState(currentPage()->objectName());
    } break;
    case QEvent::LayoutRequest: {
        saveState(currentPage()->objectName());
    }
    default:
        break;
    }
}

void DockWindow::adjustPanelsSize(DockPage* page)
{
    if (!page) {
        return;
    }

    constexpr int additionalSideSpace = 200; //! NOTE: experimental value
    bool needMinimize = qMainWindow()->width() <= qMainWindow()->minimumWidth() + additionalSideSpace;

    for (DockPanel* panel : page->panels()) {
        if (needMinimize) {
            qMainWindow()->resizeDocks({ panel->widget().panel }, { panel->minimumWidth() }, Qt::Horizontal);
        } else {
            qMainWindow()->resizeDocks({ panel->widget().panel }, { panel->preferedWidth() }, Qt::Horizontal);
        }
    }
}

void DockWindow::saveState(const QString& pageName)
{
    configuration()->setPageState(pageName.toStdString(), m_window->saveState());
}

void DockWindow::restoreState(const QString& pageName)
{
    QByteArray state = configuration()->pageState(pageName.toStdString());
    if (!state.isEmpty()) {
        m_window->restoreState(state);
    }
}

void DockWindow::togglePage(DockPage* old, DockPage* current)
{
    if (old) {
        hidePage(old);
    }

    if (current) {
        showPage(current);
    }

    updateStyle();
}

void DockWindow::hidePage(DockPage* page)
{
    saveState(page->objectName());

    QList<QWidget*> widgetsToHide;

    DockCentral* central = page->central();
    if (central) {
        DockCentral::Widget cw = central->widget();
        m_central->removeWidget(cw.widget);
        widgetsToHide << cw.widget;
    }

    QList<DockPanel*> panels = page->panels();
    for (DockPanel* panel : panels) {
        DockPanel::Widget dw = panel->widget();
        m_window->removeDockWidget(dw.panel);
        widgetsToHide << dw.panel;
    }

    DockToolBar* tool = page->toolbar();
    if (tool) {
        DockToolBar::Widget tw = tool->widget();
        m_window->removeToolBarBreak(tw.bar);
        m_window->removeToolBar(tw.bar);
        widgetsToHide << tw.bar;
    }

    DockStatusBar* status = page->statusbar();
    if (status) {
        DockStatusBar::Widget sw = status->widget();
        m_statusbar->removeWidget(sw.widget);
        widgetsToHide << sw.widget;
    }

    static QWidget* sDummy = new QWidget();
    for (QWidget* w : widgetsToHide) {
        w->setParent(sDummy);
    }

    m_window->update();
    m_window->repaint();
}

void DockWindow::showPage(DockPage* page)
{
    // ToolBar
    DockToolBar* tool = page->toolbar();
    if (tool) {
        DockToolBar::Widget tw = tool->widget();
        if (tw.breakArea != Qt::NoToolBarArea) {
            m_window->addToolBarBreak(tw.breakArea);
        }
        m_window->addToolBar(tw.bar);
    }

    // StatusBar
    DockStatusBar* status = page->statusbar();
    if (status) {
        DockStatusBar::Widget sw = status->widget();
        m_statusbar->setFixedHeight(sw.widget->height());
        m_statusbar->addWidget(sw.widget, 1);
        connect(status, &DockStatusBar::visibleEdited, this, [this](bool visible) {
            if (m_statusbar->isVisible() != visible) {
                m_statusbar->setVisible(visible);
            }
        });
        m_statusbar->setVisible(status->visible());
        sw.widget->setVisible(status->visible());
    } else {
        m_statusbar->setVisible(false);
    }

    // Panels
    QList<DockPanel*> panels = page->panels();
    for (DockPanel* panel : panels) {
        DockPanel::Widget dw = panel->widget();
        m_window->addDockWidget(dw.area, dw.panel);
    }

    auto panelByName = [](const QList<DockPanel*> panels, const QString& objectName) -> DockPanel* {
        for (DockPanel* panel : panels) {
            if (panel->objectName() == objectName) {
                return panel;
            }
        }
        return nullptr;
    };

    for (DockPanel* panel : panels) {
        DockPanel::Widget dw = panel->widget();
        if (!dw.tabifyObjectName.isEmpty()) {
            DockPanel* tp = panelByName(panels, dw.tabifyObjectName);
            if (!tp) {
                LOGE() << "unable tabify, not found panel with name: " << dw.tabifyObjectName;
                continue;
            }
            m_window->tabifyDockWidget(tp->widget().panel, dw.panel);
        }
    }

    // Central
    DockCentral* central = page->central();
    if (central) {
        DockCentral::Widget cw = central->widget();
        m_central->addWidget(cw.widget);
    }

    restoreState(page->objectName());
}

void DockWindow::updateStyle()
{
    m_window->setStyleSheet(WINDOW_QSS.arg(m_color.name()).arg(m_borderColor.name()));
    m_statusbar->setStyleSheet(STATUS_QSS.arg(m_color.name()).arg(m_borderColor.name()));
    configuration()->applyPlatformStyle(m_window);
}

void DockWindow::onMenusChanged(const QList<QMenu*>& menus)
{
    QMenuBar* menuBar = qMenuBar();
    menuBar->clear();

    for (QMenu* menu: menus) {
        addMenu(menu, menuBar);
    }
}

DockPage* DockWindow::currentPage() const
{
    return page(m_currentPageUri);
}

QString DockWindow::title() const
{
    return m_title;
}

void DockWindow::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_window->setWindowTitle(title);

    m_title = title;
    emit titleChanged(m_title);
}

QColor DockWindow::color() const
{
    return m_color;
}

void DockWindow::setColor(QColor color)
{
    if (m_color == color) {
        return;
    }

    m_color = color;
    emit colorChanged(m_color);
}

QColor DockWindow::borderColor() const
{
    return m_borderColor;
}

DockMenuBar* DockWindow::menuBar() const
{
    return m_menuBar;
}

void DockWindow::setBorderColor(QColor color)
{
    if (m_borderColor == color) {
        return;
    }

    m_borderColor = color;
    emit borderColorChanged(color);
}

QQmlListProperty<DockToolBar> DockWindow::toolbars()
{
    return m_toolbars.property();
}

DockPage* DockWindow::page(const QString& uri) const
{
    for (int i = 0; i < m_pages.count(); ++i) {
        if (m_pages.at(i)->uri() == uri) {
            return m_pages.at(i);
        }
    }
    return nullptr;
}

QQmlListProperty<DockPage> DockWindow::pages()
{
    return m_pages.property();
}

void DockWindow::onPageAppended(int index)
{
    DockPage* page = m_pages.at(index);
    qInfo() << page->uri();
    page->setParentItem(this);
    page->setWidth(this->width());
    page->setHeight(this->height());
}

QString DockWindow::currentPageUri() const
{
    return m_currentPageUri;
}

void DockWindow::setCurrentPageUri(QString uri)
{
    if (m_currentPageUri == uri) {
        return;
    }

    if (m_isComponentComplete) {
        togglePage(page(m_currentPageUri), page(uri));
        adjustPanelsSize(page(uri));
    }

    m_currentPageUri = uri;

    emit currentPageUriChanged(m_currentPageUri);
}

void DockWindow::setMenuBar(DockMenuBar* menuBar)
{
    if (m_menuBar == menuBar) {
        return;
    }

    m_menuBar = menuBar;

    connect(menuBar, &DockMenuBar::changed, this, &DockWindow::onMenusChanged);
    connect(qMenuBar(), &QMenuBar::triggered, menuBar, &DockMenuBar::onActionTriggered);

    emit menuBarChanged(m_menuBar);
}

QMainWindow* DockWindow::qMainWindow()
{
    return m_window;
}

QMenuBar* DockWindow::qMenuBar()
{
#ifdef Q_OS_MAC
    static QMenuBar menuBar;
    return &menuBar;
#else
    return m_window->menuBar();
#endif
}

void DockWindow::stackUnder(QWidget* w)
{
    m_window->stackUnder(w);
}
