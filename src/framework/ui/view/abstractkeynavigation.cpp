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

void AbstractKeyNavigation::setOrder(int order)
{
    if (m_order == order) {
        return;
    }

    m_order = order;
    emit orderChanged(m_order);
}

int AbstractKeyNavigation::order() const
{
    return m_order;
}

void AbstractKeyNavigation::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;
    emit enabledChanged(m_enabled);
}

bool AbstractKeyNavigation::enabled() const
{
    return m_enabled;
}

void AbstractKeyNavigation::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;
    emit activeChanged(m_active);
    m_activeChanged.send(m_active);
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
