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
#include "clefsettingsmodel.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/clef.h"

#include "translation.h"

using namespace mu::inspector;

ClefSettingsModel::ClefSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_CLEF);
    setTitle(muse::qtrc("inspector", "Clef"));
    setIcon(muse::ui::IconCode::Code::CLEF_BASS);
    createProperties();
}

void ClefSettingsModel::createProperties()
{
    m_shouldShowCourtesy = buildPropertyItem(mu::engraving::Pid::SHOW_COURTESY);
    m_clefToBarlinePosition = buildPropertyItem(mu::engraving::Pid::CLEF_TO_BARLINE_POS);
    updatePropertiesAvailable();
}

void ClefSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::CLEF);
}

void ClefSettingsModel::loadProperties()
{
    loadPropertyItem(m_shouldShowCourtesy);
    loadPropertyItem(m_clefToBarlinePosition);
    updatePropertiesAvailable();
}

void ClefSettingsModel::resetProperties()
{
    m_shouldShowCourtesy->resetToDefault();
    m_clefToBarlinePosition->resetToDefault();
}

PropertyItem* ClefSettingsModel::shouldShowCourtesy() const
{
    return m_shouldShowCourtesy;
}

PropertyItem* ClefSettingsModel::clefToBarlinePosition() const
{
    return m_clefToBarlinePosition;
}

bool ClefSettingsModel::isClefToBarPosAvailable() const
{
    return m_isClefToBarPosAvailable;
}

bool ClefSettingsModel::isCourtesyClefAvailable() const
{
    return m_isCourtesyClefAvailable;
}

void ClefSettingsModel::setIsClefToBarPosAvailable(bool available)
{
    if (available == m_isClefToBarPosAvailable) {
        return;
    }

    m_isClefToBarPosAvailable = available;
    emit isClefToBarPosAvailableChanged(available);
}

void ClefSettingsModel::setIsCourtesyClefAvailable(bool available)
{
    if (available == m_isCourtesyClefAvailable) {
        return;
    }

    m_isCourtesyClefAvailable = available;
    emit isCourtesyClefAvailableChanged(available);
}

void ClefSettingsModel::updatePropertiesAvailable()
{
    bool hasHeaderClef = false;
    bool hasRepeatCourtesy = false;
    bool enableCourtesy = true;
    for (mu::engraving::EngravingItem* item : m_elementList) {
        mu::engraving::Segment* clefSeg = static_cast<mu::engraving::Clef*>(item)->segment();
        if (clefSeg->isHeaderClefType()) {
            hasHeaderClef = true;
        } else if (clefSeg->isClefRepeatAnnounceType()) {
            hasRepeatCourtesy = true;
            enableCourtesy = false;
        }

        const engraving::Measure* measure = item->findMeasure();
        const engraving::Measure* prevMeasure = measure ? measure->prevMeasure() : nullptr;
        const engraving::LayoutBreak* sectionBreak = prevMeasure ? prevMeasure->sectionBreakElement() : nullptr;
        if (sectionBreak && !sectionBreak->showCourtesy()) {
            enableCourtesy = false;
        }
    }

    setIsClefToBarPosAvailable(!(hasHeaderClef || hasRepeatCourtesy));
    setIsCourtesyClefAvailable(enableCourtesy);
}
