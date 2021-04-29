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

#include "dockpage.h"

#include "log.h"

using namespace mu::dock;

DockPage::DockPage(QQuickItem* parent)
    : QQuickItem(parent), m_panels(this)
{
    setFlag(QQuickItem::ItemHasContents, true);
    setVisible(false);
}

void DockPage::componentComplete()
{
    if (objectName().isEmpty()) {
        LOGE() << "not set objectName for " << this;
        Q_ASSERT(!objectName().isEmpty());
    }
}

QString DockPage::uri() const
{
    return m_uri;
}

DockCentral* DockPage::central() const
{
    return m_central;
}

void DockPage::setUri(QString uri)
{
    if (m_uri == uri) {
        return;
    }

    m_uri = uri;
    emit uriChanged(m_uri);
}

void DockPage::setCentral(DockCentral* central)
{
    if (m_central == central) {
        return;
    }

    m_central = central;
    emit centralChanged(m_central);
}

DockToolBar* DockPage::toolbar() const
{
    return m_toolbar;
}

void DockPage::setToolbar(DockToolBar* toolbar)
{
    if (m_toolbar == toolbar) {
        return;
    }

    m_toolbar = toolbar;
    //_toolbar->setParentItem(this);
    emit toolbarChanged(m_toolbar);
}

QQmlListProperty<DockPanel> DockPage::panelsProperty()
{
    return m_panels.property();
}

QList<DockPanel*> DockPage::panels() const
{
    return m_panels.list();
}

DockStatusBar* DockPage::statusbar() const
{
    return m_statusbar;
}

void DockPage::setStatusbar(DockStatusBar* statusbar)
{
    if (m_statusbar == statusbar) {
        return;
    }

    m_statusbar = statusbar;
    emit statusbarChanged(m_statusbar);
}

QByteArray DockPage::state() const
{
    return m_state;
}

void DockPage::setState(const QByteArray& state)
{
    m_state = state;
}
