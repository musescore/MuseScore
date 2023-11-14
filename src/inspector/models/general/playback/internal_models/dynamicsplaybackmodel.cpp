/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "dynamicsplaybackmodel.h"

#include "types/dynamictypes.h"

#include "translation.h"

using namespace mu::inspector;

DynamicsPlaybackModel::DynamicsPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(qtrc("inspector", "Dynamics"));
    setModelType(InspectorModelType::TYPE_DYNAMIC);

    createProperties();
}

PropertyItem* DynamicsPlaybackModel::applyToVoice() const
{
    return m_applyToVoice;
}

void DynamicsPlaybackModel::createProperties()
{
    m_applyToVoice = buildPropertyItem(mu::engraving::Pid::VOICE, [this](const mu::engraving::Pid, const QVariant& newValue) {
        setDynamicsVoice(newValue.toInt());
    });
}

void DynamicsPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::DYNAMIC);
}

void DynamicsPlaybackModel::loadProperties()
{
    loadApplyToVoiceProperty();
}

void DynamicsPlaybackModel::resetProperties()
{
    m_applyToVoice->resetToDefault();
}

void DynamicsPlaybackModel::onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet, const mu::engraving::StyleIdSet&)
{
    if (mu::contains(changedPropertyIdSet, mu::engraving::Pid::VOICE)
        || mu::contains(changedPropertyIdSet, mu::engraving::Pid::APPLY_TO_ALL_VOICES)) {
        loadApplyToVoiceProperty();
    }
}

void DynamicsPlaybackModel::setDynamicsVoice(int value)
{
    if (m_elementList.empty()) {
        return;
    }

    beginCommand();

    bool applyToAllVoices = static_cast<DynamicTypes::ApplyToVoice>(value) == DynamicTypes::ApplyToVoice::AllVoices;

    for (engraving::EngravingItem* item : m_elementList) {
        if (applyToAllVoices) {
            item->undoChangeProperty(mu::engraving::Pid::APPLY_TO_ALL_VOICES, applyToAllVoices);
        } else {
            item->undoResetProperty(mu::engraving::Pid::APPLY_TO_ALL_VOICES);
            item->undoChangeProperty(mu::engraving::Pid::VOICE, value);
        }
    }

    endCommand();
    updateNotation();
}

void DynamicsPlaybackModel::loadApplyToVoiceProperty()
{
    QVariant applyToVoiceValue;

    for (engraving::EngravingItem* item : m_elementList) {
        mu::engraving::voice_idx_t voice = item->voice();
        bool applyToAllVoices = item->getProperty(mu::engraving::Pid::APPLY_TO_ALL_VOICES).toBool();
        QVariant currentValue = applyToAllVoices ? static_cast<int>(DynamicTypes::ApplyToVoice::AllVoices) : static_cast<int>(voice);

        if (applyToVoiceValue.isValid()) {
            if (applyToVoiceValue != currentValue) {
                applyToVoiceValue = QVariant();
                break;
            }
        }

        applyToVoiceValue = currentValue;
    }

    m_applyToVoice->setIsEnabled(true);
    m_applyToVoice->fillValues(applyToVoiceValue, 0);
}
