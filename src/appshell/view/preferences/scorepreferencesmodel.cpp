/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "scorepreferencesmodel.h"

#include "log.h"
#include "translation.h"

using namespace muse;
using namespace mu::appshell;

ScorePreferencesModel::ScorePreferencesModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
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
    case DirectoryRole: return fileDirectory(file.path);
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
        { ChooseTitleRole, "chooseTitle" },
        { DirectoryRole, "directory" }
    };

    return roles;
}

void ScorePreferencesModel::load()
{
    beginResetModel();

    m_defaultFiles = {
        { DefaultFileType::FirstScoreOrderList, muse::qtrc("appshell/preferences", "Score order list 1"), firstScoreOrderListPath(),
          scoreOrderPathFilter(), scoreOrderChooseTitle() },
        { DefaultFileType::SecondScoreOrderList, muse::qtrc("appshell/preferences", "Score order list 2"), secondScoreOrderListPath(),
          scoreOrderPathFilter(), scoreOrderChooseTitle() },
        { DefaultFileType::Style, muse::qtrc("appshell/preferences", "Style"), stylePath(),
          stylePathFilter(), styleChooseTitle() },
        { DefaultFileType::PartStyle, muse::qtrc("appshell/preferences", "Style for part"), partStylePath(),
          stylePathFilter(), partStyleChooseTitle() },
        { DefaultFileType::PaletteStyle, muse::qtrc("appshell/preferences", "Style for palette"), paletteStylePath(),
          stylePathFilter(), paletteStyleChooseTitle() },
    };

    endResetModel();

    notationConfiguration()->scoreOrderListPathsChanged().onNotify(this, [this]() {
        setPath(DefaultFileType::FirstScoreOrderList, firstScoreOrderListPath());
        setPath(DefaultFileType::SecondScoreOrderList, secondScoreOrderListPath());
    });

    notationConfiguration()->defaultStyleFilePathChanged().onReceive(this, [this](const io::path_t& val) {
        setPath(DefaultFileType::Style, val.toQString());
    });

    notationConfiguration()->partStyleFilePathChanged().onReceive(this, [this](const io::path_t& val) {
        setPath(DefaultFileType::PartStyle, val.toQString());
    });

    notationConfiguration()->paletteStyleFilePathChanged().onReceive(this, [this](const io::path_t& val) {
        setPath(DefaultFileType::PaletteStyle, val.toQString());
    });
}

void ScorePreferencesModel::savePath(ScorePreferencesModel::DefaultFileType fileType, const QString& path)
{
    switch (fileType) {
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
    case DefaultFileType::PaletteStyle:
        notationConfiguration()->setPaletteStyleFilePath(path.toStdString());
        break;
    case DefaultFileType::Undefined:
        break;
    }

    setPath(fileType, path);
}

QString ScorePreferencesModel::firstScoreOrderListPath() const
{
    io::paths_t scoreOrderListPaths = notationConfiguration()->userScoreOrderListPaths();
    if (scoreOrderListPaths.empty()) {
        return QString();
    }

    return scoreOrderListPaths[0].toQString();
}

void ScorePreferencesModel::setFirstScoreOrderListPath(const QString& path)
{
    io::paths_t scoreOrderListPaths = notationConfiguration()->userScoreOrderListPaths();
    if (scoreOrderListPaths.empty()) {
        return;
    }

    scoreOrderListPaths[0] = path.toStdString();
    notationConfiguration()->setUserScoreOrderListPaths(scoreOrderListPaths);
}

QString ScorePreferencesModel::secondScoreOrderListPath() const
{
    io::paths_t scoreOrderListPaths = notationConfiguration()->userScoreOrderListPaths();
    if (scoreOrderListPaths.size() < 1) {
        return QString();
    }

    return scoreOrderListPaths[1].toQString();
}

void ScorePreferencesModel::setSecondScoreOrderListPath(const QString& path)
{
    io::paths_t scoreOrderListPaths = notationConfiguration()->userScoreOrderListPaths();
    if (scoreOrderListPaths.size() < 1) {
        return;
    }

    scoreOrderListPaths[1] = path.toStdString();
    notationConfiguration()->setUserScoreOrderListPaths(scoreOrderListPaths);
}

QString ScorePreferencesModel::stylePath() const
{
    return notationConfiguration()->defaultStyleFilePath().toQString();
}

QString ScorePreferencesModel::partStylePath() const
{
    return notationConfiguration()->partStyleFilePath().toQString();
}

QString ScorePreferencesModel::paletteStylePath() const
{
    return notationConfiguration()->paletteStyleFilePath().toQString();
}

QStringList ScorePreferencesModel::scoreOrderPathFilter() const
{
    return { muse::qtrc("appshell/preferences", "Score order list") + " (*.xml)" };
}

QStringList ScorePreferencesModel::stylePathFilter() const
{
    return { muse::qtrc("appshell/preferences", "MuseScore style file") + " (*.mss)" };
}

QString ScorePreferencesModel::scoreOrderChooseTitle() const
{
    return muse::qtrc("appshell/preferences", "Choose score order list");
}

QString ScorePreferencesModel::styleChooseTitle() const
{
    return muse::qtrc("appshell/preferences", "Choose default style");
}

QString ScorePreferencesModel::partStyleChooseTitle() const
{
    return muse::qtrc("appshell/preferences", "Choose default style for parts");
}

QString ScorePreferencesModel::paletteStyleChooseTitle() const
{
    return muse::qtrc("appshell/preferences", "Choose default style for palette");
}

void ScorePreferencesModel::setPath(ScorePreferencesModel::DefaultFileType fileType, const QString& path)
{
    QModelIndex index = fileIndex(fileType);
    if (!index.isValid()) {
        return;
    }

    m_defaultFiles[index.row()].path = path;
    emit dataChanged(index, index, { PathRole, DirectoryRole });
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

QString ScorePreferencesModel::fileDirectory(const QString& filePath) const
{
    return io::dirpath(filePath.toStdString()).toQString();
}
