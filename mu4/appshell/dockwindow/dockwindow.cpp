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

using namespace mu::dock;

static const QString windowQss = QString("QMainWindow { background: #808000; } "
                                         "QMainWindow::separator { background: %1; width: 4px; } "
                                         "QTabBar::tab { background: %1; border: 2px solid; padding: 2px; }"
                                         "QTabBar::tab:selected { border-color: #9B9B9B; border-bottom-color: #C2C7CB; }");

static const QString statusQss = QString("QStatusBar { background: %1; } QStatusBar::item { border: 0 }");

DockWindow::DockWindow(QQuickItem* parent) :
    QQuickItem(parent), _pages(this)
{
    setFlag(QQuickItem::ItemHasContents, true);
    _window = new QMainWindow();
    _window->setMinimumSize(800, 600);
    setWidth(1024);
    setHeight(800);

    _eventsWatcher = new EventsWatcher(this);
    _window->installEventFilter(_eventsWatcher);
    connect(_eventsWatcher, &EventsWatcher::eventReceived, this, &DockWindow::onMainWindowEvent);

    _central = new QStackedWidget(_window);
    _window->setCentralWidget(_central);

    _window->setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
    _window->setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);
    _window->setAnimated(false);

    _statusbar = new QStatusBar(_window);
    _statusbar->setSizeGripEnabled(false);
    _window->setStatusBar(_statusbar);

    connect(_pages.notifier(), &QmlListPropertyNotifier::appended, this, &DockWindow::onPageAppended);
    connect(this, &DockWindow::colorChanged, this, &DockWindow::updateStyle);
}

void DockWindow::componentComplete()
{
    QQuickItem::componentComplete();

    updateStyle();

    if (_toolbar) {
        DockToolBar::Widget t = _toolbar->widget();
        t.bar->setParent(_window);
        _window->addToolBar(t.bar);
    }

    togglePage(nullptr, currentPage());

    _window->show();

    _isComponentComplete = true;
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
    qInfo() << "old page: " << (old ? old->objectName() : "null") << "new page: "
            << (current ? current->objectName() : "null");

    if (old) {
        hidePage(old);
    }

    if (current) {
        showPage(current);
    }
}

void DockWindow::hidePage(DockPage* p)
{
    p->setState(_window->saveState());

    QList<QWidget*> widgetsToHide;

    DockCentral* central = p->central();
    if (central) {
        DockCentral::Widget cw = central->widget();
        _central->removeWidget(cw.widget);
        widgetsToHide << cw.widget;
    }

    QList<DockPanel*> panels = p->panels();
    for (DockPanel* panel : panels) {
        DockPanel::Widget dw = panel->widget();
        _window->removeDockWidget(dw.panel);
        widgetsToHide << dw.panel;
    }

    DockToolBar* tool = p->toolbar();
    if (tool) {
        DockToolBar::Widget tw = tool->widget();
        _window->removeToolBarBreak(tw.bar);
        _window->removeToolBar(tw.bar);
        widgetsToHide << tw.bar;
    }

    DockStatusBar* status = p->statusbar();
    if (status) {
        DockStatusBar::Widget sw = status->widget();
        _statusbar->removeWidget(sw.widget);
        widgetsToHide << sw.widget;
    }

    static QWidget* dummy = new QWidget();
    for (QWidget* w : widgetsToHide) {
        w->hide();
        w->setParent(dummy);
    }

    _window->update();
    _window->repaint();
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
            _window->addToolBarBreak(tw.breakArea);
        }
        _window->addToolBar(tw.bar);
        widgetsToShow << tw.bar;
    }

    // StatusBar
    DockStatusBar* status = p->statusbar();
    if (status) {
        DockStatusBar::Widget sw = status->widget();
        _statusbar->setFixedHeight(sw.widget->height());
        _statusbar->addWidget(sw.widget, 1);
        widgetsToShow << sw.widget << _statusbar;
    } else {
        widgetsToHide << _statusbar;
    }

    // Panels
    QList<DockPanel*> panels = p->panels();
    for (DockPanel* panel : panels) {
        DockPanel::Widget dw = panel->widget();
        _window->addDockWidget(dw.area, dw.panel);
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
            _window->tabifyDockWidget(tp->widget().panel, dw.panel);
        }
    }

    // Central
    DockCentral* central = p->central();
    if (central) {
        DockCentral::Widget cw = central->widget();
        _central->addWidget(cw.widget);
        widgetsToShow << cw.widget;
    }

    QByteArray state = p->state();
    if (!state.isEmpty()) {
        _window->restoreState(state);
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
//    _window->setStyleSheet(windowQss.arg(_color.name()));
//    _statusbar->setStyleSheet(statusQss.arg(_color.name()));
}

DockPage* DockWindow::currentPage() const
{
    return page(_currentPageName);
}

QString DockWindow::title() const
{
    return _title;
}

void DockWindow::setTitle(QString title)
{
    if (_title == title) {
        return;
    }

    _window->setWindowTitle(title);

    _title = title;
    emit titleChanged(_title);
}

QColor DockWindow::color() const
{
    return _color;
}

void DockWindow::setColor(QColor color)
{
    if (_color == color) {
        return;
    }

    _color = color;
    emit colorChanged(_color);
}

DockToolBar* DockWindow::toolbar() const
{
    return _toolbar;
}

void DockWindow::setToolbar(DockToolBar* toolbar)
{
    if (_toolbar == toolbar) {
        return;
    }

    _toolbar = toolbar;
    _toolbar->setParentItem(this);
    emit toolbarChanged(_toolbar);
}

DockPage* DockWindow::page(const QString& name) const
{
    for (int i = 0; i < _pages.count(); ++i) {
        if (_pages.at(i)->objectName() == name) {
            return _pages.at(i);
        }
    }
    return nullptr;
}

QQmlListProperty<DockPage> DockWindow::pages()
{
    return _pages.property();
}

void DockWindow::onPageAppended(int index)
{
    DockPage* page = _pages.at(index);
    qInfo() << page->objectName();
    page->setParentItem(this);
    page->setWidth(this->width());
    page->setHeight(this->height());
}

QString DockWindow::currentPageName() const
{
    return _currentPageName;
}

void DockWindow::setCurrentPageName(QString currentPageName)
{
    if (_currentPageName == currentPageName) {
        return;
    }

    if (_isComponentComplete) {
        togglePage(page(_currentPageName), page(currentPageName));
    }

    _currentPageName = currentPageName;
    emit currentPageNameChanged(_currentPageName);
}
