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
#ifndef MU_PLUGINS_PLUGINSTYPES_H
#define MU_PLUGINS_PLUGINSTYPES_H

#include <string>
#include <QUrl>
#include <QVersionNumber>

namespace mu::plugins {
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
    QString categoryCode;
    bool requiresScore = true;
    bool enabled = false;
    bool hasUpdate = false;
    QVersionNumber version;
    std::string shortcuts;

    bool isValid() const { return !codeKey.isEmpty(); }
    bool operator==(const PluginInfo& other) const { return other.codeKey == codeKey; }
};

using PluginInfoMap = std::map<CodeKey, PluginInfo>;
}

#endif // MU_PLUGINS_IPLUGINSCONFIGURATION_H
