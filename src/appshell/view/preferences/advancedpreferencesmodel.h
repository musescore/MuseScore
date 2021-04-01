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
#ifndef MU_APPSHELL_ADVANCEDPREFERENCESMODEL_H
#define MU_APPSHELL_ADVANCEDPREFERENCESMODEL_H

#include <QAbstractListModel>

#include "settings.h"

namespace mu::appshell {
class AdvancedPreferencesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit AdvancedPreferencesModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int,QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void resetToDefault();

private:

    enum Roles {
        KeyRole = Qt::UserRole + 1,
        TypeRole,
        ValRole
    };

    void changeVal(int index, QVariant newVal);
    QString typeToString(Val::Type type) const;

    QList<framework::Settings::Item> m_items;
};
}

#endif // MU_APPSHELL_ADVANCEDPREFERENCESMODEL_H
