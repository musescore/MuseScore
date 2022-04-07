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
    return m_categories.count();
}

QVariant FoldersPreferencesModel::data(const QModelIndex& index, int role) const
{
    const CategoryInfo& category = m_categories.at(index.row());
    switch (role) {
    case TitleRole: return category.title;
    case PathRole: return category.value;
    case DirRole: return category.dir;
    case IsMutliDirectoriesRole: return category.valueType == CategoryValueType::MultiDirectories;
    }

    return QVariant();
}

bool FoldersPreferencesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const CategoryInfo category = m_categories.at(index.row());

    switch (role) {
    case PathRole:
        if (category.value == value.toString()) {
            return false;
        }

        saveCategoryData(category.type, value.toString());
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
        { IsMutliDirectoriesRole, "isMutliDirectories" }
    };

    return roles;
}

void FoldersPreferencesModel::load()
{
    beginResetModel();

    m_categories = {
        { CategoryType::Scores, qtrc("appshell", "Scores"), projectConfiguration()->userProjectsPath().toQString(),
          projectConfiguration()->userProjectsPath().toQString() },
        { CategoryType::Styles, qtrc("appshell", "Styles"), notationConfiguration()->userStylesPath().toQString(),
          notationConfiguration()->userStylesPath().toQString() },
        { CategoryType::Templates, qtrc("appshell", "Templates"), projectConfiguration()->userTemplatesPath().toQString(),
          projectConfiguration()->userTemplatesPath().toQString() },
        { CategoryType::Plugins, qtrc("appshell", "Plugins"), pluginsConfiguration()->userPluginsPath().toQString(),
          pluginsConfiguration()->userPluginsPath().toQString() },
        { CategoryType::SoundFonts, qtrc("appshell", "SoundFonts"), pathsToString(audioConfiguration()->userSoundFontDirectories()),
          configuration()->userDataPath().toQString(), CategoryValueType::MultiDirectories },
        { CategoryType::Images, qtrc("appshell", "Images"), "", "" } // todo: need implement
    };

    endResetModel();

    setupConnections();
}

void FoldersPreferencesModel::setupConnections()
{
    projectConfiguration()->userProjectsPathChanged().onReceive(this, [this](const io::path& path) {
        setCategoryData(CategoryType::Scores, path.toQString());
    });

    notationConfiguration()->userStylesPathChanged().onReceive(this, [this](const io::path& path) {
        setCategoryData(CategoryType::Styles, path.toQString());
    });

    projectConfiguration()->userTemplatesPathChanged().onReceive(this, [this](const io::path& path) {
        setCategoryData(CategoryType::Templates, path.toQString());
    });

    pluginsConfiguration()->userPluginsPathChanged().onReceive(this, [this](const io::path& path) {
        setCategoryData(CategoryType::Plugins, path.toQString());
    });

    audioConfiguration()->soundFontDirectoriesChanged().onReceive(this, [this](const io::paths&) {
        io::paths userSoundFontsPaths = audioConfiguration()->userSoundFontDirectories();
        setCategoryData(CategoryType::SoundFonts, pathsToString(userSoundFontsPaths));
    });
}

void FoldersPreferencesModel::saveCategoryData(FoldersPreferencesModel::CategoryType categoryType, const QString& data)
{
    switch (categoryType) {
    case CategoryType::Scores: {
        io::path folderPath = data.toStdString();
        projectConfiguration()->setUserProjectsPath(folderPath);
        break;
    }
    case CategoryType::Styles: {
        io::path folderPath = data.toStdString();
        notationConfiguration()->setUserStylesPath(folderPath);
        break;
    }
    case CategoryType::Templates: {
        io::path folderPath = data.toStdString();
        projectConfiguration()->setUserTemplatesPath(folderPath);
        break;
    }
    case CategoryType::Plugins: {
        io::path folderPath = data.toStdString();
        pluginsConfiguration()->setUserPluginsPath(folderPath);
        break;
    }
    case CategoryType::SoundFonts: {
        audioConfiguration()->setUserSoundFontDirectories(pathsFromString(data));
        break;
    }
    case CategoryType::Images:
        NOT_IMPLEMENTED;
        break;
    case CategoryType::Undefined:
        break;
    }
}

void FoldersPreferencesModel::setCategoryData(FoldersPreferencesModel::CategoryType categoryType, const QString& data)
{
    QModelIndex index = categoryIndex(categoryType);
    if (!index.isValid()) {
        return;
    }

    m_categories[index.row()].value = data;
    emit dataChanged(index, index, { PathRole });
}

QModelIndex FoldersPreferencesModel::categoryIndex(FoldersPreferencesModel::CategoryType folderType)
{
    for (int i = 0; i < m_categories.count(); ++i) {
        CategoryInfo& categoryInfo = m_categories[i];
        if (categoryInfo.type == folderType) {
            return index(i, 0);
        }
    }

    return QModelIndex();
}

QString FoldersPreferencesModel::pathsToString(const mu::io::paths& paths) const
{
    return QString::fromStdString(io::pathsToString(paths));
}

mu::io::paths FoldersPreferencesModel::pathsFromString(const QString& pathsStr) const
{
    return io::pathsFromString(pathsStr.toStdString());
}
