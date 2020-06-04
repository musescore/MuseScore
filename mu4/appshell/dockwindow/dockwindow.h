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

#ifndef MU_DOCK_DOCKWINDOW_H
#define MU_DOCK_DOCKWINDOW_H

#include <QQuickItem>

#include "qmllistproperty.h"
#include "dockpage.h"

class QMainWindow;
class QStackedWidget;
class QStatusBar;

namespace mu {
namespace dock {
class EventsWatcher;
class DockWindow : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    Q_PROPERTY(DockToolBar * toolbar READ toolbar WRITE setToolbar NOTIFY toolbarChanged)

    Q_PROPERTY(QQmlListProperty<mu::dock::DockPage> pages READ pages)
    Q_PROPERTY(QString currentPageName READ currentPageName WRITE setCurrentPageName NOTIFY currentPageNameChanged)

    Q_CLASSINFO("DefaultProperty", "pages")
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit DockWindow(QQuickItem* parent = nullptr);

    QString title() const;
    QColor color() const;
    DockToolBar* toolbar() const;

    QQmlListProperty<DockPage> pages();
    QString currentPageName() const;

public slots:
    void setTitle(QString title);
    void setColor(QColor color);
    void setToolbar(DockToolBar* toolbar);
    void setCurrentPageName(QString currentPageName);

signals:
    void titleChanged(QString title);
    void colorChanged(QColor color);
    void toolbarChanged(DockToolBar* toolbar);
    void currentPageNameChanged(QString currentPageName);

private slots:
    void onMainWindowEvent(QEvent* e);
    void onPageAppended(int index);
    void updateStyle();

private:

    void componentComplete() override;

    DockPage* page(const QString& name) const;
    DockPage* currentPage() const;

    void togglePage(DockPage* old, DockPage* current);
    void hidePage(DockPage* p);
    void showPage(DockPage* p);

    QMainWindow* _window = nullptr;
    EventsWatcher* _eventsWatcher = nullptr;
    QString _title;
    DockToolBar* _toolbar = nullptr;
    QStackedWidget* _central = nullptr;
    QStatusBar* _statusbar = nullptr;
    QmlListProperty<DockPage> _pages;
    QString _currentPageName;
    bool _isComponentComplete = false;
    QColor _color;
};
}
}

#endif // MU_DOCK_DOCKWINDOW_H
