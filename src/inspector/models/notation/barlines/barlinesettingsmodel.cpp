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
#include "barlinesettingsmodel.h"

#include "translation.h"
#include "types/barlinetypes.h"
#include "barline.h"

using namespace mu::inspector;

BarlineSettingsModel::BarlineSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_BARLINE);
    setTitle(qtrc("inspector", "Barline"));
    setIcon(ui::IconCode::Code::SECTION_BREAK);
    createProperties();
}

void BarlineSettingsModel::createProperties()
{
    m_type = buildPropertyItem(Ms::Pid::BARLINE_TYPE);
    m_isSpanToNextStaff = buildPropertyItem(Ms::Pid::BARLINE_SPAN);
    m_spanFrom = buildPropertyItem(Ms::Pid::BARLINE_SPAN_FROM);
    m_spanTo = buildPropertyItem(Ms::Pid::BARLINE_SPAN_TO);
    m_hasToShowTips = buildPropertyItem(Ms::Pid::BARLINE_SHOW_TIPS);
}

void BarlineSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::BAR_LINE);
}

void BarlineSettingsModel::loadProperties()
{
    loadPropertyItem(m_type, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toInt();
    });

    loadPropertyItem(m_isSpanToNextStaff, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toBool();
    });

    loadPropertyItem(m_spanFrom, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toInt();
    });

    loadPropertyItem(m_spanTo, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toInt();
    });

    loadPropertyItem(m_hasToShowTips);
}

void BarlineSettingsModel::resetProperties()
{
    m_type->resetToDefault();
    m_isSpanToNextStaff->resetToDefault();
    m_spanFrom->resetToDefault();
    m_spanTo->resetToDefault();
    m_hasToShowTips->resetToDefault();
}

void BarlineSettingsModel::applySpanPreset(const int presetType)
{
    BarlineTypes::SpanPreset type = static_cast<BarlineTypes::SpanPreset>(presetType);
    switch (type) {
    case BarlineTypes::SpanPreset::PRESET_DEFAULT:
        m_isSpanToNextStaff->resetToDefault();
        m_spanFrom->resetToDefault();
        m_spanTo->resetToDefault();
        break;
    case BarlineTypes::SpanPreset::PRESET_TICK_1:
        m_isSpanToNextStaff->setValue(false);
        m_spanFrom->setValue(Ms::BARLINE_SPAN_TICK1_FROM);
        m_spanTo->setValue(Ms::BARLINE_SPAN_TICK1_TO);
        break;
    case BarlineTypes::SpanPreset::PRESET_TICK_2:
        m_isSpanToNextStaff->setValue(false);
        m_spanFrom->setValue(Ms::BARLINE_SPAN_TICK2_FROM);
        m_spanTo->setValue(Ms::BARLINE_SPAN_TICK2_TO);
        break;
    case BarlineTypes::SpanPreset::PRESET_SHORT_1:
        m_isSpanToNextStaff->setValue(false);
        m_spanFrom->setValue(Ms::BARLINE_SPAN_SHORT1_FROM);
        m_spanTo->setValue(Ms::BARLINE_SPAN_SHORT1_TO);
        break;
    case BarlineTypes::SpanPreset::PRESET_SHORT_2:
        m_isSpanToNextStaff->setValue(false);
        m_spanFrom->setValue(Ms::BARLINE_SPAN_SHORT2_FROM);
        m_spanTo->setValue(Ms::BARLINE_SPAN_SHORT2_TO);
        break;
    default:
        break;
    }
}

PropertyItem* BarlineSettingsModel::type() const
{
    return m_type;
}

PropertyItem* BarlineSettingsModel::isSpanToNextStaff() const
{
    return m_isSpanToNextStaff;
}

PropertyItem* BarlineSettingsModel::spanFrom() const
{
    return m_spanFrom;
}

PropertyItem* BarlineSettingsModel::spanTo() const
{
    return m_spanTo;
}

PropertyItem* BarlineSettingsModel::hasToShowTips() const
{
    return m_hasToShowTips;
}
