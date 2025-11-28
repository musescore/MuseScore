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
#include "sectionbreaksettingsmodel.h"

#include "translation.h"
#include "dataformatter.h"

using namespace mu::inspector;

SectionBreakSettingsModel::SectionBreakSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_SECTIONBREAK);
    setTitle(muse::qtrc("inspector", "Section break"));
    setIcon(muse::ui::IconCode::Code::SECTION_BREAK);
    createProperties();
}

void SectionBreakSettingsModel::createProperties()
{
    m_shouldStartWithLongInstrNames = buildPropertyItem(mu::engraving::Pid::START_WITH_LONG_NAMES);
    m_shouldResetBarNums = buildPropertyItem(mu::engraving::Pid::START_WITH_MEASURE_ONE);
    m_pauseDuration = buildPropertyItem(mu::engraving::Pid::PAUSE);
    m_firstSystemIndent = buildPropertyItem(mu::engraving::Pid::FIRST_SYSTEM_INDENTATION);
    m_showCourtesySignatures = buildPropertyItem(mu::engraving::Pid::SHOW_COURTESY);
}

void SectionBreakSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::LAYOUT_BREAK);
}

void SectionBreakSettingsModel::loadProperties()
{
    loadPropertyItem(m_shouldStartWithLongInstrNames);
    loadPropertyItem(m_shouldResetBarNums);
    loadPropertyItem(m_pauseDuration, formatDoubleFunc);
    loadPropertyItem(m_firstSystemIndent);
    loadPropertyItem(m_showCourtesySignatures);
}

void SectionBreakSettingsModel::resetProperties()
{
    m_shouldStartWithLongInstrNames->resetToDefault();
    m_shouldResetBarNums->resetToDefault();
    m_pauseDuration->resetToDefault();
    m_firstSystemIndent->resetToDefault();
    m_showCourtesySignatures->resetToDefault();
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

PropertyItem* SectionBreakSettingsModel::firstSystemIndent() const
{
    return m_firstSystemIndent;
}

PropertyItem* SectionBreakSettingsModel::showCourtesySignatures() const
{
    return m_showCourtesySignatures;
}
