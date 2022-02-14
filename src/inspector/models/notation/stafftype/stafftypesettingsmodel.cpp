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
#include "stafftypesettingsmodel.h"

#include "types/commontypes.h"

#include "translation.h"

using namespace mu::inspector;

StaffTypeSettingsModel::StaffTypeSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_STAFF_TYPE_CHANGES);
    setTitle(qtrc("inspector", "Staff type changes"));
    setIcon(ui::IconCode::Code::STAFF_TYPE_CHANGE);
    createProperties();
}

void StaffTypeSettingsModel::createProperties()
{
    m_isSmall = buildPropertyItem(Ms::Pid::SMALL);
    m_verticalOffset = buildPropertyItem(Ms::Pid::STAFF_YOFFSET);
    m_scale = buildPropertyItem(Ms::Pid::MAG, [this](const Ms::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toDouble() / 100);
    });

    m_lineCount = buildPropertyItem(Ms::Pid::STAFF_LINES); // int
    m_lineDistance = buildPropertyItem(Ms::Pid::LINE_DISTANCE);
    m_stepOffset = buildPropertyItem(Ms::Pid::STEP_OFFSET); // int
    m_isInvisible = buildPropertyItem(Ms::Pid::STAFF_INVISIBLE);
    m_color = buildPropertyItem(Ms::Pid::STAFF_COLOR);

    m_noteheadSchemeType = buildPropertyItem(Ms::Pid::HEAD_SCHEME);
    m_isStemless = buildPropertyItem(Ms::Pid::STAFF_STEMLESS);
    m_shouldShowBarlines = buildPropertyItem(Ms::Pid::STAFF_SHOW_BARLINES);
    m_shouldShowLedgerLines = buildPropertyItem(Ms::Pid::STAFF_SHOW_LEDGERLINES);
    m_shouldGenerateClefs = buildPropertyItem(Ms::Pid::STAFF_GEN_CLEF);
    m_shouldGenerateTimeSignatures = buildPropertyItem(Ms::Pid::STAFF_GEN_TIMESIG);
    m_shouldGenerateKeySignatures = buildPropertyItem(Ms::Pid::STAFF_GEN_KEYSIG);
}

void StaffTypeSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::STAFFTYPE_CHANGE);
}

void StaffTypeSettingsModel::loadProperties()
{
    loadPropertyItem(m_isSmall);
    loadPropertyItem(m_verticalOffset, formatDoubleFunc);
    loadPropertyItem(m_scale, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::roundDouble(elementPropertyValue.toDouble()) * 100;
    });

    loadPropertyItem(m_lineCount);
    loadPropertyItem(m_lineDistance, formatDoubleFunc);
    loadPropertyItem(m_stepOffset);
    loadPropertyItem(m_isInvisible);
    loadPropertyItem(m_color);

    loadPropertyItem(m_noteheadSchemeType);
    loadPropertyItem(m_isStemless);
    loadPropertyItem(m_shouldShowBarlines);
    loadPropertyItem(m_shouldShowLedgerLines);
    loadPropertyItem(m_shouldGenerateClefs);
    loadPropertyItem(m_shouldGenerateTimeSignatures);
    loadPropertyItem(m_shouldGenerateKeySignatures);
}

void StaffTypeSettingsModel::resetProperties()
{
    m_isSmall->resetToDefault();
    m_verticalOffset->resetToDefault();
    m_scale->resetToDefault();

    m_lineCount->resetToDefault();
    m_lineDistance->resetToDefault();
    m_stepOffset->resetToDefault();
    m_isInvisible->resetToDefault();
    m_color->resetToDefault();

    m_noteheadSchemeType->resetToDefault();
    m_isStemless->resetToDefault();
    m_shouldShowBarlines->resetToDefault();
    m_shouldShowLedgerLines->resetToDefault();
    m_shouldGenerateClefs->resetToDefault();
    m_shouldGenerateTimeSignatures->resetToDefault();
    m_shouldGenerateKeySignatures->resetToDefault();
}

PropertyItem* StaffTypeSettingsModel::isSmall() const
{
    return m_isSmall;
}

PropertyItem* StaffTypeSettingsModel::verticalOffset() const
{
    return m_verticalOffset;
}

PropertyItem* StaffTypeSettingsModel::scale() const
{
    return m_scale;
}

PropertyItem* StaffTypeSettingsModel::lineCount() const
{
    return m_lineCount;
}

PropertyItem* StaffTypeSettingsModel::lineDistance() const
{
    return m_lineDistance;
}

PropertyItem* StaffTypeSettingsModel::stepOffset() const
{
    return m_stepOffset;
}

PropertyItem* StaffTypeSettingsModel::isInvisible() const
{
    return m_isInvisible;
}

PropertyItem* StaffTypeSettingsModel::color() const
{
    return m_color;
}

PropertyItem* StaffTypeSettingsModel::noteheadSchemeType() const
{
    return m_noteheadSchemeType;
}

PropertyItem* StaffTypeSettingsModel::isStemless() const
{
    return m_isStemless;
}

PropertyItem* StaffTypeSettingsModel::shouldShowBarlines() const
{
    return m_shouldShowBarlines;
}

PropertyItem* StaffTypeSettingsModel::shouldShowLedgerLines() const
{
    return m_shouldShowLedgerLines;
}

PropertyItem* StaffTypeSettingsModel::shouldGenerateClefs() const
{
    return m_shouldGenerateClefs;
}

PropertyItem* StaffTypeSettingsModel::shouldGenerateTimeSignatures() const
{
    return m_shouldGenerateTimeSignatures;
}

PropertyItem* StaffTypeSettingsModel::shouldGenerateKeySignatures() const
{
    return m_shouldGenerateKeySignatures;
}
