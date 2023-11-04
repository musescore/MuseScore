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
#include "sectionbreaksettingsmodel.h"

#include "translation.h"
#include "dataformatter.h"

using namespace mu::inspector;

SectionBreakSettingsModel::SectionBreakSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_SECTIONBREAK);
    setTitle(qtrc("inspector", "Section break"));
    setIcon(ui::IconCode::Code::SECTION_BREAK);
    createProperties();
}

void SectionBreakSettingsModel::createProperties()
{
    m_shouldStartWithLongInstrNames = buildPropertyItem(mu::engraving::Pid::START_WITH_LONG_NAMES);
    m_shouldResetBarNums = buildPropertyItem(mu::engraving::Pid::START_WITH_MEASURE_ONE);
    m_pauseDuration = buildPropertyItem(mu::engraving::Pid::PAUSE);
}

void SectionBreakSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::LAYOUT_BREAK);
}

void SectionBreakSettingsModel::loadProperties()
{
    loadPropertyItem(m_shouldStartWithLongInstrNames);
    loadPropertyItem(m_shouldResetBarNums);
    loadPropertyItem(m_pauseDuration, roundedDouble_internalToUi_converter(mu::engraving::Pid::PAUSE));
}

void SectionBreakSettingsModel::resetProperties()
{
    m_shouldStartWithLongInstrNames->resetToDefault();
    m_shouldResetBarNums->resetToDefault();
    m_pauseDuration->resetToDefault();
}

PropertyItem* SectionBreakSettingsModel::shouldStartWithLongInstrNames() const
{
    return m_shouldStartWithLongInstrNames;
}

PropertyItem* SectionBreakSettingsModel::shouldResetBarNums() const
{
    return m_shouldResetBarNums;
}

PropertyItem* SectionBreakSettingsModel::pauseDuration() const
{
    return m_pauseDuration;
}
