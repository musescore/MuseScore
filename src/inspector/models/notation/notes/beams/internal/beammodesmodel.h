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
#ifndef MU_INSPECTOR_BEAMMODESMODEL_H
#define MU_INSPECTOR_BEAMMODESMODEL_H

#include "beammodelistmodel.h"
#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class BeamModesModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(QObject * modeListModel READ modeListModel NOTIFY modeListModelChanged)
    Q_PROPERTY(PropertyItem * mode READ mode CONSTANT)
    Q_PROPERTY(PropertyItem * isFeatheringAvailable READ isFeatheringAvailable CONSTANT)

public:
    explicit BeamModesModel(QObject* parent, IElementRepositoryService* repository);

    QObject* modeListModel() const;
    PropertyItem* mode() const;
    PropertyItem* isFeatheringAvailable() const;

public slots:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    void setModeListModel(BeamModeListModel* modeListModel);

signals:
    void modeListModelChanged(QObject* modeListModel);

private:
    BeamModeListModel* m_modeListModel = nullptr;
    PropertyItem* m_mode = nullptr;
    PropertyItem* m_isFeatheringAvailable = nullptr;
};
}

#endif // MU_INSPECTOR_BEAMMODESMODEL_H
