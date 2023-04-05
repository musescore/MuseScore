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
#include "temposettingsmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;

TempoSettingsModel::TempoSettingsModel(QObject* parent, IElementRepositoryService* repository, InspectorModelType modelType)
    : AbstractInspectorModel(parent, repository)
{
    Q_ASSERT(modelType == InspectorModelType::TYPE_TEMPO || modelType == InspectorModelType::TYPE_TEMPO_RESTORE_PREVIOUS);

    setModelType(modelType);

    if (modelType == InspectorModelType::TYPE_TEMPO) {
        setTitle(qtrc("inspector", "Tempo"));
    } else {
        setTitle(qtrc("inspector", "Restore previous tempo (a tempo)"));
    }

    setIcon(ui::IconCode::Code::METRONOME);
    createProperties();
}

void TempoSettingsModel::createProperties()
{
    m_isFollowText
        = buildPropertyItem(mu::engraving::Pid::TEMPO_FOLLOW_TEXT, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        emit requestReloadPropertyItems();
    });

    m_tempo = buildPropertyItem(mu::engraving::Pid::TEMPO);
}

void TempoSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::TEMPO_TEXT);
}

void TempoSettingsModel::loadProperties()
{
    loadPropertyItem(m_isFollowText);
    loadPropertyItem(m_tempo, formatDoubleFunc);
}

void TempoSettingsModel::resetProperties()
{
    m_isFollowText->resetToDefault();
    m_tempo->resetToDefault();
}

PropertyItem* TempoSettingsModel::isFollowText() const
{
    return m_isFollowText;
}

PropertyItem* TempoSettingsModel::tempo() const
{
    return m_tempo;
}
