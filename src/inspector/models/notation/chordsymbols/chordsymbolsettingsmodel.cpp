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
#include "chordsymbolsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

ChordSymbolSettingsModel::ChordSymbolSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_CHORD_SYMBOL);
    setTitle(qtrc("inspector", "Chord symbol"));
    createProperties();
}

void ChordSymbolSettingsModel::createProperties()
{
    m_isLiteral = buildPropertyItem(Ms::Pid::HARMONY_VOICE_LITERAL);
    m_voicingType = buildPropertyItem(Ms::Pid::HARMONY_VOICING);
    m_durationType = buildPropertyItem(Ms::Pid::HARMONY_DURATION);
}

void ChordSymbolSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HARMONY);
}

void ChordSymbolSettingsModel::loadProperties()
{
    loadPropertyItem(m_isLiteral);
    loadPropertyItem(m_voicingType);
    loadPropertyItem(m_durationType);
}

void ChordSymbolSettingsModel::resetProperties()
{
    m_isLiteral->resetToDefault();
    m_voicingType->resetToDefault();
    m_durationType->resetToDefault();
}

PropertyItem* ChordSymbolSettingsModel::isLiteral() const
{
    return m_isLiteral;
}

PropertyItem* ChordSymbolSettingsModel::voicingType() const
{
    return m_voicingType;
}

PropertyItem* ChordSymbolSettingsModel::durationType() const
{
    return m_durationType;
}
