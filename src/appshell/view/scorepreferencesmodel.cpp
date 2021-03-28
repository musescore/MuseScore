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
#include "scorepreferencesmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::appshell;

ScorePreferencesModel::ScorePreferencesModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ScorePreferencesModel::rowCount(const QModelIndex&) const
{
    return m_defaultFiles.count();
}

QVariant ScorePreferencesModel::data(const QModelIndex& index, int role) const
{
    const DefaultFileInfo& file = m_defaultFiles.at(index.row());
    switch (role) {
    case TitleRole: return file.title;
    case PathRole: return file.path;
    case PathFilterRole: return file.pathFilter;
    case ChooseTitleRole: return file.chooseTitle;
    }

    return QVariant();
}

bool ScorePreferencesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const DefaultFileInfo file = m_defaultFiles.at(index.row());

    switch (role) {
    case PathRole:
        if (file.path == value.toString()) {
            return false;
        }

        savePath(file.type, value.toString());
        emit dataChanged(index, index, { PathRole });
        return true;
    default:
        break;
    }

    return false;
}

QHash<int, QByteArray> ScorePreferencesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { TitleRole, "title" },
        { PathRole, "path" },
        { PathFilterRole, "pathFilter" },
        { ChooseTitleRole, "chooseTitle" }
    };

    return roles;
}

void ScorePreferencesModel::load()
{
    beginResetModel();

    m_defaultFiles = {
        { DefaultFileType::FirstInstrumentList, qtrc("appshell", "Instrument list 1"), firstInstrumentListPath(),
          instrumentPathFilter(), instrumentChooseTitle() },
        { DefaultFileType::SecondInstrumentList, qtrc("appshell", "Instrument list 2"), secondInstrumentListPath(),
          instrumentPathFilter(), instrumentChooseTitle() },
        { DefaultFileType::FirstScoreOrderList, qtrc("appshell", "Score order list 1"), firstScoreOrderListPath(),
          scoreOrderPathFilter(), scoreOrderChooseTitle() },
        { DefaultFileType::SecondScoreOrderList, qtrc("appshell", "Score order list 2"), secondScoreOrderListPath(),
          scoreOrderPathFilter(), scoreOrderChooseTitle() },
        { DefaultFileType::Style, qtrc("appshell", "Style"), stylePath(),
          stylePathFilter(), styleChooseTitle() },
        { DefaultFileType::PartStyle, qtrc("appshell", "Style for part"), partStylePath(),
          stylePathFilter(), partStyleChooseTitle() },
    };

    endResetModel();
}

QString ScorePreferencesModel::fileDirectory(const QString& filePath) const
{
    return io::dirpath(filePath.toStdString()).toQString();
}

bool ScorePreferencesModel::isShowMIDIControls() const
{
    return audioConfiguration()->isShowControlsInMixer();
}

void ScorePreferencesModel::setIsShowMIDIControls(bool value)
{
    if (isShowMIDIControls() == value) {
        return;
    }

    audioConfiguration()->setIsShowControlsInMixer(value);
    emit isShowMIDIControlsChanged(value);
}

void ScorePreferencesModel::savePath(ScorePreferencesModel::DefaultFileType fileType, const QString& path)
{
    switch (fileType) {
    case DefaultFileType::FirstInstrumentList:
        setFirstInstrumentListPath(path);
        break;
    case DefaultFileType::SecondInstrumentList:
        setSecondInstrumentListPath(path);
        break;
    case DefaultFileType::FirstScoreOrderList:
        setFirstScoreOrderListPath(path);
        break;
    case DefaultFileType::SecondScoreOrderList:
        setSecondScoreOrderListPath(path);
        break;
    case DefaultFileType::Style:
        notationConfiguration()->setDefaultStyleFilePath(path.toStdString());
        break;
    case DefaultFileType::PartStyle:
        notationConfiguration()->setPartStyleFilePath(path.toStdString());
        break;
    case DefaultFileType::Undefined:
        break;
    }

    setPath(fileType, path);
}

QString ScorePreferencesModel::firstInstrumentListPath() const
{
    io::paths instrumentListPaths = instrumentsConfiguration()->userInstrumentListPaths();
    if (instrumentListPaths.empty()) {
        return QString();
    }

    return instrumentListPaths[0].toQString();
}

