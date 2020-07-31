//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_EXTENSIONS_EXTENSIONLISTMODEL_H
#define MU_EXTENSIONS_EXTENSIONLISTMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "iextensionscontroller.h"
#include "async/asyncable.h"

namespace mu {
namespace extensions {
class ExtensionListModel : public QAbstractListModel, async::Asyncable
{
    Q_OBJECT

    INJECT(extensions, IExtensionsController, extensionsController)

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

signals:
    void progress(const QString& status, bool indeterminate, qint64 current, qint64 total);
    void finish();

private:
    int itemIndexByCode(const QString& code) const;

    QHash<int, QByteArray> m_roles;
    QList<Extension> m_list;
};
}
}

#endif // MU_EXTENSIONS_EXTENSIONLISTMODEL_H
