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
#ifndef MU_EXTENSIONS_EXTENSIONSMODEL_H
#define MU_EXTENSIONS_EXTENSIONSMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "extensions/iextensionsconfiguration.h"
#include "extensions/iextensionsprovider.h"

namespace mu::extensions {
class ExtensionsListModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    Inject<IInteractive> interactive;
    Inject<IExtensionsProvider> provider;
    Inject<IExtensionsConfiguration> configuration;

public:
    explicit ExtensionsListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void setEnable(const QString& uri, bool enable);
    Q_INVOKABLE void editShortcut(QString codeKey);
    Q_INVOKABLE void reloadPlugins();

    Q_INVOKABLE QVariantList categories() const;

signals:
    void finished();

private:
    enum Roles {
        rCode = Qt::UserRole + 1,
        rName,
        rDescription,
        rThumbnailUrl,
        rEnabled,
        rCategory,
        rVersion,
        rShortcuts
    };

    void updatePlugin(const Manifest& plugin);
    int itemIndexByCodeKey(const QString& uri) const;

    QHash<int, QByteArray> m_roles;
    ManifestList m_plugins;
};
}

#endif // MU_EXTENSIONS_EXTENSIONSMODEL_H
