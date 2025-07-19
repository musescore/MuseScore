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
#include "inspectormodelwithvoiceandpositionoptions.h"

using namespace mu::inspector;
using namespace mu::engraving;

InspectorModelWithVoiceAndPositionOptions::InspectorModelWithVoiceAndPositionOptions(QObject* parent, IElementRepositoryService* repository,
                                                                                     ElementType elementType)
    : AbstractInspectorModel(parent, repository, elementType)
{
    createProperties();
}

void InspectorModelWithVoiceAndPositionOptions::createProperties()
{
    m_voiceBasedPosition = buildPropertyItem(Pid::DIRECTION, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        updateIsStaveCenteringAvailable();
    });
    m_voiceAssignment = buildPropertyItem(Pid::VOICE_ASSIGNMENT);
    m_voice = buildPropertyItem(Pid::VOICE);
    m_centerBetweenStaves = buildPropertyItem(Pid::CENTER_BETWEEN_STAVES);
    updateIsMultiStaffInstrument();
    updateIsStaveCenteringAvailable();
}

void InspectorModelWithVoiceAndPositionOptions::loadProperties()
{
    loadPropertyItem(m_voiceBasedPosition);
    loadPropertyItem(m_voiceAssignment);
    loadPropertyItem(m_voice);
    loadPropertyItem(m_centerBetweenStaves);
    updateIsMultiStaffInstrument();
    updateIsStaveCenteringAvailable();
}

void InspectorModelWithVoiceAndPositionOptions::resetProperties()
{
    m_voiceBasedPosition->resetToDefault();
    m_voiceAssignment->resetToDefault();
    m_voice->setValue(0);
    m_centerBetweenStaves->resetToDefault();
}

void InspectorModelWithVoiceAndPositionOptions::onNotationChanged(const PropertyIdSet&, const StyleIdSet&)
{
    loadProperties();
}

void InspectorModelWithVoiceAndPositionOptions::updateIsMultiStaffInstrument()
{
    bool isMultiStaffInstrument = true;
    for (EngravingItem* item : m_elementList) {
        if (item->part()->nstaves() <= 1) {
            isMultiStaffInstrument = false;
            break;
        }
    }

    setIsMultiStaffInstrument(isMultiStaffInstrument);
}

void InspectorModelWithVoiceAndPositionOptions::updateIsStaveCenteringAvailable()
{
    if (!m_isMultiStaffInstrument) {
        setIsStaveCenteringAvailable(false);
        return;
    }

    bool isStaveCenteringAvailable = true;
    for (EngravingItem* item : m_elementList) {
        staff_idx_t thisStaffIdx = item->staffIdx();
        DirectionV itemDirection = item->getProperty(Pid::DIRECTION).value<DirectionV>();
        const std::vector<Staff*>& partStaves = item->part()->staves();
        staff_idx_t firstStaffOfPart = partStaves.front()->idx();
        staff_idx_t lastStaffOfPart = partStaves.back()->idx();
        if ((itemDirection == DirectionV::UP && thisStaffIdx == firstStaffOfPart)
            || (itemDirection == DirectionV::DOWN && thisStaffIdx == lastStaffOfPart)) {
            isStaveCenteringAvailable = false;
            break;
        }
    }

    setIsStaveCenteringAvailable(isStaveCenteringAvailable);
}

PropertyItem* InspectorModelWithVoiceAndPositionOptions::voiceBasedPosition() const
{
    return m_voiceBasedPosition;
}

PropertyItem* InspectorModelWithVoiceAndPositionOptions::voiceAssignment() const
{
    return m_voiceAssignment;
}

PropertyItem* InspectorModelWithVoiceAndPositionOptions::voice() const
{
    return m_voice;
}

PropertyItem* InspectorModelWithVoiceAndPositionOptions::centerBetweenStaves() const
{
    return m_centerBetweenStaves;
}

bool InspectorModelWithVoiceAndPositionOptions::isMultiStaffInstrument() const
{
    return m_isMultiStaffInstrument;
}

bool InspectorModelWithVoiceAndPositionOptions::isStaveCenteringAvailable() const
{
    return m_isStaveCenteringAvailable;
}

void InspectorModelWithVoiceAndPositionOptions::setIsMultiStaffInstrument(bool v)
{
    if (v == m_isMultiStaffInstrument) {
        return;
    }

    m_isMultiStaffInstrument = v;
    emit isMultiStaffInstrumentChanged(m_isMultiStaffInstrument);
}

void InspectorModelWithVoiceAndPositionOptions::setIsStaveCenteringAvailable(bool v)
{
    if (v == m_isStaveCenteringAvailable) {
        return;
    }

    m_isStaveCenteringAvailable = v;
    emit isStaveCenteringAvailableChanged(m_isStaveCenteringAvailable);
}

void InspectorModelWithVoiceAndPositionOptions::changeVoice(int voice)
{
    if (m_elementList.empty()) {
        return;
    }

    beginCommand(TranslatableString("undoableAction", "Change voice"));

    for (EngravingItem* item : m_elementList) {
        IF_ASSERT_FAILED(item) {
            continue;
        }

        item->undoChangeProperty(Pid::VOICE_ASSIGNMENT, VoiceAssignment::CURRENT_VOICE_ONLY);
        item->undoChangeProperty(Pid::VOICE, voice);
    }

    loadPropertyItem(m_voiceAssignment);
    loadPropertyItem(m_voice);

    updateNotation();
    endCommand();
}
