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

#ifndef MU_DOCK_DOCKPAGE_H
#define MU_DOCK_DOCKPAGE_H

#include <QQuickItem>

#include "uicomponents/view/qmllistproperty.h"
#include "dockcentral.h"
#include "docktoolbar.h"
#include "dockpanel.h"
#include "dockstatusbar.h"

namespace mu {
namespace dock {
class DockPage : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(DockToolBar * toolbar READ toolbar WRITE setToolbar NOTIFY toolbarChanged)
    Q_PROPERTY(DockCentral * central READ central WRITE setCentral NOTIFY centralChanged)
    Q_PROPERTY(QQmlListProperty<mu::dock::DockPanel> panels READ panelsProperty)
    Q_PROPERTY(DockStatusBar * statusbar READ statusbar WRITE setStatusbar NOTIFY statusbarChanged)

public:
    explicit DockPage(QQuickItem* parent = nullptr);

    QString uri() const;
    DockCentral* central() const;
    DockToolBar* toolbar() const;
    QQmlListProperty<DockPanel> panelsProperty();
    QList<DockPanel*> panels() const;
    DockStatusBar* statusbar() const;

    QByteArray state() const;
    void setState(const QByteArray& st);

public slots:
    void setUri(QString uri);
    void setCentral(DockCentral* central);
    void setToolbar(DockToolBar* toolbar);
    void setStatusbar(DockStatusBar* statusbar);

signals:
    void uriChanged(QString uri);
    void centralChanged(DockCentral* central);
    void toolbarChanged(DockToolBar* toolbar);
    void statusbarChanged(DockStatusBar* statusbar);

private:

    void componentComplete() override;

    QString m_uri;
    DockCentral* _central = nullptr;
    DockToolBar* _toolbar = nullptr;
    framework::QmlListProperty<DockPanel> _panels;
    DockStatusBar* _statusbar = nullptr;

    QByteArray _state;
};
}
}

#endif // MU_DOCK_DOCKPAGE_H
