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
#ifndef MU_INSPECTOR_NOTEHEADTYPESMODEL_H
#define MU_INSPECTOR_NOTEHEADTYPESMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "note.h"
#include "types/noteheadtypes.h"

namespace mu::inspector {
class NoteheadTypesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int selectedHeadTypeIndex READ selectedHeadTypeIndex WRITE setSelectedHeadTypeIndex NOTIFY selectedHeadTypeIndexChanged)

public:
    enum RoleNames {
        HeadGroupRole = Qt::UserRole + 1,
        HintRole
    };

    explicit NoteheadTypesModel(QObject* parent = nullptr);

    void load();

    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;

    void init(const Ms::NoteHead::Group noteHeadGroup);

    int selectedHeadTypeIndex() const;

public slots:
    void setSelectedHeadTypeIndex(int selectedHeadTypeIndex);

signals:
    void noteHeadGroupSelected(int headGroup);
    void selectedHeadTypeIndexChanged(int selectedHeadTypeIndex);

private:
    struct HeadTypeData {
        Ms::NoteHead::Group group = Ms::NoteHead::Group::HEAD_INVALID;
        QString hint;
    };

    int indexOfHeadGroup(const Ms::NoteHead::Group group) const;

    QList<HeadTypeData> m_noteheadTypeDataList;
    QHash<int, QByteArray> m_roleNames;
    int m_selectedHeadTypeIndex = 0;
};
}

#endif // MU_INSPECTOR_NOTEHEADTYPESMODEL_H
