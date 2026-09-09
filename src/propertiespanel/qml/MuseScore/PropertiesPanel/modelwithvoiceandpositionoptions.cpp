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

#include "modelwithvoiceandpositionoptions.h"

#include "engraving/dom/part.h"

using namespace mu::propertiespanel;
using namespace mu::engraving;

ModelWithVoiceAndPositionOptions::ModelWithVoiceAndPositionOptions(QObject* parent,
                                                                   const muse::modularity::ContextPtr& iocCtx,
                                                                   IElementRepositoryService* repository,
                                                                   ElementType elementType)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository, elementType)
{
    createProperties();
}

void ModelWithVoiceAndPositionOptions::createProperties()
{
    m_voiceBasedPosition = buildPropertyItem(Pid::DIRECTION, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        updateStaveCenteringFlags();
    });
    m_voiceAssignment = buildPropertyItem(Pid::VOICE_ASSIGNMENT);
    m_voice = buildPropertyItem(Pid::VOICE);
    m_centerBetweenStaves = buildPropertyItem(Pid::CENTER_BETWEEN_STAVES);
    updateIsMultiStaffInstrument();
    updateStaveCenteringFlags();
}

void ModelWithVoiceAndPositionOptions::loadProperties()
{
    loadPropertyItem(m_voiceBasedPosition);
    loadPropertyItem(m_voiceAssignment);
    loadPropertyItem(m_voice);
    loadPropertyItem(m_centerBetweenStaves);
    updateIsMultiStaffInstrument();
    updateStaveCenteringFlags();
}

void ModelWithVoiceAndPositionOptions::onNotationChanged(const PropertyIdSet&, const StyleIdSet&)
{
    loadProperties();
}

void ModelWithVoiceAndPositionOptions::updateIsMultiStaffInstrument()
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

void ModelWithVoiceAndPositionOptions::updateStaveCenteringFlags()
{
    bool isApplicable = true;
    bool isAvailable = true;

    for (EngravingItem* item : m_elementList) {
        const bool otherStaffAbove = item->staffToCenterAgainst(true) != nullptr;
        const bool otherStaffBelow = item->staffToCenterAgainst(false) != nullptr;
        if (!otherStaffAbove && !otherStaffBelow) {
            isApplicable = false;
            isAvailable = false;
            break;
        }

        const DirectionV itemDirection = item->getProperty(Pid::DIRECTION).value<DirectionV>();
        const bool otherStaffOnRelevantSide = (itemDirection != DirectionV::DOWN && otherStaffAbove)
                                              || (itemDirection != DirectionV::UP && otherStaffBelow);
        if (!otherStaffOnRelevantSide) {
            isAvailable = false;
        }
    }

    setIsStaveCenteringApplicable(isApplicable);
    setIsStaveCenteringAvailable(isAvailable);
}

PropertyItem* ModelWithVoiceAndPositionOptions::voiceBasedPosition() const
{
    return m_voiceBasedPosition;
}

PropertyItem* ModelWithVoiceAndPositionOptions::voiceAssignment() const
{
    return m_voiceAssignment;
}

PropertyItem* ModelWithVoiceAndPositionOptions::voice() const
{
    return m_voice;
}

PropertyItem* ModelWithVoiceAndPositionOptions::centerBetweenStaves() const
{
    return m_centerBetweenStaves;
}

bool ModelWithVoiceAndPositionOptions::isMultiStaffInstrument() const
{
    return m_isMultiStaffInstrument;
}

bool ModelWithVoiceAndPositionOptions::isStaveCenteringApplicable() const
{
    return m_isStaveCenteringApplicable;
}

bool ModelWithVoiceAndPositionOptions::isStaveCenteringAvailable() const
{
    return m_isStaveCenteringAvailable;
}

void ModelWithVoiceAndPositionOptions::setIsMultiStaffInstrument(bool v)
{
    if (v == m_isMultiStaffInstrument) {
        return;
    }

    m_isMultiStaffInstrument = v;
    emit isMultiStaffInstrumentChanged(m_isMultiStaffInstrument);
}

void ModelWithVoiceAndPositionOptions::setIsStaveCenteringApplicable(bool v)
{
    if (v == m_isStaveCenteringApplicable) {
        return;
    }

    m_isStaveCenteringApplicable = v;
    emit isStaveCenteringApplicableChanged(m_isStaveCenteringApplicable);
}

void ModelWithVoiceAndPositionOptions::setIsStaveCenteringAvailable(bool v)
{
    if (v == m_isStaveCenteringAvailable) {
        return;
    }

    m_isStaveCenteringAvailable = v;
    emit isStaveCenteringAvailableChanged(m_isStaveCenteringAvailable);
}

void ModelWithVoiceAndPositionOptions::changeVoice(int voice)
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
        item->undoChangeProperty(Pid::VOICE, static_cast<voice_idx_t>(voice));
    }

    loadPropertyItem(m_voiceAssignment);
    loadPropertyItem(m_voice);

    updateNotation();
    endCommand();
}

QString ModelWithVoiceAndPositionOptions::shortcutUseVoice1() const
{
    return shortcutsForActionCode("voice-1");
}

QString ModelWithVoiceAndPositionOptions::shortcutUseVoice2() const
{
    return shortcutsForActionCode("voice-2");
}

QString ModelWithVoiceAndPositionOptions::shortcutUseVoice3() const
{
    return shortcutsForActionCode("voice-3");
}

QString ModelWithVoiceAndPositionOptions::shortcutUseVoice4() const
{
    return shortcutsForActionCode("voice-4");
}

QString ModelWithVoiceAndPositionOptions::shortcutUseAllVoicesInstrument() const
{
    return shortcutsForActionCode("voice-assignment-all-in-instrument");
}

QString ModelWithVoiceAndPositionOptions::shortcutUseAllVoicesStaff() const
{
    return shortcutsForActionCode("voice-assignment-all-in-staff");
}
