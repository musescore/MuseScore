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

#ifndef MU_DOCK_DOCKPAGE_H
#define MU_DOCK_DOCKPAGE_H

#include <QQuickItem>

#include "uicomponents/view/qmllistproperty.h"
#include "dockcentral.h"
#include "docktoolbar.h"
#include "dockpanel.h"
#include "dockstatusbar.h"

namespace mu::dock {
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
    void setState(const QByteArray& state);

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
    DockCentral* m_central = nullptr;
    DockToolBar* m_toolbar = nullptr;
    uicomponents::QmlListProperty<DockPanel> m_panels;
    DockStatusBar* m_statusbar = nullptr;

    QByteArray m_state;
};
}

#endif // MU_DOCK_DOCKPAGE_H
