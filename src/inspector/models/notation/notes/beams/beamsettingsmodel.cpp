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
#include "beamsettingsmodel.h"

#include "translation.h"
#include "dataformatter.h"

using namespace mu::inspector;
using namespace mu::engraving;

BeamSettingsModel::BeamSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_BEAM);
    setTitle(qtrc("inspector", "Beam"));
    setBeamModesModel(new BeamModesModel(this, repository));

    createProperties();
}

void BeamSettingsModel::createProperties()
{
    m_featheringHeightLeft = buildPropertyItem(mu::engraving::Pid::GROW_LEFT);
    m_featheringHeightRight = buildPropertyItem(mu::engraving::Pid::GROW_RIGHT);

    m_isBeamHidden = buildPropertyItem(mu::engraving::Pid::VISIBLE, [this](const mu::engraving::Pid pid, const QVariant& isBeamHidden) {
        onPropertyValueChanged(pid, !isBeamHidden.toBool());
    });

    m_beamVectorX = buildPropertyItem(mu::engraving::Pid::BEAM_POS, [this](const mu::engraving::Pid, const QVariant& newValue) {
        updateBeamHeight(newValue.toDouble(), m_beamVectorY->value().toDouble());
    });

    m_beamVectorY = buildPropertyItem(mu::engraving::Pid::BEAM_POS, [this](const mu::engraving::Pid, const QVariant& newValue) {
        updateBeamHeight(m_beamVectorX->value().toDouble(), newValue.toDouble());
    });
}

void BeamSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::BEAM);
}

void BeamSettingsModel::loadProperties()
{
    loadPropertyItem(m_featheringHeightLeft, formatDoubleFunc);
    loadPropertyItem(m_featheringHeightRight, formatDoubleFunc);

    loadPropertyItem(m_isBeamHidden, [](const QVariant& isVisible) -> QVariant {
        return !isVisible.toBool();
    });

    loadPropertyItem(m_beamVectorX, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().x());
    });

    loadPropertyItem(m_beamVectorY, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().y());
    });

    m_cachedBeamVector.setX(m_beamVectorX->value().toDouble());
    m_cachedBeamVector.setY(m_beamVectorY->value().toDouble());

    updateFeatheringMode(m_featheringHeightLeft->value().toDouble(), m_featheringHeightRight->value().toDouble());
}

void BeamSettingsModel::resetProperties()
{
    m_featheringHeightLeft->resetToDefault();
    m_featheringHeightRight->resetToDefault();
    m_beamVectorX->resetToDefault();
    m_beamVectorY->resetToDefault();

    m_cachedBeamVector = PointF();

    setFeatheringMode(BeamTypes::FeatheringMode::FEATHERING_NONE);
    setIsBeamHeightLocked(false);
}

void BeamSettingsModel::updatePropertiesOnNotationChanged()
{
    loadPropertyItem(m_featheringHeightLeft, formatDoubleFunc);
    loadPropertyItem(m_featheringHeightRight, formatDoubleFunc);

    updateFeatheringMode(m_featheringHeightLeft->value().toDouble(), m_featheringHeightRight->value().toDouble());
}

void BeamSettingsModel::forceHorizontal()
{
    onPropertyValueChanged(mu::engraving::Pid::BEAM_NO_SLOPE, true);

    emit requestReloadPropertyItems();
}

void BeamSettingsModel::updateBeamHeight(const qreal& x, const qreal& y)
{
    if (m_isBeamHeightLocked) {
        synchronizeLockedBeamHeight(x, y);
    }

    onPropertyValueChanged(mu::engraving::Pid::USER_MODIFIED, true);
    onPropertyValueChanged(mu::engraving::Pid::BEAM_POS,
                           QVariant::fromValue(QPair<qreal, qreal>(m_beamVectorX->value().toDouble(), m_beamVectorY->value().toDouble())));

    m_cachedBeamVector.setX(m_beamVectorX->value().toDouble());
    m_cachedBeamVector.setY(m_beamVectorY->value().toDouble());
}

void BeamSettingsModel::synchronizeLockedBeamHeight(const qreal& currentX, const qreal& currentY)
{
    qreal deltaX = currentX - m_cachedBeamVector.x();
    qreal deltaY = currentY - m_cachedBeamVector.y();

    qreal maxDelta = qMax(qAbs(deltaX), qAbs(deltaY));

    if (qFuzzyCompare(qAbs(deltaX), maxDelta)) {
        m_beamVectorY->updateCurrentValue(m_cachedBeamVector.y() + deltaX);
    } else {
        m_beamVectorX->updateCurrentValue(m_cachedBeamVector.x() + deltaY);
    }
}

void BeamSettingsModel::updateFeatheringMode(const qreal& x, const qreal& y)
{
    if (x != y) {
        setFeatheringMode(x > y ? BeamTypes::FeatheringMode::FEATHERED_DECELERATE
                          : BeamTypes::FeatheringMode::FEATHERED_ACCELERATE);
    } else {
        setFeatheringMode(BeamTypes::FeatheringMode::FEATHERING_NONE);
    }
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

PropertyItem* BeamSettingsModel::beamVectorX() const
{
    return m_beamVectorX;
}

PropertyItem* BeamSettingsModel::beamVectorY() const
{
    return m_beamVectorY;
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

void BeamSettingsModel::setBeamModesModel(BeamModesModel* beamModesModel)
{
    m_beamModesModel = beamModesModel;

    connect(m_beamModesModel->isFeatheringAvailable(), &PropertyItem::propertyModified, this, [this](const mu::engraving::Pid,
                                                                                                     const QVariant& newValue) {
        if (!newValue.toBool()) {
            setFeatheringMode(BeamTypes::FeatheringMode::FEATHERING_NONE);
        }
    });

    connect(m_beamModesModel->mode(), &PropertyItem::propertyModified, this, &AbstractInspectorModel::requestReloadPropertyItems);

    emit beamModesModelChanged(m_beamModesModel);
}
