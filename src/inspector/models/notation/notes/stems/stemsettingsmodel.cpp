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

#include "engraving/dom/beam.h"

#include "translation.h"
#include "log.h"

using namespace mu::inspector;
using namespace mu::engraving;

StemSettingsModel::StemSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_STEM);
    setTitle(qtrc("inspector", "Stem"));

    createProperties();
}

void StemSettingsModel::createProperties()
{
    m_thickness = buildPropertyItem(Pid::LINE_WIDTH);
    m_length = buildPropertyItem(Pid::USER_LEN);

    m_stemDirection = buildPropertyItem(Pid::STEM_DIRECTION, [this](const Pid, const QVariant& newValue) {
        onStemDirectionChanged(static_cast<mu::engraving::DirectionV>(newValue.toInt()));
    });

    m_horizontalOffset = buildPropertyItem(mu::engraving::Pid::OFFSET, [](const QVariant& value, const engraving::EngravingItem* element) {
        double newX = value.toDouble();
        newX *= element->sizeIsSpatiumDependent() ? element->spatium() : mu::engraving::DPMM;
        return PointF(newX, element->getProperty(mu::engraving::Pid::OFFSET).value<PointF>().y());
    }, [this](const QVariant& value) {
        // TODO: What if m_verticalOffset->value() is invalid?
        return PointF(value.toDouble(), m_verticalOffset->value().toDouble());
    });

    m_verticalOffset = buildPropertyItem(mu::engraving::Pid::OFFSET, [](const QVariant& value, const engraving::EngravingItem* element) {
        double newY = value.toDouble();
        newY *= element->sizeIsSpatiumDependent() ? element->spatium() : mu::engraving::DPMM;
        return PointF(element->getProperty(mu::engraving::Pid::OFFSET).value<PointF>().x(), newY);
    }, [this](const QVariant& value) {
        // TODO: What if m_horizontalOffset->value() is invalid?
        return PointF(m_horizontalOffset->value().toDouble(), value.toDouble());
    });
}

void StemSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(ElementType::STEM);
}

void StemSettingsModel::loadProperties()
{
    static const PropertyIdSet propertyIdSet {
        Pid::VISIBLE,
        Pid::LINE_WIDTH,
        Pid::USER_LEN,
        Pid::STEM_DIRECTION,
        Pid::OFFSET,
    };

    loadProperties(propertyIdSet);
    emit useStraightNoteFlagsChanged();
}

void StemSettingsModel::resetProperties()
{
    m_thickness->resetToDefault();
    m_length->resetToDefault();
    m_stemDirection->resetToDefault();
    m_horizontalOffset->resetToDefault();
    m_verticalOffset->resetToDefault();
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
    return styleValue(Sid::useStraightNoteFlags).toBool();
}

void StemSettingsModel::setUseStraightNoteFlags(bool use)
{
    if (setStyleValue(Sid::useStraightNoteFlags, use)) {
        emit useStraightNoteFlagsChanged();
    }
}

void StemSettingsModel::onStemDirectionChanged(DirectionV newDirection)
{
    beginCommand();

    for (EngravingItem* element : m_elementList) {
        Stem* stem = toStem(element);
        IF_ASSERT_FAILED(stem) {
            continue;
        }

        EngravingItem* root = stem;
        if (Beam* beam = stem->chord()->beam()) {
            root = beam;
        }

        root->undoChangeProperty(Pid::STEM_DIRECTION, newDirection);
    }

    endCommand();
    updateNotation();
}

void StemSettingsModel::onNotationChanged(const PropertyIdSet& changedPropertyIdSet, const StyleIdSet& changedStyleIdSet)
{
    loadProperties(changedPropertyIdSet);

    if (mu::contains(changedStyleIdSet, Sid::useStraightNoteFlags)) {
        emit useStraightNoteFlagsChanged();
    }
}

void StemSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (mu::contains(propertyIdSet, Pid::LINE_WIDTH)) {
        loadPropertyItem(m_thickness, roundedDoubleElementInternalToUiConverter(Pid::LINE_WIDTH));
    }

    if (mu::contains(propertyIdSet, Pid::USER_LEN)) {
        loadPropertyItem(m_length, roundedDoubleElementInternalToUiConverter(Pid::USER_LEN));
    }

    if (mu::contains(propertyIdSet, Pid::STEM_DIRECTION)) {
        loadPropertyItem(m_stemDirection);
    }

    if (mu::contains(propertyIdSet, Pid::OFFSET)) {
        loadPropertyItem(m_horizontalOffset, [](const engraving::PropertyValue& propertyValue, const engraving::EngravingItem* element) {
            double x = propertyValue.value<PointF>().x();
            x /= element->sizeIsSpatiumDependent() ? element->spatium() : mu::engraving::DPMM;
            return x;
        });

        loadPropertyItem(m_verticalOffset, [](const engraving::PropertyValue& propertyValue, const engraving::EngravingItem* element) {
            double y = propertyValue.value<PointF>().y();
            y /= element->sizeIsSpatiumDependent() ? element->spatium() : mu::engraving::DPMM;
            return y;
        });
    }
}
