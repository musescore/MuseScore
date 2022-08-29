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

namespace mu::engraving {
class EngravingItem;
class MScore;
class MsProcess;
class Score;

//---------------------------------------------------------
//   QmlPlugin
//   @@ MuseScore
//   @P title                QString           the title of this plugin
//   @P menuPath             QString           where the plugin is placed in menu
//   @P filePath             QString           source file path, without the file name (read only)
//   @P version              QString           version of this plugin
//   @P description          QString           human readable description, displayed in Plugin Manager
//   @P pluginType           QString           type may be dialog, dock, or not defined.
//   @P dockArea             QString           where to dock on main screen. left,top,bottom, right(default)
//   @P requiresScore        bool              whether the plugin requires an existing score to run
//   @P thumbnailName        QString           the thumbnail of this plugin
//   @P categoryCode         QString           the code of category this plugin belongs to
//   @P division             int               number of MIDI ticks for 1/4 note (read only)
//   @P mscoreVersion        int               complete version number of MuseScore in the form: MMmmuu (read only)
//   @P mscoreMajorVersion   int               1st part of the MuseScore version (read only)
//   @P mscoreMinorVersion   int               2nd part of the MuseScore version (read only)
//   @P mscoreUpdateVersion  int               3rd part of the MuseScore version (read only)
//   @P mscoreDPI            qreal             (read only)
//   @P curScore             mu::engraving::Score*        current score, if any (read only)
//   @P scores               array[mu::engraving::Score]  all currently open scores (read only)
//---------------------------------------------------------

class QmlPlugin : public QQuickItem
{
    Q_OBJECT

    QString _title;
    QString _pluginType;
    bool _requiresScore = true;
    QString _version;
    QString _description;
    QString _thumbnailName;
    QString _categoryCode;

protected:
    QString _filePath;              // the path of the source file, without file name

public slots:
    virtual void endCmd(const QMap<QString, QVariant>&) = 0;

signals:
    void closeRequested();

public:
    QmlPlugin(QQuickItem* parent = 0);

    void setMenuPath(const QString& s);
    QString menuPath() const;
    void setTitle(const QString& s);
    QString title() const;
    void setVersion(const QString& s);
    QString version() const;
    void setDescription(const QString& s);
    QString description() const;
    void setFilePath(const QString s);
    QString filePath() const;
    void setThumbnailName(const QString& s);
    QString thumbnailName() const;
    void setCategoryCode(const QString& s);
    QString categoryCode() const;
    void setPluginType(const QString& s);
    QString pluginType() const;
    void setDockArea(const QString& s);
    QString dockArea() const;
    void setRequiresScore(bool b);
    bool requiresScore() const;

    virtual void runPlugin() = 0;

    int division() const;
    int mscoreVersion() const;
    int mscoreMajorVersion() const;
    int mscoreMinorVersion() const;
    int mscoreUpdateVersion() const;
    qreal mscoreDPI() const;
};
} // namespace Ms
#endif
