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
#include "launcheruriregister.h"

#include "log.h"

using namespace mu::framework;

const QString PATH_KEY("path");
const QString PARAMS_KEY("params");

void LauncherUriRegister::registerUri(const QString& uri, const QString& qmlPath)
{
    if (m_qmlUriHash.contains(uri)) {
        LOGW() << "Qml URI" << uri << "already register. Will be rewrite";
    }

    m_qmlUriHash[uri] = qmlPath;
}

void LauncherUriRegister::registerUri(const QString& uri, int dialogMetaTypeId)
{
    if (m_widgetUriHash.contains(uri)) {
        LOGW() << "Widget URI" << uri << "already register. Will be rewrite";
    }

    m_widgetUriHash[uri] = dialogMetaTypeId;
}

UriType LauncherUriRegister::uriType(const QString& uri) const
{
    if (m_qmlUriHash.contains(uri)) {
        return UriType::Qml;
    } else if (m_widgetUriHash.contains(uri)) {
        return UriType::Widget;
    }

    return UriType::Undefined;
}

QVariantMap LauncherUriRegister::qmlPage(const QString& uri, const QVariantMap& params) const
{
    if (!m_qmlUriHash.contains(uri)) {
        LOGW() << "Qml URI" << uri << "not registered";
        return QVariantMap();
    }

    QVariantMap res;

    res[PATH_KEY] = m_qmlUriHash[uri];
    res[PARAMS_KEY] = params;

    return res;
}

int LauncherUriRegister::widgetDialogMetaTypeId(const QString& uri) const
{
    if (!m_widgetUriHash.contains(uri)) {
        LOGW() << "Widget URI" << uri << "not registered";
        return QMetaType::UnknownType;
    }

    return m_widgetUriHash[uri];
}
