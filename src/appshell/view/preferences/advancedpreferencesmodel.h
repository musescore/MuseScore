/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_APPSHELL_ADVANCEDPREFERENCESMODEL_H
#define MU_APPSHELL_ADVANCEDPREFERENCESMODEL_H

#include <QAbstractListModel>

#include "async/asyncable.h"
#include "settings.h"

namespace mu::appshell {
class AdvancedPreferencesModel : public QAbstractListModel, public muse::async::Asyncable
{
    Q_OBJECT

public:
    explicit AdvancedPreferencesModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void resetToDefault();

private:

    enum Roles {
        KeyRole = Qt::UserRole + 1,
        DescriptionRole,
        TypeRole,
        ValueRole,
        MinValueRole,
        MaxValueRole
    };

    QModelIndex findIndex(const muse::Settings::Key& key);
    void changeVal(int index, const muse::Val& newVal);
    void changeModelVal(muse::Settings::Item& item, const muse::Val& newVal);
    QString typeToString(muse::Val::Type type) const;

    QList<muse::Settings::Item> m_items;
};
}

#endif // MU_APPSHELL_ADVANCEDPREFERENCESMODEL_H
