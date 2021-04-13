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
#include "abstractkeynavdevitem.h"

using namespace mu::ui;

AbstractKeyNavDevItem::AbstractKeyNavDevItem(IKeyNavigation* keynav)
    : m_keynav(keynav)
{
    m_keynav->indexChanged().onReceive(this, [this](const IKeyNavigation::Index&) {
        emit indexChanged();
    });

    m_keynav->enabledChanged().onReceive(this, [this](bool) {
        emit enabledChanged();
    });

    m_keynav->activeChanged().onReceive(this, [this](bool) {
        emit activeChanged();
    });
}

QString AbstractKeyNavDevItem::name() const
{
    return m_keynav->name();
}

QVariant AbstractKeyNavDevItem::index() const
{
    QVariantMap m;
    m["row"] = m_keynav->index().row;
    m["column"] = m_keynav->index().column;
    return m;
}

bool AbstractKeyNavDevItem::enabled() const
{
    return m_keynav->enabled();
}

bool AbstractKeyNavDevItem::active() const
{
    return m_keynav->active();
}
