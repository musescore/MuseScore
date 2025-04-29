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
#include "folderspreferencesmodel.h"

#include "translation.h"

#include "muse_framework_config.h"

using namespace mu::appshell;
using namespace muse;

FoldersPreferencesModel::FoldersPreferencesModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

int FoldersPreferencesModel::rowCount(const QModelIndex&) const
{
    return m_folders.count();
}

QVariant FoldersPreferencesModel::data(const QModelIndex& index, int role) const
{
    const FolderInfo& folder = m_folders.at(index.row());
    switch (role) {
    case TitleRole: return folder.title;
    case PathRole: return folder.value;
    case DirRole: return folder.dir;
    case IsMultiDirectoriesRole: return folder.valueType == FolderValueType::MultiDirectories;
    }

    return QVariant();
}

bool FoldersPreferencesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const FolderInfo& folder = m_folders.at(index.row());

    switch (role) {
    case PathRole:
        if (folder.value == value.toString()) {
            return false;
        }

        saveFolderPaths(folder.type, value.toString());
        return true;
    default:
        break;
    }

    return false;
}

QHash<int, QByteArray> FoldersPreferencesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { TitleRole, "title" },
        { PathRole, "path" },
        { DirRole, "dir" },
        { IsMultiDirectoriesRole, "isMultiDirectories" }
    };

    return roles;
}

void FoldersPreferencesModel::load()
{
    beginResetModel();

    m_folders = {
        {
            FolderType::Scores, muse::qtrc("appshell/preferences", "Scores"), projectConfiguration()->userProjectsPath().toQString(),
            projectConfiguration()->userProjectsPath().toQString()
        },
        {
            FolderType::Styles, muse::qtrc("appshell/preferences", "Styles"), notationConfiguration()->userStylesPath().toQString(),
            notationConfiguration()->userStylesPath().toQString()
        },
        {
            FolderType::Templates, muse::qtrc("appshell/preferences", "Templates"), projectConfiguration()->userTemplatesPath().toQString(),
            projectConfiguration()->userTemplatesPath().toQString()
        },
        {
            FolderType::Plugins, muse::qtrc("appshell/preferences", "Plugins"), extensionsConfiguration()->pluginsUserPath().toQString(),
            extensionsConfiguration()->pluginsUserPath().toQString()
        },
        {
            FolderType::SoundFonts, muse::qtrc("appshell/preferences", "SoundFonts"), pathsToString(
                audioConfiguration()->userSoundFontDirectories()),
            configuration()->userDataPath().toQString(), FolderValueType::MultiDirectories
        },
        {
            FolderType::MusicFonts, muse::qtrc("appshell/preferences", "Musical symbol fonts"),
            notationConfiguration()->userMusicFontsPath().toQString(),
            notationConfiguration()->userMusicFontsPath().toQString()
        },
#ifdef MUSE_MODULE_VST
        {
            FolderType::VST3, muse::qtrc("appshell/preferences", "VST3"), pathsToString(vstConfiguration()->userVstDirectories()),
            configuration()->userDataPath().toQString(), FolderValueType::MultiDirectories
        }
#endif
    };

    endResetModel();

    setupConnections();
}

void FoldersPreferencesModel::setupConnections()
{
    projectConfiguration()->userProjectsPathChanged().onReceive(this, [this](const muse::io::path_t& path) {
        setFolderPaths(FolderType::Scores, path.toQString());
    });

    notationConfiguration()->userStylesPathChanged().onReceive(this, [this](const muse::io::path_t& path) {
        setFolderPaths(FolderType::Styles, path.toQString());
    });

    projectConfiguration()->userTemplatesPathChanged().onReceive(this, [this](const muse::io::path_t& path) {
        setFolderPaths(FolderType::Templates, path.toQString());
    });

    extensionsConfiguration()->pluginsUserPathChanged().onReceive(this, [this](const muse::io::path_t& path) {
        setFolderPaths(FolderType::Plugins, path.toQString());
    });

    audioConfiguration()->soundFontDirectoriesChanged().onReceive(this, [this](const io::paths_t&) {
        io::paths_t userSoundFontsPaths = audioConfiguration()->userSoundFontDirectories();
        setFolderPaths(FolderType::SoundFonts, pathsToString(userSoundFontsPaths));
    });

    notationConfiguration()->userMusicFontsPathChanged().onReceive(this, [this](const muse::io::path_t& path) {
        setFolderPaths(FolderType::MusicFonts, path.toQString());
    });

    vstConfiguration()->userVstDirectoriesChanged().onReceive(this, [this](const io::paths_t& paths) {
        setFolderPaths(FolderType::VST3, pathsToString(paths));
    });
}

void FoldersPreferencesModel::saveFolderPaths(FoldersPreferencesModel::FolderType folderType, const QString& paths)
{
    switch (folderType) {
    case FolderType::Scores: {
        muse::io::path_t folderPath = paths.toStdString();
        projectConfiguration()->setUserProjectsPath(folderPath);
        break;
    }
    case FolderType::Styles: {
        muse::io::path_t folderPath = paths.toStdString();
        notationConfiguration()->setUserStylesPath(folderPath);
        break;
    }
    case FolderType::Templates: {
        muse::io::path_t folderPath = paths.toStdString();
        projectConfiguration()->setUserTemplatesPath(folderPath);
        break;
    }
    case FolderType::Plugins: {
        muse::io::path_t folderPath = paths.toStdString();
        extensionsConfiguration()->setUserPluginsPath(folderPath);
        break;
    }
    case FolderType::SoundFonts: {
        audioConfiguration()->setUserSoundFontDirectories(pathsFromString(paths));
        break;
    }
    case FolderType::MusicFonts: {
        notationConfiguration()->setUserMusicFontsPath(paths.toStdString());
        break;
    }
    case FolderType::VST3: {
        vstConfiguration()->setUserVstDirectories(pathsFromString(paths));
        break;
    }
    case FolderType::Undefined:
        break;
    }
}

void FoldersPreferencesModel::setFolderPaths(FoldersPreferencesModel::FolderType folderType, const QString& paths)
{
    QModelIndex index = folderIndex(folderType);
    if (!index.isValid()) {
        return;
    }

    m_folders[index.row()].value = paths;
    emit dataChanged(index, index, { PathRole });
}

QModelIndex FoldersPreferencesModel::folderIndex(FoldersPreferencesModel::FolderType folderType)
{
    for (int i = 0; i < m_folders.count(); ++i) {
        FolderInfo& folderInfo = m_folders[i];
        if (folderInfo.type == folderType) {
            return index(i, 0);
        }
    }

    return QModelIndex();
}

QString FoldersPreferencesModel::pathsToString(const io::paths_t& paths) const
{
    return QString::fromStdString(io::pathsToString(paths));
}

io::paths_t FoldersPreferencesModel::pathsFromString(const QString& pathsStr) const
{
    return io::pathsFromString(pathsStr.toStdString());
}
