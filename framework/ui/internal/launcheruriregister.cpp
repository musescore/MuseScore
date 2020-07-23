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

void LauncherUriRegister::registerUri(const QString& uri, const ContainerMeta &meta)
{
    IF_ASSERT_FAILED(!m_uriHash.contains(uri)) {
        LOGW() << "URI" << uri << "already register. Will be rewrite";
    }

    m_uriHash[uri] = meta;
}

ContainerType LauncherUriRegister::containerType(const QString& uri) const
{
    if (m_uriHash.contains(uri)) {
        return m_uriHash[uri].type;
    }

    return ContainerType::Undefined;
}

ContainerMeta LauncherUriRegister::container(const QString& uri) const
{
    if (!m_uriHash.contains(uri)) {
        LOGW() << "URI" << uri << "not registered";
        return ContainerMeta();
    }

    return m_uriHash[uri];
}
