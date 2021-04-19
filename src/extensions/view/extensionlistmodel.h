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
#ifndef MU_EXTENSIONS_EXTENSIONLISTMODEL_H
#define MU_EXTENSIONS_EXTENSIONLISTMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "iextensionsservice.h"
#include "async/asyncable.h"

namespace mu::extensions {
class ExtensionListModel : public QAbstractListModel, async::Asyncable
{
    Q_OBJECT

    INJECT(extensions, IExtensionsService, extensionsService)

public:
    ExtensionListModel(QObject* parent = nullptr);

    enum Roles {
        rCode = Qt::UserRole + 1,
        rName,
        rDescription,
        rVersion,
        rFileSize,
        rStatus,
        rInstalledGroup
    };

    QVariant data(const QModelIndex& index, int role) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void install(QString code);
    Q_INVOKABLE void uninstall(QString code);
    Q_INVOKABLE void update(QString code);
    Q_INVOKABLE void openFullDescription(QString code);

    Q_INVOKABLE QVariantMap extension(QString code);

signals:
    void progress(const QString& extensionCode, const QString& status, bool indeterminate, qint64 current, qint64 total);
    void finish(const QVariantMap& item);

private:
    int itemIndexByCode(const QString& code) const;

    QHash<int, QByteArray> m_roles;
    QList<Extension> m_list;
};
}

#endif // MU_EXTENSIONS_EXTENSIONLISTMODEL_H
