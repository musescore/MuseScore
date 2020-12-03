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
#ifndef MU_SETTINGLISTMODEL_H
#define MU_SETTINGLISTMODEL_H

#include <QAbstractListModel>
#include <QMap>

#include "settings.h"

namespace mu {
class SettingListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SettingListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int,QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void changeVal(int index, QVariant newVal);

private:

    enum Roles {
        SectionRole = Qt::UserRole + 1,
        KeyRole,
        TypeRole,
        ValRole
    };

    QString typeToString(Val::Type t) const;

    QList<framework::Settings::Item> m_items;
};
}

#endif // MU_SETTINGLISTMODEL_H
