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

#include "qmlplugin.h"

#include "muversion.h"

#include "engraving/dom/mscore.h"

#include "log.h"

namespace mu::plugins {
//---------------------------------------------------------
//   QmlPlugin
//---------------------------------------------------------

QmlPlugin::QmlPlugin(QQuickItem* parent)
    : QQuickItem(parent)
{}

void QmlPlugin::setMenuPath(const QString&)
{
    NOT_SUPPORTED_USE("title");
}

QString QmlPlugin::menuPath() const
{
    return QString();
}

void QmlPlugin::setTitle(const QString& s)
{
    _title = s;
}

QString QmlPlugin::title() const
{
    return _title;
}

void QmlPlugin::setVersion(const QString& s)
{
    _version = s;
}

QString QmlPlugin::version() const
{
    return _version;
}

void QmlPlugin::setDescription(const QString& s)
{
    _description = s;
}

QString QmlPlugin::description() const
{
    return _description;
}

void QmlPlugin::setFilePath(const QString s)
{
    _filePath = s;
}

QString QmlPlugin::filePath() const
{
    return _filePath;
}

void QmlPlugin::setThumbnailName(const QString& s)
{
    _thumbnailName = s;
}

QString QmlPlugin::thumbnailName() const
{
    return _thumbnailName;
}

void QmlPlugin::setCategoryCode(const QString& s)
{
    _categoryCode = s;
}

QString QmlPlugin::categoryCode() const
{
    return _categoryCode;
}

void QmlPlugin::setPluginType(const QString& s)
{
    _pluginType = s;
}

QString QmlPlugin::pluginType() const
{
    return _pluginType;
}

void QmlPlugin::setDockArea(const QString&)
{
    NOT_SUPPORTED;
}

QString QmlPlugin::dockArea() const
{
    return QString();
}

void QmlPlugin::setRequiresScore(bool b)
{
    _requiresScore = b;
}

bool QmlPlugin::requiresScore() const
{
    return _requiresScore;
}

int QmlPlugin::division() const
{
    return engraving::Constants::DIVISION;
}

int QmlPlugin::mscoreVersion() const
{
    return mscoreMajorVersion() * 10000 + mscoreMinorVersion() * 100 + mscoreUpdateVersion();
}

int QmlPlugin::mscoreMajorVersion() const
{
    return framework::MUVersion::majorVersion();
}

int QmlPlugin::mscoreMinorVersion() const
{
    return framework::MUVersion::minorVersion();
}

int QmlPlugin::mscoreUpdateVersion() const
{
    return framework::MUVersion::patchVersion();
}

qreal QmlPlugin::mscoreDPI() const
{
    return engraving::DPI;
}
}
