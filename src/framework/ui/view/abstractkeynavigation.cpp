//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "abstractkeynavigation.h"

using namespace mu::ui;

AbstractKeyNavigation::AbstractKeyNavigation(QObject* parent)
    : QObject(parent)
{
}

void AbstractKeyNavigation::setName(QString name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;
    emit nameChanged(m_name);
}

QString AbstractKeyNavigation::name() const
{
    return m_name;
}

const IKeyNavigation::Index& AbstractKeyNavigation::index() const
{
    return m_index;
}

mu::async::Channel<IKeyNavigation::Index> AbstractKeyNavigation::indexChanged() const
{
    return m_indexChanged;
}

void AbstractKeyNavigation::setOrder(int order)
{
    if (m_index.order() == order) {
        return;
    }

    m_index.setOrder(order);
    emit orderChanged(order);

    if (m_indexChanged.isConnected()) {
        m_indexChanged.send(m_index);
    }
}

int AbstractKeyNavigation::order() const
{
    return m_index.order();
}

void AbstractKeyNavigation::setColumn(int column)
{
    if (m_index.column == column) {
        return;
    }

    m_index.column = column;
    emit columnChanged(column);

    if (m_indexChanged.isConnected()) {
        m_indexChanged.send(m_index);
    }
}

int AbstractKeyNavigation::column() const
{
    return m_index.column;
}

void AbstractKeyNavigation::setRow(int row)
{
    if (m_index.row == row) {
        return;
    }

    m_index.row = row;
    emit rowChanged(row);

    if (m_indexChanged.isConnected()) {
        m_indexChanged.send(m_index);
    }
}

int AbstractKeyNavigation::row() const
{
    return m_index.row;
}

void AbstractKeyNavigation::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;
    emit enabledChanged(m_enabled);

    if (m_enabledChanged.isConnected()) {
        m_enabledChanged.send(m_enabled);
    }
}

bool AbstractKeyNavigation::enabled() const
{
    return m_enabled;
}

mu::async::Channel<bool> AbstractKeyNavigation::enabledChanged() const
{
    return m_enabledChanged;
}

void AbstractKeyNavigation::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;
    emit activeChanged(m_active);

    if (m_activeChanged.isConnected()) {
        m_activeChanged.send(m_active);
    }
}

bool AbstractKeyNavigation::active() const
{
    return m_active;
}

mu::async::Channel<bool> AbstractKeyNavigation::activeChanged() const
{
    return m_activeChanged;
}

void AbstractKeyNavigation::classBegin()
{
}

void AbstractKeyNavigation::componentComplete()
{
}
