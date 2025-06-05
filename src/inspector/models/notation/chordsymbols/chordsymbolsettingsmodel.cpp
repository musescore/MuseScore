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
#include "chordsymbolsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

ChordSymbolSettingsModel::ChordSymbolSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_CHORD_SYMBOL);
    setTitle(muse::qtrc("inspector", "Chord symbol"));
    setIcon(muse::ui::IconCode::Code::CHORD_SYMBOL);
    createProperties();
}

void ChordSymbolSettingsModel::createProperties()
{
    m_isLiteral = buildPropertyItem(mu::engraving::Pid::HARMONY_VOICE_LITERAL);
    m_voicingType = buildPropertyItem(mu::engraving::Pid::HARMONY_VOICING);
    m_durationType = buildPropertyItem(mu::engraving::Pid::HARMONY_DURATION);
}

void ChordSymbolSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::HARMONY);
    updateHasLinkedFretboardDiagram();
    updateIsDurationAvailable();
}

void ChordSymbolSettingsModel::loadProperties()
{
    loadPropertyItem(m_isLiteral);
    loadPropertyItem(m_voicingType);

    loadPropertyItem(m_durationType);
    updateIsDurationAvailable();
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

void ChordSymbolSettingsModel::addFretboardDiagram()
{
    dispatcher()->dispatch("add-fretboard-diagram");
}

bool ChordSymbolSettingsModel::hasLinkedFretboardDiagram() const
{
    return m_hasLinkedFretboardDiagram;
}

void ChordSymbolSettingsModel::setHasLinkedFretboardDiagram(bool has)
{
    if (m_hasLinkedFretboardDiagram == has) {
        return;
    }

    m_hasLinkedFretboardDiagram = has;
    emit hasLinkedFretboardDiagramChanged();
}

void ChordSymbolSettingsModel::updateHasLinkedFretboardDiagram()
{
    bool hasHarmonyWhithoutFretboardDiagram = false;

    for (mu::engraving::EngravingItem* item : m_elementList) {
        engraving::EngravingObject* parent = item->explicitParent();
        if (parent && !parent->isFretDiagram()) {
            hasHarmonyWhithoutFretboardDiagram = true;
            break;
        }
    }

    setHasLinkedFretboardDiagram(!hasHarmonyWhithoutFretboardDiagram);
}

void ChordSymbolSettingsModel::updateIsDurationAvailable()
{
    bool available = true;

    for (engraving::EngravingItem* item : m_elementList) {
        if (engraving::toHarmony(item)->isInFretBox()) {
            available = false;
            break;
        }
    }

    m_durationType->setIsVisible(available);
}
