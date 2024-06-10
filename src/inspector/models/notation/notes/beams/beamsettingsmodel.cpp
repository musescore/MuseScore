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
#include "beamsettingsmodel.h"
#include "engraving/dom/beam.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

BeamSettingsModel::BeamSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_BEAM);
    setTitle(muse::qtrc("inspector", "Beam"));

    m_beamModesModel = new BeamModesModel(this, repository);
    m_beamModesModel->init();

    connect(m_beamModesModel->isFeatheringAvailable(), &PropertyItem::propertyModified,
            this, [this](const mu::engraving::Pid, const QVariant& newValue) {
        if (!newValue.toBool()) {
            setFeatheringMode(BeamTypes::FeatheringMode::FEATHERING_NONE);
        }
    });

    connect(m_beamModesModel->mode(), &PropertyItem::propertyModified, this, &AbstractInspectorModel::requestReloadPropertyItems);

    createProperties();
}

void BeamSettingsModel::createProperties()
{
    m_featheringHeightLeft = buildPropertyItem(mu::engraving::Pid::GROW_LEFT);
    m_featheringHeightRight = buildPropertyItem(mu::engraving::Pid::GROW_RIGHT);

    m_isBeamHidden = buildPropertyItem(mu::engraving::Pid::VISIBLE, [this](const mu::engraving::Pid pid, const QVariant& isBeamHidden) {
        onPropertyValueChanged(pid, !isBeamHidden.toBool());
    });

    m_beamHeightLeft = buildPropertyItem(mu::engraving::Pid::BEAM_POS, [this](const mu::engraving::Pid, const QVariant& newValue) {
        setBeamHeightLeft(newValue.toDouble());
    });

    m_beamHeightRight = buildPropertyItem(mu::engraving::Pid::BEAM_POS, [this](const mu::engraving::Pid, const QVariant& newValue) {
        setBeamHeightRight(newValue.toDouble());
    });

    m_forceHorizontal = buildPropertyItem(mu::engraving::Pid::BEAM_NO_SLOPE,
                                          [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        loadBeamHeightProperties();
    });

    m_customPositioned = buildPropertyItem(mu::engraving::Pid::USER_MODIFIED,
                                           [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        loadBeamHeightProperties();
    });

    m_stemDirection = buildPropertyItem(mu::engraving::Pid::STEM_DIRECTION);

    m_crossStaffMove = buildPropertyItem(mu::engraving::Pid::BEAM_CROSS_STAFF_MOVE,
                                         [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        loadPropertyItem(m_crossStaffMove);
    });
}

void BeamSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::BEAM);
}

void BeamSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::GROW_LEFT,
        Pid::GROW_RIGHT,
        Pid::VISIBLE,
        Pid::BEAM_POS,
        Pid::BEAM_NO_SLOPE,
        Pid::USER_MODIFIED,
        Pid::BEAM_CROSS_STAFF_MOVE,
        Pid::STEM_DIRECTION,
    };

    loadProperties(propertyIdSet);
}

void BeamSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet&)
{
    loadProperties(changedPropertyIdSet);
}

void BeamSettingsModel::loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::VISIBLE)) {
        loadPropertyItem(m_isBeamHidden, [](const QVariant& isVisible) -> QVariant {
            return !isVisible.toBool();
        });
    }

    if (muse::contains(propertyIdSet, Pid::GROW_LEFT)) {
        loadPropertyItem(m_featheringHeightLeft, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::GROW_RIGHT)) {
        loadPropertyItem(m_featheringHeightRight, formatDoubleFunc);
    }

    loadBeamHeightProperties();

    if (m_featheringHeightLeft->value().isValid() && m_featheringHeightRight->value().isValid()) {
        updateFeatheringMode(m_featheringHeightLeft->value().toDouble(), m_featheringHeightRight->value().toDouble());
    }

    loadPropertyItem(m_stemDirection);
    loadPropertyItem(m_crossStaffMove);
    updateIsCrossStaffMoveAvailable();
}

void BeamSettingsModel::loadBeamHeightProperties()
{
    loadPropertyItem(m_beamHeightLeft, [](const QVariant& elementPropertyValue) -> QVariant {
        return round((elementPropertyValue.value<QPair<double, double> >().first) * 100) / 100;
    });

    loadPropertyItem(m_beamHeightRight, [](const QVariant& elementPropertyValue) -> QVariant {
        return round((elementPropertyValue.value<QPair<double, double> >().second) * 100) / 100;
    });

    loadPropertyItem(m_forceHorizontal);
    loadPropertyItem(m_customPositioned);

    m_cachedBeamHeights.first = m_beamHeightLeft->value().toDouble();
    m_cachedBeamHeights.second = m_beamHeightRight->value().toDouble();
}

void BeamSettingsModel::resetProperties()
{
    m_featheringHeightLeft->resetToDefault();
    m_featheringHeightRight->resetToDefault();
    m_beamHeightLeft->resetToDefault();
    m_beamHeightRight->resetToDefault();
    m_forceHorizontal->resetToDefault();
    m_customPositioned->resetToDefault();
    m_stemDirection->resetToDefault();
    m_crossStaffMove->resetToDefault();

    m_cachedBeamHeights = PairF();

    setFeatheringMode(BeamTypes::FeatheringMode::FEATHERING_NONE);
    setIsBeamHeightLocked(false);
}

PropertyItem* BeamSettingsModel::forceHorizontal()
{
    return m_forceHorizontal;
}

