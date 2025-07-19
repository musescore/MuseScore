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
#include "diagnosticspathsmodel.h"

#include "log.h"

using namespace muse::diagnostics;

DiagnosticsPathsModel::DiagnosticsPathsModel(QObject* parent)
    : QAbstractListModel(parent), Injectable(muse::iocCtxForQmlObject(this))
{
}

void DiagnosticsPathsModel::load()
{
    const std::vector<IDiagnosticsPathsRegister::Item>& items = pathsRegister()->items();

    beginResetModel();

    QString dump;
    for (const IDiagnosticsPathsRegister::Item& item : items) {
        QVariantMap it;
        it["name"] = QString::fromStdString(item.name);
        it["path"] = item.path.toQString();

        dump += QString::fromStdString(item.name) + " " + item.path.toQString() + "\n";

        m_items << it;
    }

    endResetModel();

    LOGI() << dump;
}

void DiagnosticsPathsModel::openPath(const QString& path)
{
    interactive()->openUrl(QUrl::fromUserInput(path));
}

QVariant DiagnosticsPathsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case rItemData: return m_items.at(index.row());
    }

    return QVariant();
}

int DiagnosticsPathsModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int, QByteArray> DiagnosticsPathsModel::roleNames() const
{
    return { { rItemData, "itemData" } };
}
