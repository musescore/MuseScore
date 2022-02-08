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
    }

    return QVariant();
}

bool FoldersPreferencesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const FolderInfo folder = m_folders.at(index.row());

    switch (role) {
    case PathRole:
        if (folder.path == value.toString()) {
            return false;
        }

        savePath(folder.type, value.toString());
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
        { PathRole, "path" }
    };

    return roles;
}

void FoldersPreferencesModel::load()
{
    beginResetModel();

    m_folders = {
        { FolderType::Scores, qtrc("appshell", "Scores"), projectConfiguration()->userProjectsPath().toQString() },
        { FolderType::Styles, qtrc("appshell", "Styles"), notationConfiguration()->userStylesPath().toQString() },
        { FolderType::Templates, qtrc("appshell", "Templates"), projectConfiguration()->userTemplatesPath().toQString() },
        { FolderType::Plugins, qtrc("appshell", "Plugins"), pluginsConfiguration()->userPluginsPath().toQString() },
        { FolderType::SoundFonts, qtrc("appshell", "SoundFonts"), "" }, // todo: need implement
        { FolderType::Images, qtrc("appshell", "Images"), "" } // todo: need implement
    };

    endResetModel();

    setupConnections();
}

void FoldersPreferencesModel::setupConnections()
{
    projectConfiguration()->userProjectsPathChanged().onReceive(this, [this](const io::path& path) {
        setPath(FolderType::Scores, path.toQString());
    });

    notationConfiguration()->userStylesPathChanged().onReceive(this, [this](const io::path& path) {
        setPath(FolderType::Styles, path.toQString());
    });

    projectConfiguration()->userTemplatesPathChanged().onReceive(this, [this](const io::path& path) {
        setPath(FolderType::Templates, path.toQString());
    });

    pluginsConfiguration()->userPluginsPathChanged().onReceive(this, [this](const io::path& path) {
        setPath(FolderType::Plugins, path.toQString());
    });
}

void FoldersPreferencesModel::savePath(FoldersPreferencesModel::FolderType folderType, const QString& path)
{
    io::path folderPath = path.toStdString();

    switch (folderType) {
    case FolderType::Scores:
        projectConfiguration()->setUserProjectsPath(folderPath);
        break;
    case FolderType::Styles:
        notationConfiguration()->setUserStylesPath(folderPath);
        break;
    case FolderType::Templates:
        projectConfiguration()->setUserTemplatesPath(folderPath);
        break;
    case FolderType::Plugins:
        pluginsConfiguration()->setUserPluginsPath(folderPath);
        break;
    case FolderType::Undefined:
        break;
    case FolderType::SoundFonts:
    case FolderType::Images:
        NOT_IMPLEMENTED;
        break;
    }
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
