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

#include "uicomponents/view/qmllistproperty.h"
#include "uicomponents/uicomponentstypes.h"
#include "dockpage.h"
#include "dockmenubar.h"

#include "ui/imainwindow.h"

class QMainWindow;
class QStackedWidget;
class QStatusBar;

namespace mu::dock {
class EventsWatcher;
class DockWindow : public QQuickItem, public ui::IMainWindow
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)

    Q_PROPERTY(mu::dock::DockMenuBar* menuBar READ menuBar WRITE setMenuBar NOTIFY menuBarChanged)

    Q_PROPERTY(QQmlListProperty<mu::dock::DockToolBar> toolbars READ toolbars)

    Q_PROPERTY(QQmlListProperty<mu::dock::DockPage> pages READ pages)
    Q_PROPERTY(QString currentPageUri READ currentPageUri WRITE setCurrentPageUri NOTIFY currentPageUriChanged)

    Q_CLASSINFO("DefaultProperty", "pages")
    Q_INTERFACES(QQmlParserStatus)

    INJECT(dock, ui::IUiConfiguration, configuration)

public:
    explicit DockWindow(QQuickItem* parent = nullptr);

    QString title() const;
    QColor color() const;
    QColor borderColor() const;

    DockMenuBar* menuBar() const;
    QQmlListProperty<DockToolBar> toolbars();
    QQmlListProperty<DockPage> pages();

    QString currentPageUri() const;

    QMainWindow* qMainWindow() override;
    void stackUnder(QWidget* w) override;

public slots:
    void setTitle(QString title);
    void setColor(QColor color);
    void setBorderColor(QColor color);
    void setCurrentPageUri(QString uri);
    void setMenuBar(DockMenuBar* menuBar);

signals:
    void titleChanged(QString title);
    void colorChanged(QColor color);
    void borderColorChanged(QColor color);
    void currentPageUriChanged(QString currentPageUri);
    void menuBarChanged(DockMenuBar* menuBar);

private slots:
    void onMainWindowEvent(QEvent* event);
    void onPageAppended(int index);
    void updateStyle();
    void onMenusChanged(const QList<QMenu*>& menus);

private:
    void componentComplete() override;

    DockPage* page(const QString& uri) const;
    DockPage* currentPage() const;

    void togglePage(DockPage* old, DockPage* current);
    void hidePage(DockPage* page);
    void showPage(DockPage* page);
    void adjustPanelsSize(DockPage* page);

    void saveState(const QString& pageName);
    void restoreState(const QString& pageName);

    QMainWindow* m_window = nullptr;
    EventsWatcher* m_eventsWatcher = nullptr;
    QString m_title;
    DockMenuBar* m_menuBar = nullptr;
    uicomponents::QmlListProperty<DockToolBar> m_toolbars;
    QStackedWidget* m_central = nullptr;
    QStatusBar* m_statusbar = nullptr;
    uicomponents::QmlListProperty<DockPage> m_pages;
    QString m_currentPageUri;
    bool m_isComponentComplete = false;
    QColor m_color;
    QColor m_borderColor;
};
}

#endif // MU_DOCK_DOCKWINDOW_H
