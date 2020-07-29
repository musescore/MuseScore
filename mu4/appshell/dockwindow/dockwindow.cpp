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

#include "log.h"
#include "eventswatcher.h"
#include "modularity/ioc.h"

using namespace mu::dock;

static const QString windowQss = QString("QMainWindow { background: %1; } "
                                         "QMainWindow::separator { background: %1; width: 4px; } "
                                         "QTabBar::tab { background: %1; border: 2px solid; padding: 2px; }"
                                         "QTabBar::tab:selected { border-color: #9B9B9B; border-bottom-color: #C2C7CB; }");

static const QString statusQss = QString("QStatusBar { background: %1; } QStatusBar::item { border: 0 }");

DockWindow::DockWindow(QQuickItem* parent)
    : QQuickItem(parent), m_toolbars(this), m_pages(this)
{
    framework::ioc()->registerExportNoDelete<framework::IMainWindow>("dock", this);

    setFlag(QQuickItem::ItemHasContents, true);
    m_window = new QMainWindow();
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

    connect(m_pages.notifier(), &framework::QmlListPropertyNotifier::appended, this, &DockWindow::onPageAppended);
    connect(this, &DockWindow::colorChanged, this, &DockWindow::updateStyle);
}

void DockWindow::componentComplete()
{
    QQuickItem::componentComplete();

    updateStyle();

    for (DockToolBar* t : m_toolbars.list()) {
        DockToolBar::Widget tw = t->widget();
        tw.bar->setParent(m_window);
        m_window->addToolBar(tw.bar);
    }

    togglePage(nullptr, currentPage());

    m_window->show();

    m_isComponentComplete = true;
}

void DockWindow::onMainWindowEvent(QEvent* e)
{
    if (QEvent::Resize == e->type()) {
        QResizeEvent* re = static_cast<QResizeEvent*>(e);
        setSize(QSizeF(re->size()));
    }
}

void DockWindow::togglePage(DockPage* old, DockPage* current)
{
//    qInfo() << "old page: " << (old ? old->uri() : "null") << "new page: "
//            << (current ? current->uri() : "null");

    if (old) {
        hidePage(old);
    }

    if (current) {
        showPage(current);
    }
}

void DockWindow::hidePage(DockPage* p)
{
    p->setState(m_window->saveState());

    QList<QWidget*> widgetsToHide;

    DockCentral* central = p->central();
    if (central) {
        DockCentral::Widget cw = central->widget();
        m_central->removeWidget(cw.widget);
        widgetsToHide << cw.widget;
    }

    QList<DockPanel*> panels = p->panels();
    for (DockPanel* panel : panels) {
        DockPanel::Widget dw = panel->widget();
        m_window->removeDockWidget(dw.panel);
        widgetsToHide << dw.panel;
    }

    DockToolBar* tool = p->toolbar();
    if (tool) {
        DockToolBar::Widget tw = tool->widget();
        m_window->removeToolBarBreak(tw.bar);
        m_window->removeToolBar(tw.bar);
        widgetsToHide << tw.bar;
    }

    DockStatusBar* status = p->statusbar();
    if (status) {
        DockStatusBar::Widget sw = status->widget();
        m_statusbar->removeWidget(sw.widget);
        widgetsToHide << sw.widget;
    }

    static QWidget* dummy = new QWidget();
    for (QWidget* w : widgetsToHide) {
        w->hide();
        w->setParent(dummy);
    }

    m_window->update();
    m_window->repaint();
}

void DockWindow::showPage(DockPage* p)
{
    QList<QWidget*> widgetsToShow;
    QList<QWidget*> widgetsToHide;

    // ToolBar
    DockToolBar* tool = p->toolbar();
    if (tool) {
        DockToolBar::Widget tw = tool->widget();
        if (tw.breakArea != Qt::NoToolBarArea) {
            m_window->addToolBarBreak(tw.breakArea);
        }
        m_window->addToolBar(tw.bar);
        widgetsToShow << tw.bar;
    }

    // StatusBar
    DockStatusBar* status = p->statusbar();
    if (status) {
        DockStatusBar::Widget sw = status->widget();
        m_statusbar->setFixedHeight(sw.widget->height());
        m_statusbar->addWidget(sw.widget, 1);
        widgetsToShow << sw.widget << m_statusbar;
    } else {
        widgetsToHide << m_statusbar;
    }

    // Panels
    QList<DockPanel*> panels = p->panels();
    for (DockPanel* panel : panels) {
        DockPanel::Widget dw = panel->widget();
        m_window->addDockWidget(dw.area, dw.panel);
        widgetsToShow << dw.panel;
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
    DockCentral* central = p->central();
    if (central) {
        DockCentral::Widget cw = central->widget();
        m_central->addWidget(cw.widget);
        widgetsToShow << cw.widget;
    }

    QByteArray state = p->state();
    if (!state.isEmpty()) {
        m_window->restoreState(state);
    }

    for (QWidget* w : widgetsToShow) {
        w->show();
    }

    for (QWidget* w : widgetsToHide) {
        w->hide();
    }
}

void DockWindow::updateStyle()
{
    m_window->setStyleSheet(windowQss.arg(m_color.name()));
    m_statusbar->setStyleSheet(statusQss.arg(m_color.name()));
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
    }

    m_currentPageUri = uri;

    emit currentPageUriChanged(m_currentPageUri);
}

QMainWindow* DockWindow::qMainWindow()
{
    return m_window;
}

void DockWindow::stackUnder(QWidget* w)
{
    m_window->stackUnder(w);
}
