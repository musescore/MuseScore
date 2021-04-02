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
#include "folderspreferencesmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::appshell;

FoldersPreferencesModel::FoldersPreferencesModel(QObject* parent)
    : QAbstractListModel(parent)
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
    case PathRole: return folder.path;
    case IsMultiplePathsRole: return folder.isMultiplePaths;
    }

    return QVariant();
}

bool FoldersPreferencesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const FolderInfo folder = m_folders.at(index.row());

    switch (role) {
    case PathRole: {
        QString path = value.toString();
        if (folder.path == path) {
            return false;
        }

        savePath(folder.type, path);

        m_folders[index.row()].path = path;
        emit dataChanged(index, index, { PathRole });
        return true;
    }
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
        { IsMultiplePathsRole, "isMultiplePaths" }
    };

    return roles;
}

void FoldersPreferencesModel::load()
{
    beginResetModel();

    m_folders = {
        { FolderType::Scores, qtrc("appshell", "Scores"), scoresPath() },
        { FolderType::Styles, qtrc("appshell", "Styles"), stylesPath() },
        { FolderType::Templates, qtrc("appshell", "Templates"), templatesPath() },
        { FolderType::Plugins, qtrc("appshell", "Plugins"), pluginsPath() },
        { FolderType::SoundFonts, qtrc("appshell", "SoundFonts"), soundFontsPaths(), true },
        { FolderType::Images, qtrc("appshell", "Images"), "" }, // todo: need implement
        { FolderType::Extensions, qtrc("appshell", "Extensions"), extensionsPath() }
    };

    endResetModel();
}

void FoldersPreferencesModel::savePath(FoldersPreferencesModel::FolderType folderType, const QString& path)
{
    io::path folderPath = path.toStdString();

    switch (folderType) {
    case FolderType::Scores:
        userScoresConfiguration()->setScoresPath(folderPath);
        break;
    case FolderType::Styles:
        notationConfiguration()->setStylesPath(folderPath);
        break;
    case FolderType::Templates:
        userScoresConfiguration()->setUserTemplatesPath(folderPath);
        break;
    case FolderType::Plugins:
        pluginsConfiguration()->setPluginsPath(folderPath);
        break;
    case FolderType::Extensions:
        extensionsConfiguration()->setUserExtensionsPath(folderPath);
        break;
    case FolderType::SoundFonts: {
        io::paths paths = io::pathsFromString(path.toStdString());
        audioConfiguration()->setUserSoundFontPaths(paths);
        break;
    }
    case FolderType::Images:
        NOT_IMPLEMENTED;
        break;
    case FolderType::Undefined:
        break;
    }
}

QString FoldersPreferencesModel::scoresPath() const
{
    return userScoresConfiguration()->scoresPath().toQString();
}

QString FoldersPreferencesModel::stylesPath() const
{
    return notationConfiguration()->stylesPath().val.toQString();
}

QString FoldersPreferencesModel::templatesPath() const
{
    return userScoresConfiguration()->userTemplatesPath().toQString();
}

QString FoldersPreferencesModel::pluginsPath() const
{
    return pluginsConfiguration()->pluginsPath().val.toQString();
}

QString FoldersPreferencesModel::soundFontsPaths() const
{
    io::paths paths = audioConfiguration()->userSoundFontPaths();
    return QString::fromStdString(io::pathsToString(paths));
}

QString FoldersPreferencesModel::extensionsPath() const
{
    return extensionsConfiguration()->userExtensionsPath().toQString();
}

void FoldersPreferencesModel::setPath(FoldersPreferencesModel::FolderType folderType, const QString& path)
{
    QModelIndex index = folderIndex(folderType);
    if (!index.isValid()) {
        return;
    }

    m_folders[index.row()].path = path;
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
