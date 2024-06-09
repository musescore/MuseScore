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
#ifndef BEAMSETTINGSMODEL_H
#define BEAMSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"
#include "models/notation/beams/beammodesmodel.h"
#include "types/beamtypes.h"

namespace mu::inspector {
class BeamSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(QObject * beamModesModel READ beamModesModel CONSTANT)

    Q_PROPERTY(PropertyItem * featheringHeightLeft READ featheringHeightLeft CONSTANT)
    Q_PROPERTY(PropertyItem * featheringHeightRight READ featheringHeightRight CONSTANT)
    Q_PROPERTY(
        mu::inspector::BeamTypes::FeatheringMode featheringMode READ featheringMode WRITE setFeatheringMode NOTIFY featheringModeChanged)
    Q_PROPERTY(bool isFeatheringHeightChangingAllowed READ isFeatheringHeightChangingAllowed NOTIFY featheringModeChanged)

    Q_PROPERTY(PropertyItem * beamHeightLeft READ beamHeightLeft CONSTANT)
    Q_PROPERTY(PropertyItem * beamHeightRight READ beamHeightRight CONSTANT)
    Q_PROPERTY(bool isBeamHeightLocked READ isBeamHeightLocked WRITE setIsBeamHeightLocked NOTIFY isBeamHeightLockedChanged)

    Q_PROPERTY(PropertyItem * isBeamHidden READ isBeamHidden CONSTANT)

    Q_PROPERTY(PropertyItem * forceHorizontal READ forceHorizontal CONSTANT)

    Q_PROPERTY(PropertyItem * customPositioned READ customPositioned CONSTANT)

    Q_PROPERTY(PropertyItem * stemDirection READ stemDirection CONSTANT)
    Q_PROPERTY(PropertyItem * crossStaffMove READ crossStaffMove CONSTANT)
    Q_PROPERTY(
        bool isCrossStaffMoveAvailable READ isCrossStaffMoveAvailable WRITE setIsCrossStaffMoveAvailable NOTIFY isCrossStaffMoveAvailableChanged)

public:
    explicit BeamSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* forceHorizontal();

    PropertyItem* customPositioned();

    QObject* beamModesModel() const;

    PropertyItem* featheringHeightLeft() const;
    PropertyItem* featheringHeightRight() const;
    BeamTypes::FeatheringMode featheringMode() const;
    bool isFeatheringHeightChangingAllowed() const;

    PropertyItem* isBeamHidden() const;

    PropertyItem* beamHeightLeft() const;
    PropertyItem* beamHeightRight() const;
    bool isBeamHeightLocked() const;

    PropertyItem* stemDirection() const;
    PropertyItem* crossStaffMove() const;
    bool isCrossStaffMoveAvailable() const;

public slots:
    void setIsBeamHeightLocked(bool isBeamHeightLocked);
    void setFeatheringMode(BeamTypes::FeatheringMode featheringMode);
    void setIsCrossStaffMoveAvailable(bool isCrossStaffMoveAvailable);

signals:
    void isBeamHeightLockedChanged(bool isBeamHeightLocked);
    void featheringModeChanged(BeamTypes::FeatheringMode featheringMode);
    void isCrossStaffMoveAvailableChanged(bool isCrossStaffMoveAvailable);

private:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

    void loadBeamHeightProperties();
    void setBeamHeightLeft(const qreal left);
    void setBeamHeightRight(const qreal height2);
    void setBeamHeight(const qreal left, const qreal right);

    void updateFeatheringMode(const qreal left, const qreal right);

    void updateIsCrossStaffMoveAvailable();

    void onCurrentNotationChanged() override;

    BeamModesModel* m_beamModesModel = nullptr;

    PropertyItem* m_featheringHeightLeft = nullptr;
    PropertyItem* m_featheringHeightRight = nullptr;
    BeamTypes::FeatheringMode m_featheringMode = BeamTypes::FeatheringMode::FEATHERING_NONE;

    PropertyItem* m_beamHeightLeft = nullptr;
    PropertyItem* m_beamHeightRight = nullptr;
    muse::PairF m_cachedBeamHeights; //!Note used in delta calculation
    bool m_isBeamHeightLocked = false;

    PropertyItem* m_isBeamHidden = nullptr;
    PropertyItem* m_forceHorizontal = nullptr;
    PropertyItem* m_customPositioned = nullptr;

    PropertyItem* m_stemDirection = nullptr;
    PropertyItem* m_crossStaffMove = nullptr;
    bool m_isCrossStaffMoveAvailable = false;
};
}

#endif // BEAMSETTINGSMODEL_H