void ScorePreferencesModel::setFirstInstrumentListPath(const QString& path)
{
    io::paths instrumentListPaths = instrumentsConfiguration()->userInstrumentListPaths();
    if (instrumentListPaths.empty()) {
        return;
    }

    instrumentListPaths[0] = path.toStdString();
    instrumentsConfiguration()->setUserInstrumentListPaths(instrumentListPaths);
}

QString ScorePreferencesModel::secondInstrumentListPath() const
{
    io::paths instrumentListPaths = instrumentsConfiguration()->userInstrumentListPaths();
    if (instrumentListPaths.size() < 1) {
        return QString();
    }

    return instrumentListPaths[1].toQString();
}

void ScorePreferencesModel::setSecondInstrumentListPath(const QString& path)
{
    io::paths instrumentListPaths = instrumentsConfiguration()->userInstrumentListPaths();
    if (instrumentListPaths.size() < 1) {
        return;
    }

    instrumentListPaths[1] = path.toStdString();
    instrumentsConfiguration()->setUserInstrumentListPaths(instrumentListPaths);
}

QString ScorePreferencesModel::firstScoreOrderListPath() const
{
    io::paths scoreOrderListPaths = instrumentsConfiguration()->userScoreOrderListPaths();
    if (scoreOrderListPaths.empty()) {
        return QString();
    }

    return scoreOrderListPaths[0].toQString();
}

void ScorePreferencesModel::setFirstScoreOrderListPath(const QString& path)
{
    io::paths scoreOrderListPaths = instrumentsConfiguration()->userScoreOrderListPaths();
    if (scoreOrderListPaths.empty()) {
        return;
    }

    scoreOrderListPaths[0] = path.toStdString();
    instrumentsConfiguration()->setUserScoreOrderListPaths(scoreOrderListPaths);
}

QString ScorePreferencesModel::secondScoreOrderListPath() const
{
    io::paths scoreOrderListPaths = instrumentsConfiguration()->userScoreOrderListPaths();
    if (scoreOrderListPaths.size() < 1) {
        return QString();
    }

    return scoreOrderListPaths[1].toQString();
}

void ScorePreferencesModel::setSecondScoreOrderListPath(const QString& path)
{
    io::paths scoreOrderListPaths = instrumentsConfiguration()->userScoreOrderListPaths();
    if (scoreOrderListPaths.size() < 1) {
        return;
    }

    scoreOrderListPaths[1] = path.toStdString();
    instrumentsConfiguration()->setUserScoreOrderListPaths(scoreOrderListPaths);
}

QString ScorePreferencesModel::stylePath() const
{
    return notationConfiguration()->defaultStyleFilePath().toQString();
}

QString ScorePreferencesModel::partStylePath() const
{
    return notationConfiguration()->partStyleFilePath().toQString();
}

QString ScorePreferencesModel::instrumentPathFilter() const
{
    return qtrc("appshell", "Instrument List") + " (*.xml)";
}

QString ScorePreferencesModel::scoreOrderPathFilter() const
{
    return qtrc("appshell", "Score Order List") + " (*.xml)";
}

QString ScorePreferencesModel::stylePathFilter() const
{
    return qtrc("appshell", "MuseScore Style File") + " (*.mss)";
}

QString ScorePreferencesModel::instrumentChooseTitle() const
{
    return qtrc("appshell", "Choose Instrument List");
}

QString ScorePreferencesModel::scoreOrderChooseTitle() const
{
    return qtrc("appshell", "Choose Score Order List");
}

QString ScorePreferencesModel::styleChooseTitle() const
{
    return qtrc("appshell", "Choose Default Style");
}

QString ScorePreferencesModel::partStyleChooseTitle() const
{
    return qtrc("appshell", "Choose Default Style for Parts");
}

void ScorePreferencesModel::setPath(ScorePreferencesModel::DefaultFileType fileType, const QString& path)
{
    QModelIndex index = fileIndex(fileType);
    if (!index.isValid()) {
        return;
    }

    m_defaultFiles[index.row()].path = path;
    emit dataChanged(index, index, { PathRole });
}

QModelIndex ScorePreferencesModel::fileIndex(ScorePreferencesModel::DefaultFileType fileType)
{
    for (int i = 0; i < m_defaultFiles.count(); ++i) {
        DefaultFileInfo& fileInfo = m_defaultFiles[i];
        if (fileInfo.type == fileType) {
            return index(i, 0);
        }
    }

    return QModelIndex();
}
