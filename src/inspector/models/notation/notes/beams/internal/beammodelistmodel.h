/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_INSPECTOR_BEAMTYPESMODEL_H
#define MU_INSPECTOR_BEAMTYPESMODEL_H

#include <QAbstractListModel>
#include "types/beamtypes.h"

namespace mu::inspector {
class BeamModeListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int selectedTypeIndex READ selectedTypeIndex WRITE setSelectedTypeIndex NOTIFY selectedTypeIndexChanged)

public:
    enum RoleNames {
        BeamModeRole = Qt::UserRole + 1,
        HintRole
    };

    explicit BeamModeListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setSelectedBeamMode(const BeamTypes::Mode beamMode);

    int selectedTypeIndex() const;

public slots:
    void setSelectedTypeIndex(int selectedTypeIndex);

signals:
    void selectedTypeIndexChanged(int selectedTypeIndex);
    void beamModeSelected(BeamTypes::Mode beamMode);

private:
    struct BeamTypesData {
        BeamTypes::Mode mode;
        QString hint;
    };

    int indexOfBeamMode(const BeamTypes::Mode beamMode) const;

    QHash<int, QByteArray> m_roleNames;

    QVector<BeamTypesData> m_beamTypesDataList;
    int m_selectedTypeIndex = -1;
};
}

#endif // MU_INSPECTOR_BEAMTYPESMODEL_H
