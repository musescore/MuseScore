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
#include "qmllaunchprovider.h"

using namespace mu::framework;

QmlLaunchProvider::QmlLaunchProvider(QObject* parent)
    : QObject(parent)
{
}

void QmlLaunchProvider::open(const UriQuery& uri)
{
    m_currentUriQuery = uri;
    emit fireOpen(toQVariantMap(uri));
}

QVariantMap QmlLaunchProvider::toQVariantMap(const UriQuery& q) const
{
    QVariantMap data;
    data["uri"] = QString::fromStdString(q.uri().toString());

    QVariantMap params;
    const UriQuery::Params& p = q.params();
    for (auto it = p.cbegin(); it != p.cend(); ++it) {
        params[QString::fromStdString(it->first)] = it->second.toQVariant();
    }

    data["params"] = params;

    return data;
}

mu::Uri QmlLaunchProvider::currentUri() const
{
    return m_currentUriQuery.uri();
}
