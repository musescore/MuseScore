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
    setTitle(muse::qtrc("inspector", "Stem"));

    createProperties();
}

void StemSettingsModel::createProperties()
{
    m_thickness = buildPropertyItem(Pid::LINE_WIDTH);
    m_length = buildPropertyItem(Pid::USER_LEN);

    m_stemDirection = buildPropertyItem(Pid::STEM_DIRECTION, [this](const Pid, const QVariant& newValue) {
        onStemDirectionChanged(static_cast<mu::engraving::DirectionV>(newValue.toInt()));
    });

    m_offset = buildPointFPropertyItem(Pid::OFFSET);
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
    m_offset->resetToDefault();
}

PropertyItem* StemSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* StemSettingsModel::length() const
{
    return m_length;
}

PropertyItem* StemSettingsModel::offset() const
{
    return m_offset;
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
    if (updateStyleValue(Sid::useStraightNoteFlags, use)) {
        emit useStraightNoteFlagsChanged();
    }
}

void StemSettingsModel::onStemDirectionChanged(DirectionV newDirection)
{
    beginCommand(muse::TranslatableString("undoableAction", "Change stem direction"));

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

    if (muse::contains(changedStyleIdSet, Sid::useStraightNoteFlags)) {
        emit useStraightNoteFlagsChanged();
    }
}

void StemSettingsModel::loadProperties(const PropertyIdSet& propertyIdSet)
{
    if (muse::contains(propertyIdSet, Pid::LINE_WIDTH)) {
        loadPropertyItem(m_thickness, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::USER_LEN)) {
        loadPropertyItem(m_length, formatDoubleFunc);
    }

    if (muse::contains(propertyIdSet, Pid::STEM_DIRECTION)) {
        loadPropertyItem(m_stemDirection);
    }

    if (muse::contains(propertyIdSet, Pid::OFFSET)) {
        loadPropertyItem(m_offset);
    }
}
