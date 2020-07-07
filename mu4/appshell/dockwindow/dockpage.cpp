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

#include "dockpage.h"

#include "log.h"

using namespace mu::dock;

DockPage::DockPage(QQuickItem* parent)
    : QQuickItem(parent), _panels(this)
{
    setFlag(QQuickItem::ItemHasContents, true);
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
    return _central;
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
    if (_central == central) {
        return;
    }

    _central = central;
    emit centralChanged(_central);
}

DockToolBar* DockPage::toolbar() const
{
    return _toolbar;
}

void DockPage::setToolbar(DockToolBar* toolbar)
{
    if (_toolbar == toolbar) {
        return;
    }

    _toolbar = toolbar;
    //_toolbar->setParentItem(this);
    emit toolbarChanged(_toolbar);
}

QQmlListProperty<DockPanel> DockPage::panelsProperty()
{
    return _panels.property();
}

QList<DockPanel*> DockPage::panels() const
{
    return _panels.list();
}

DockStatusBar* DockPage::statusbar() const
{
    return _statusbar;
}

void DockPage::setStatusbar(DockStatusBar* statusbar)
{
    if (_statusbar == statusbar) {
        return;
    }

    _statusbar = statusbar;
    emit statusbarChanged(_statusbar);
}

QByteArray DockPage::state() const
{
    return _state;
}

void DockPage::setState(const QByteArray& st)
{
    _state = st;
}
