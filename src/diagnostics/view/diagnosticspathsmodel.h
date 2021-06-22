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
#ifndef MU_DIAGNOSTICS_DIAGNOSTICSPATHSMODEL_H
#define MU_DIAGNOSTICS_DIAGNOSTICSPATHSMODEL_H

#include <QAbstractListModel>
#include "modularity/ioc.h"
#include "idiagnosticspathsregister.h"

namespace mu::diagnostics {
class DiagnosticsPathsModel : public QAbstractListModel
{
    Q_OBJECT

    INJECT(diagnostics, IDiagnosticsPathsRegister, pathsRegister)

public:
    explicit DiagnosticsPathsModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();

private:
    enum Roles {
        rItemData = Qt::UserRole + 1
    };

    QVariantList m_items;
};
}

#endif // MU_DIAGNOSTICS_DIAGNOSTICSPATHSMODEL_H