PropertyItem* BeamSettingsModel::stemDirection() const
{
    return m_stemDirection;
}

PropertyItem* BeamSettingsModel::crossStaffMove() const
{
    return m_crossStaffMove;
}

bool BeamSettingsModel::isCrossStaffMoveAvailable() const
{
    return m_isCrossStaffMoveAvailable;
}

PropertyItem* BeamSettingsModel::customPositioned()
{
    return m_customPositioned;
}

void BeamSettingsModel::setBeamHeightLeft(const qreal left)
{
    qreal right = m_beamHeightRight->value().toDouble();
    if (m_forceHorizontal->value().toBool()) {
        right = left;
    } else if (m_isBeamHeightLocked) {
        qreal delta = left - m_cachedBeamHeights.first;
        right += delta;
    }

    setBeamHeight(left, right);
}

void BeamSettingsModel::setBeamHeightRight(const qreal right)
{
    qreal left = m_beamHeightLeft->value().toDouble();
    if (m_forceHorizontal->value().toBool()) {
        left = right;
    } else if (m_isBeamHeightLocked) {
        qreal delta = right - m_cachedBeamHeights.second;
        left += delta;
    }

    setBeamHeight(left, right);
}

void BeamSettingsModel::setBeamHeight(const qreal left, const qreal right)
{
    m_customPositioned->setValue(true);

    onPropertyValueChanged(engraving::Pid::BEAM_POS, QVariant::fromValue(QPair<qreal, qreal>(left, right)));

    loadBeamHeightProperties();
}

void BeamSettingsModel::updateFeatheringMode(const qreal left, const qreal right)
{
    BeamTypes::FeatheringMode newFeathering = BeamTypes::FeatheringMode::FEATHERING_NONE;
    if (left != right) {
        newFeathering = left > right ? BeamTypes::FeatheringMode::FEATHERED_DECELERATE
                        : BeamTypes::FeatheringMode::FEATHERED_ACCELERATE;
    } else {
        newFeathering = BeamTypes::FeatheringMode::FEATHERING_NONE;
    }
    if (newFeathering != m_featheringMode) {
        m_featheringMode = newFeathering;
        emit featheringModeChanged(m_featheringMode);
    }
}

void BeamSettingsModel::updateIsCrossStaffMoveAvailable()
{
    bool available = true;
    for (EngravingItem* item : m_elementList) {
        if (!item->isBeam()) {
            continue;
        }
        if (!toBeam(item)->cross()) {
            available = false;
            break;
        }
    }

    setIsCrossStaffMoveAvailable(available);
}

bool BeamSettingsModel::isBeamHeightLocked() const
{
    return m_isBeamHeightLocked;
}

QObject* BeamSettingsModel::beamModesModel() const
{
    return m_beamModesModel;
}

PropertyItem* BeamSettingsModel::featheringHeightLeft() const
{
    return m_featheringHeightLeft;
}

PropertyItem* BeamSettingsModel::featheringHeightRight() const
{
    return m_featheringHeightRight;
}

BeamTypes::FeatheringMode BeamSettingsModel::featheringMode() const
{
    return m_featheringMode;
}

bool BeamSettingsModel::isFeatheringHeightChangingAllowed() const
{
    return m_featheringMode != BeamTypes::FeatheringMode::FEATHERING_NONE;
}

PropertyItem* BeamSettingsModel::isBeamHidden() const
{
    return m_isBeamHidden;
}

PropertyItem* BeamSettingsModel::beamHeightLeft() const
{
    return m_beamHeightLeft;
}

PropertyItem* BeamSettingsModel::beamHeightRight() const
{
    return m_beamHeightRight;
}

void BeamSettingsModel::setIsBeamHeightLocked(bool isBeamHeightLocked)
{
    if (m_isBeamHeightLocked == isBeamHeightLocked) {
        return;
    }

    m_isBeamHeightLocked = isBeamHeightLocked;
    emit isBeamHeightLockedChanged(m_isBeamHeightLocked);
}

void BeamSettingsModel::setFeatheringMode(BeamTypes::FeatheringMode featheringMode)
{
    if (m_featheringMode == featheringMode) {
        return;
    }

    m_featheringMode = featheringMode;

    switch (featheringMode) {
    case BeamTypes::FeatheringMode::FEATHERING_NONE:
        m_featheringHeightLeft->setValue(1.0);
        m_featheringHeightRight->setValue(1.0);
        break;
    case BeamTypes::FeatheringMode::FEATHERED_DECELERATE:
        m_featheringHeightLeft->setValue(1.0);
        m_featheringHeightRight->setValue(0.0);
        break;
    case BeamTypes::FeatheringMode::FEATHERED_ACCELERATE:
        m_featheringHeightLeft->setValue(0.0);
        m_featheringHeightRight->setValue(1.0);
        break;
    }

    emit featheringModeChanged(featheringMode);
}

void BeamSettingsModel::setIsCrossStaffMoveAvailable(bool isCrossStaffMoveAvailable)
{
    if (m_isCrossStaffMoveAvailable == isCrossStaffMoveAvailable) {
        return;
    }

    m_isCrossStaffMoveAvailable = isCrossStaffMoveAvailable;
    emit isCrossStaffMoveAvailableChanged(isCrossStaffMoveAvailable);
}

void BeamSettingsModel::onCurrentNotationChanged()
{
    AbstractInspectorModel::onCurrentNotationChanged();

    m_beamModesModel->onCurrentNotationChanged();
}
