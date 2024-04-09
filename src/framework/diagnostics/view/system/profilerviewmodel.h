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
#ifndef MUSE_DIAGNOSTICS_PROFILERVIEWMODEL_H
#define MUSE_DIAGNOSTICS_PROFILERVIEWMODEL_H

#include <QAbstractListModel>

namespace muse::diagnostics {
class ProfilerViewModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ProfilerViewModel(QObject* parent = 0);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void find(const QString& str);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void print();

private:

    enum Roles {
        rData = Qt::UserRole + 1,
        rGroup
    };

    struct Item {
        QString group;
        QString data;
    };

    QHash<int, QByteArray> m_roles;
    QList<Item> m_list;
    QList<Item> m_allList;
    QString m_searchText;
};
}

#endif // MUSE_DIAGNOSTICS_PROFILERVIEWMODEL_H
