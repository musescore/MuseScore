/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

using namespace mu::propertiespanel;

StaffTypeSettingsModel::StaffTypeSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                               IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setModelType(PropertiesPanelModelType::TYPE_STAFF_TYPE_CHANGES);
    setTitle(muse::qtrc("propertiespanel", "Staff type changes"));
    setIcon(muse::ui::IconCode::Code::STAFF_TYPE_CHANGE);
    createProperties();
}

void StaffTypeSettingsModel::createProperties()
{
    m_isSmall = buildPropertyItem(mu::engraving::Pid::SMALL);
    m_verticalOffset = buildPropertyItem(mu::engraving::Pid::STAFF_YOFFSET);
    m_scale = buildPropertyItem(mu::engraving::Pid::MAG, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toDouble() / 100);
    });

    m_lineCount = buildPropertyItem(mu::engraving::Pid::STAFF_LINES); // int
    m_lineDistance = buildPropertyItem(mu::engraving::Pid::LINE_DISTANCE);
    m_stepOffset = buildPropertyItem(mu::engraving::Pid::STEP_OFFSET); // int
    m_isInvisible = buildPropertyItem(mu::engraving::Pid::STAFF_INVISIBLE);
    m_color = buildPropertyItem(mu::engraving::Pid::STAFF_COLOR);

    m_noteheadSchemeType = buildPropertyItem(mu::engraving::Pid::HEAD_SCHEME);
    m_isStemless = buildPropertyItem(mu::engraving::Pid::STAFF_STEMLESS);
    m_shouldShowBarlines = buildPropertyItem(mu::engraving::Pid::STAFF_SHOW_BARLINES);
    m_shouldShowLedgerLines = buildPropertyItem(mu::engraving::Pid::STAFF_SHOW_LEDGERLINES);
    m_shouldGenerateClefs = buildPropertyItem(mu::engraving::Pid::STAFF_GEN_CLEF);
    m_shouldGenerateTimeSignatures = buildPropertyItem(mu::engraving::Pid::STAFF_GEN_TIMESIG);
    m_shouldGenerateKeySignatures = buildPropertyItem(mu::engraving::Pid::STAFF_GEN_KEYSIG);

    m_staffLongName = buildPropertyItem(mu::engraving::Pid::STAFF_LONG_NAME);
    m_staffShortName = buildPropertyItem(mu::engraving::Pid::STAFF_SHORT_NAME);
}

void StaffTypeSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::STAFFTYPE_CHANGE);
}

void StaffTypeSettingsModel::loadProperties()
{
    loadPropertyItem(m_isSmall);
    loadPropertyItem(m_verticalOffset, formatDoubleFunc);
    loadPropertyItem(m_scale, [](const QVariant& elementPropertyValue) -> QVariant {
        return muse::DataFormatter::roundDouble(elementPropertyValue.toDouble()) * 100;
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

    loadPropertyItem(m_staffLongName);
    loadPropertyItem(m_staffShortName);
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

PropertyItem* StaffTypeSettingsModel::staffLongName() const
{
    return m_staffLongName;
}

PropertyItem* StaffTypeSettingsModel::staffShortName() const
{
    return m_staffShortName;
}
