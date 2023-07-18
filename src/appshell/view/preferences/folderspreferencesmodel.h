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
#include "vst/ivstconfiguration.h"
#include "iappshellconfiguration.h"

namespace mu::appshell {
class FoldersPreferencesModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(project::IProjectConfiguration, projectConfiguration)
    INJECT(notation::INotationConfiguration, notationConfiguration)
    INJECT(plugins::IPluginsConfiguration, pluginsConfiguration)
    INJECT(audio::IAudioConfiguration, audioConfiguration)
    INJECT(vst::IVstConfiguration, vstConfiguration)
    INJECT(IAppShellConfiguration, configuration)

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
        DirRole,
        IsMultiDirectoriesRole
    };

    enum class FolderType {
        Undefined,
        Scores,
        Styles,
        Templates,
        Plugins,
        SoundFonts,
        VST3
    };

    enum class FolderValueType {
        Directory,
        MultiDirectories
    };

    struct FolderInfo {
        FolderType type = FolderType::Undefined;
        QString title;
        QString value;
        QString dir;
        FolderValueType valueType = FolderValueType::Directory;
    };

    void saveFolderPaths(FolderType folderType, const QString& paths);

    void setFolderPaths(FolderType folderType, const QString& paths);
    QModelIndex folderIndex(FolderType folderType);

    QString pathsToString(const io::paths_t& paths) const;
    io::paths_t pathsFromString(const QString& pathsStr) const;

    QList<FolderInfo> m_folders;
};
}

#endif // MU_APPSHELL_FOLDERSPREFERENCESMODEL_H
