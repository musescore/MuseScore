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
#ifndef MU_PLUGINS_PLUGINSTYPES_H
#define MU_PLUGINS_PLUGINSTYPES_H

#include <string>
#include <QUrl>
#include <QVersionNumber>

#include "io/path.h"

namespace mu {
namespace plugins {
using CodeKey = QString;
using CodeKeyList = QList<CodeKey>;

struct PluginInfo
{
    CodeKey codeKey;
    QUrl url;
    QUrl thumbnailUrl;
    QUrl detailsUrl;
    QString name;
    QString description;
    QString category;
    bool installed = false;
    bool hasUpdate = false;
    QVersionNumber version;

    bool isValid() const { return !codeKey.isEmpty(); }
    bool operator==(const PluginInfo& other) const { return other.codeKey == codeKey; }
};

using PluginInfoList = QList<PluginInfo>;
}
}

#endif // MU_PLUGINS_IPLUGINSCONFIGURATION_H
