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
    m_verticalAlign = buildPropertyItem(mu::engraving::Pid::EXCLUDE_VERTICAL_ALIGN);
    m_position = buildPropertyItem(mu::engraving::Pid::POSITION);
    m_bassScale = buildPropertyItem(mu::engraving::Pid::HARMONY_BASS_SCALE, [this](const engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toDouble() / 100);
    }, [this](const engraving::Sid sid, const QVariant& newValue) {
        updateStyleValue(sid, newValue.toDouble() / 100);
        emit requestReloadPropertyItems();
    });
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
    loadPropertyItem(m_verticalAlign);
    loadPropertyItem(m_position);
    loadPropertyItem(m_bassScale, [](const QVariant& elementPropertyValue) -> QVariant {
        return muse::DataFormatter::roundDouble(elementPropertyValue.toDouble()) * 100;
    });
}

void ChordSymbolSettingsModel::resetProperties()
{
    m_isLiteral->resetToDefault();
    m_voicingType->resetToDefault();
    m_durationType->resetToDefault();
    m_verticalAlign->resetToDefault();
    m_position->resetToDefault();
    m_bassScale->resetToDefault();
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

PropertyItem* ChordSymbolSettingsModel::verticalAlign() const
{
    return m_verticalAlign;
}

PropertyItem* ChordSymbolSettingsModel::position() const
{
    return m_position;
}

PropertyItem* ChordSymbolSettingsModel::bassScale() const
{
    return m_bassScale;
}
