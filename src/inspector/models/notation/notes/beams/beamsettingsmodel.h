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
#ifndef BEAMSETTINGSMODEL_H
#define BEAMSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"
#include "internal/beammodesmodel.h"
#include "types/beamtypes.h"

namespace mu::inspector {
class BeamSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(QObject * beamModesModel READ beamModesModel NOTIFY beamModesModelChanged)

    Q_PROPERTY(PropertyItem * featheringHeightLeft READ featheringHeightLeft CONSTANT)
    Q_PROPERTY(PropertyItem * featheringHeightRight READ featheringHeightRight CONSTANT)
    Q_PROPERTY(
        mu::inspector::BeamTypes::FeatheringMode featheringMode READ featheringMode WRITE setFeatheringMode NOTIFY featheringModeChanged)
    Q_PROPERTY(bool isFeatheringHeightChangingAllowed READ isFeatheringHeightChangingAllowed NOTIFY featheringModeChanged)

    Q_PROPERTY(PropertyItem * beamVectorX READ beamVectorX CONSTANT)
    Q_PROPERTY(PropertyItem * beamVectorY READ beamVectorY CONSTANT)
    Q_PROPERTY(bool isBeamHeightLocked READ isBeamHeightLocked WRITE setIsBeamHeightLocked NOTIFY isBeamHeightLockedChanged)

    Q_PROPERTY(PropertyItem * isBeamHidden READ isBeamHidden CONSTANT)

public:
    explicit BeamSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void forceHorizontal();

    QObject* beamModesModel() const;

    PropertyItem* featheringHeightLeft() const;
    PropertyItem* featheringHeightRight() const;
    BeamTypes::FeatheringMode featheringMode() const;
    bool isFeatheringHeightChangingAllowed() const;

    PropertyItem* isBeamHidden() const;

    PropertyItem* beamVectorX() const;
    PropertyItem* beamVectorY() const;
    bool isBeamHeightLocked() const;

public slots:
    void setIsBeamHeightLocked(bool isBeamHeightLocked);
    void setFeatheringMode(BeamTypes::FeatheringMode featheringMode);
    void setBeamModesModel(BeamModesModel* beamModesModel);

signals:
    void isBeamHeightLockedChanged(bool isBeamHeightLocked);
    void featheringModeChanged(BeamTypes::FeatheringMode featheringMode);
    void beamModesModelChanged(QObject* beamModesModel);

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    void updateBeamHeight(const qreal& x, const qreal& y);
    void synchronizeLockedBeamHeight(const qreal& currentX, const qreal& currentY);
    void updateFeatheringMode(const qreal& x, const qreal& y);

    BeamModesModel* m_beamModesModel = nullptr;

    PropertyItem* m_featheringHeightLeft = nullptr;
    PropertyItem* m_featheringHeightRight = nullptr;
    BeamTypes::FeatheringMode m_featheringMode = BeamTypes::FeatheringMode::FEATHERING_NONE;

    PropertyItem* m_beamVectorX = nullptr;
    PropertyItem* m_beamVectorY = nullptr;
    PointF m_cachedBeamVector; //!Note used in delta calculation
    bool m_isBeamHeightLocked = false;

    PropertyItem* m_isBeamHidden = nullptr;
};
}

#endif // BEAMSETTINGSMODEL_H
