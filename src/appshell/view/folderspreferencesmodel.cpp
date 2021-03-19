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
        { FolderType::Scores, qtrc("appshell", "Scores"), scoresPath() },
        { FolderType::Styles, qtrc("appshell", "Styles"), stylesPath() },
        { FolderType::Templates, qtrc("appshell", "Templates"), templatesPath() },
        { FolderType::Plugins, qtrc("appshell", "Plugins"), pluginsPath() },
        { FolderType::SoundFonts, qtrc("appshell", "SoundFonts"), "" }, // todo: need implement
        { FolderType::Images, qtrc("appshell", "Images"), "" }, // todo: need implement
        { FolderType::Extensions, qtrc("appshell", "Extensions"), extensionsPath() }
    };

    endResetModel();

    setupConnections();
}

void FoldersPreferencesModel::setupConnections()
{
    userScoresConfiguration()->scoresPath().ch.onReceive(this, [this](const io::path& path) {
        setPath(FolderType::Scores, path.toQString());
    });

    notationConfiguration()->stylesPath().ch.onReceive(this, [this](const io::path& path) {
        setPath(FolderType::Styles, path.toQString());
    });

    userScoresConfiguration()->templatesPath().ch.onReceive(this, [this](const io::path& path) {
        setPath(FolderType::Templates, path.toQString());
    });

    pluginsConfiguration()->pluginsPath().ch.onReceive(this, [this](const io::path& path) {
        setPath(FolderType::Plugins, path.toQString());
    });

    extensionsConfiguration()->extensionsPath().ch.onReceive(this, [this](const io::path& path) {
        setPath(FolderType::Extensions, path.toQString());
    });
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
        userScoresConfiguration()->setTemplatesPath(folderPath);
        break;
    case FolderType::Plugins:
        pluginsConfiguration()->setPluginsPath(folderPath);
        break;
    case FolderType::Extensions:
        extensionsConfiguration()->setExtensionsPath(folderPath);
        break;
    case FolderType::Undefined:
        break;
    case FolderType::SoundFonts:
    case FolderType::Images:
        NOT_IMPLEMENTED;
        break;
    }
}

QString FoldersPreferencesModel::scoresPath() const
{
    return userScoresConfiguration()->scoresPath().val.toQString();
}

QString FoldersPreferencesModel::stylesPath() const
{
    return notationConfiguration()->stylesPath().val.toQString();
}

QString FoldersPreferencesModel::templatesPath() const
{
    return userScoresConfiguration()->templatesPath().val.toQString();
}

QString FoldersPreferencesModel::pluginsPath() const
{
    return pluginsConfiguration()->pluginsPath().val.toQString();
}

QString FoldersPreferencesModel::extensionsPath() const
{
    return extensionsConfiguration()->extensionsPath().val.toQString();
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
