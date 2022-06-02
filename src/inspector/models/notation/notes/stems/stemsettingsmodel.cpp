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
#include "stemsettingsmodel.h"

#include "types/stemtypes.h"
#include "dataformatter.h"

#include "translation.h"
#include "log.h"

using namespace mu::inspector;

StemSettingsModel::StemSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_STEM);
    setTitle(qtrc("inspector", "Stem"));

    createProperties();
}

void StemSettingsModel::createProperties()
{
    m_isStemHidden = buildPropertyItem(mu::engraving::Pid::VISIBLE, [this](const mu::engraving::Pid pid, const QVariant& isStemHidden) {
        onPropertyValueChanged(pid, !isStemHidden.toBool());
    });

    m_thickness = buildPropertyItem(mu::engraving::Pid::LINE_WIDTH);
    m_length = buildPropertyItem(mu::engraving::Pid::USER_LEN);

    m_stemDirection = buildPropertyItem(mu::engraving::Pid::STEM_DIRECTION, [this](const mu::engraving::Pid, const QVariant& newValue) {
        onStemDirectionChanged(static_cast<mu::engraving::DirectionV>(newValue.toInt()));
    });

    m_horizontalOffset = buildPropertyItem(mu::engraving::Pid::OFFSET, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QPointF(newValue.toDouble(), m_verticalOffset->value().toDouble()));
    });

    m_verticalOffset = buildPropertyItem(mu::engraving::Pid::OFFSET, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, QPointF(m_horizontalOffset->value().toDouble(), newValue.toDouble()));
    });

    context()->currentNotation()->style()->styleChanged().onNotify(this, [this]() {
        emit useStraightNoteFlagsChanged();
    });
}

void StemSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::STEM);
}

void StemSettingsModel::loadProperties()
{
    loadPropertyItem(m_isStemHidden, [](const QVariant& isVisible) -> QVariant {
        return !isVisible.toBool();
    });

    loadPropertyItem(m_thickness, formatDoubleFunc);
    loadPropertyItem(m_length, formatDoubleFunc);

    loadPropertyItem(m_stemDirection);

    loadPropertyItem(m_horizontalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().x());
    });

    loadPropertyItem(m_verticalOffset, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.value<QPointF>().y());
    });
}

void StemSettingsModel::resetProperties()
{
    m_isStemHidden->resetToDefault();
    m_thickness->resetToDefault();
    m_length->resetToDefault();
    m_stemDirection->resetToDefault();
    m_horizontalOffset->resetToDefault();
    m_verticalOffset->resetToDefault();
}

PropertyItem* StemSettingsModel::isStemHidden() const
{
    return m_isStemHidden;
}

PropertyItem* StemSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* StemSettingsModel::length() const
{
    return m_length;
}

PropertyItem* StemSettingsModel::horizontalOffset() const
{
    return m_horizontalOffset;
}

PropertyItem* StemSettingsModel::verticalOffset() const
{
    return m_verticalOffset;
}

PropertyItem* StemSettingsModel::stemDirection() const
{
    return m_stemDirection;
}

bool StemSettingsModel::useStraightNoteFlags() const
{
    return context()->currentNotation()->style()->styleValue(mu::engraving::Sid::useStraightNoteFlags).toBool();
}

void StemSettingsModel::setUseStraightNoteFlags(bool use)
{
    if (use == useStraightNoteFlags()) {
        return;
    }

    context()->currentNotation()->undoStack()->prepareChanges();
    context()->currentNotation()->style()->setStyleValue(mu::engraving::Sid::useStraightNoteFlags, use);
    context()->currentNotation()->undoStack()->commitChanges();
}

void StemSettingsModel::onStemDirectionChanged(mu::engraving::DirectionV newDirection)
{
    beginCommand();

    for (mu::engraving::EngravingItem* element : m_elementList) {
        mu::engraving::Stem* stem = mu::engraving::toStem(element);
        IF_ASSERT_FAILED(stem) {
            continue;
        }

        mu::engraving::EngravingItem* root = stem;
        if (mu::engraving::Beam* beam = stem->chord()->beam()) {
            root = beam;
        }

        root->undoChangeProperty(mu::engraving::Pid::STEM_DIRECTION, newDirection);
    }

    endCommand();
    updateNotation();
}

void StemSettingsModel::updatePropertiesOnNotationChanged()
{
    loadPropertyItem(m_length, formatDoubleFunc);
}
