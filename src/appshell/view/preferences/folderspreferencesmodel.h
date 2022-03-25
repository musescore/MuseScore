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
#ifndef MU_APPSHELL_FOLDERSPREFERENCESMODEL_H
#define MU_APPSHELL_FOLDERSPREFERENCESMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "project/iprojectconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "plugins/ipluginsconfiguration.h"
#include "audio/iaudioconfiguration.h"

namespace mu::appshell {
class FoldersPreferencesModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, project::IProjectConfiguration, projectConfiguration)
    INJECT(appshell, notation::INotationConfiguration, notationConfiguration)
    INJECT(appshell, plugins::IPluginsConfiguration, pluginsConfiguration)
    INJECT(appshell, audio::IAudioConfiguration, audioConfiguration)

public:
    explicit FoldersPreferencesModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();

private:
    void setupConnections();

    enum Roles {
        TitleRole = Qt::UserRole + 1,
        PathRole,
        IsMutliDirectoriesRole
    };

    enum class CategoryType {
        Undefined,
        Scores,
        Styles,
        Templates,
        Plugins,
        SoundFonts,
        Images
    };

    enum class CategoryValueType {
        Directory,
        MultiDirectories
    };

    struct CategoryInfo {
        CategoryType type = CategoryType::Undefined;
        QString title;
        QString value;
        CategoryValueType valueType = CategoryValueType::Directory;
    };

    void saveCategoryData(CategoryType categoryType, const QString& path);

    void setCategoryData(CategoryType categoryType, const QString& data);
    QModelIndex categoryIndex(CategoryType folderType);

    QString pathsToString(const io::paths& paths) const;
    io::paths pathsFromString(const QString& pathsStr) const;

    QList<CategoryInfo> m_categories;
};
}

#endif // MU_APPSHELL_FOLDERSPREFERENCESMODEL_H
