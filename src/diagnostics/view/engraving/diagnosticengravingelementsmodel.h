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
#ifndef MU_DIAGNOSTICS_DIAGNOSTICENGRAVINGELEMENTSMODEL_H
#define MU_DIAGNOSTICS_DIAGNOSTICENGRAVINGELEMENTSMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "modularity/ioc.h"
#include "idiagnosticengravingregister.h"

namespace mu::diagnostics {
class DiagnosticEngravingElementsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString info READ info NOTIFY infoChanged)

    INJECT(diagnostics, IDiagnosticEngravingRegister, engravingRegister)

public:
    DiagnosticEngravingElementsModel(QObject* parent = 0);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString info() const;

    Q_INVOKABLE void init();
    Q_INVOKABLE void reload();

signals:
    void infoChanged();

private:

    enum Roles {
        rItemData = Qt::UserRole + 1,
    };

    struct Item
    {
        Ms::ScoreElement* el = nullptr;
        QVariant toQVariant() const;
    };

    void updateInfo();

    QList<Item> m_items;
    QString m_info;
};
}

#endif // MU_DIAGNOSTICS_DIAGNOSTICENGRAVINGELEMENTSMODEL_H
