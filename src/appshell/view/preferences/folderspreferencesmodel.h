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
#ifndef MU_APPSHELL_FOLDERSPREFERENCESMODEL_H
#define MU_APPSHELL_FOLDERSPREFERENCESMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "userscores/iuserscoresconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "plugins/ipluginsconfiguration.h"
#include "extensions/iextensionsconfiguration.h"

namespace mu::appshell {
class FoldersPreferencesModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, userscores::IUserScoresConfiguration, userScoresConfiguration)
    INJECT(appshell, notation::INotationConfiguration, notationConfiguration)
    INJECT(appshell, plugins::IPluginsConfiguration, pluginsConfiguration)
    INJECT(appshell, extensions::IExtensionsConfiguration, extensionsConfiguration)

public:
    explicit FoldersPreferencesModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int,QByteArray> roleNames() const override;

    Q_INVOKABLE void load();

private:
    void setupConnections();

    enum Roles {
        TitleRole = Qt::UserRole + 1,
        PathRole
    };

    enum class FolderType {
        Undefined,
        Scores,
        Styles,
        Templates,
        Plugins,
        SoundFonts,
        Images,
        Extensions
    };

    struct FolderInfo {
        FolderType type = FolderType::Undefined;
        QString title;
        QString path;
    };

    void savePath(FolderType folderType, const QString& path);

    QString scoresPath() const;
    QString stylesPath() const;
    QString templatesPath() const;
    QString pluginsPath() const;
    QString extensionsPath() const;

    void setPath(FolderType folderType, const QString& path);
    QModelIndex folderIndex(FolderType folderType);

    QList<FolderInfo> m_folders;
};
}

#endif // MU_APPSHELL_FOLDERSPREFERENCESMODEL_H
