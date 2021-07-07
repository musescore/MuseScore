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

#ifndef __QMLPLUGIN_H__
#define __QMLPLUGIN_H__

#include <QQuickItem>

#include "config.h"

#include "libmscore/mscore.h"
#include "libmscore/musescoreCore.h"
#include "libmscore/utils.h"

namespace Ms {
class MsProcess;
class Score;
class Element;
class MScore;

//---------------------------------------------------------
//   QmlPlugin
//   @@ MuseScore
//   @P menuPath             QString           where the plugin is placed in menu
//   @P filePath             QString           source file path, without the file name (read only)
//   @P version              QString           version of this plugin
//   @P description          QString           human readable description, displayed in Plugin Manager
//   @P pluginType           QString           type may be dialog, dock, or not defined.
//   @P dockArea             QString           where to dock on main screen. left,top,bottom, right(default)
//   @P requiresScore        bool              whether the plugin requires an existing score to run
//   @P division             int               number of MIDI ticks for 1/4 note (read only)
//   @P mscoreVersion        int               complete version number of MuseScore in the form: MMmmuu (read only)
//   @P mscoreMajorVersion   int               1st part of the MuseScore version (read only)
//   @P mscoreMinorVersion   int               2nd part of the MuseScore version (read only)
//   @P mscoreUpdateVersion  int               3rd part of the MuseScore version (read only)
//   @P mscoreDPI            qreal             (read only)
//   @P curScore             Ms::Score*        current score, if any (read only)
//   @P scores               array[Ms::Score]  all currently open scores (read only)
//---------------------------------------------------------

class QmlPlugin : public QQuickItem
{
    Q_OBJECT

    QString _menuPath;
    QString _pluginType;
    QString _dockArea;
    bool _requiresScore = true;
    QString _version;
    QString _description;

protected:
    QString _filePath;              // the path of the source file, without file name
    virtual MuseScoreCore* msc() const = 0;

public slots:
    virtual void endCmd(const QMap<QString, QVariant>&) = 0;

public:
    QmlPlugin(QQuickItem* parent = 0);

    void setMenuPath(const QString& s) { _menuPath = s; }
    QString menuPath() const { return _menuPath; }
    void setVersion(const QString& s) { _version = s; }
    QString version() const { return _version; }
    void setDescription(const QString& s) { _description = s; }
    QString description() const { return _description; }
    void setFilePath(const QString s) { _filePath = s; }
    QString filePath() const { return _filePath; }

    void setPluginType(const QString& s) { _pluginType = s; }
    QString pluginType() const { return _pluginType; }
    void setDockArea(const QString& s) { _dockArea = s; }
    QString dockArea() const { return _dockArea; }
    void setRequiresScore(bool b) { _requiresScore = b; }
    bool requiresScore() const { return _requiresScore; }

    virtual void runPlugin() = 0;

    int division() const { return MScore::division; }
    int mscoreVersion() const { return Ms::version(); }
    int mscoreMajorVersion() const { return majorVersion(); }
    int mscoreMinorVersion() const { return minorVersion(); }
    int mscoreUpdateVersion() const { return patchVersion(); }
    qreal mscoreDPI() const { return DPI; }
};
} // namespace Ms
#endif
