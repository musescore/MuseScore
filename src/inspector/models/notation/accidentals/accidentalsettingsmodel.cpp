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
#include "accidentalsettingsmodel.h"

#include "translation.h"

#include "engraving/dom/accidental.h"

using namespace mu::inspector;
using namespace mu::engraving;

AccidentalSettingsModel::AccidentalSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_ACCIDENTAL);
    setTitle(muse::qtrc("inspector", "Accidental"));
    setIcon(muse::ui::IconCode::Code::ACCIDENTAL_SHARP);
    createProperties();
}

void AccidentalSettingsModel::createProperties()
{
    m_bracketType = buildPropertyItem(mu::engraving::Pid::ACCIDENTAL_BRACKET);
    m_isSmall = buildPropertyItem(mu::engraving::Pid::SMALL);
    m_stackingOrderOffset = buildPropertyItem(mu::engraving::Pid::ACCIDENTAL_STACKING_ORDER_OFFSET,
                                              [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        loadPropertyItem(m_stackingOrderOffset);
    });
}

void AccidentalSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::ACCIDENTAL);
}

void AccidentalSettingsModel::loadProperties()
{
    loadPropertyItem(m_bracketType);
    loadPropertyItem(m_isSmall);
    updateIsSmallAvailable();
    loadPropertyItem(m_stackingOrderOffset);
    updateIsStackingOrderAvailableAndEnabled();
}

void AccidentalSettingsModel::resetProperties()
{
    m_bracketType->resetToDefault();
    m_stackingOrderOffset->resetToDefault();
}

PropertyItem* AccidentalSettingsModel::bracketType() const
{
    return m_bracketType;
}

PropertyItem* AccidentalSettingsModel::isSmall() const
{
    return m_isSmall;
}

bool AccidentalSettingsModel::isSmallAvailable() const
{
    return m_isSmallAvailable;
}

PropertyItem* AccidentalSettingsModel::stackingOrderOffset() const
{
    return m_stackingOrderOffset;
}

bool AccidentalSettingsModel::isStackingOrderAvailable() const
{
    return m_isStackinOrderAvailable;
}

bool AccidentalSettingsModel::isStackingOrderEnabled() const
{
    return m_isStackingOrderEnabled;
}

void AccidentalSettingsModel::updateIsSmallAvailable()
{
    bool available = true;
    for (mu::engraving::EngravingItem* item : m_elementList) {
        mu::engraving::EngravingItem* parent = item ? item->parentItem() : nullptr;
        if (parent && (parent->isOrnament() || parent->isTrillSegment())) {
            available = false;
            break;
        }
    }
    setIsSmallAvailable(available);
}

void AccidentalSettingsModel::setIsSmallAvailable(bool available)
{
    if (m_isSmallAvailable == available) {
        return;
    }

    m_isSmallAvailable = available;
    emit isSmallAvailableChanged(m_isSmallAvailable);
}

void AccidentalSettingsModel::updateIsStackingOrderAvailableAndEnabled()
{
    setIsStackingOrderEnabled(m_elementList.size() == 1);

    for (EngravingItem* item : m_elementList) {
        if (!item->isAccidental() || !toAccidental(item)->note()) {
            continue;
        }
        setIsStackingOrderEnabled(item->addToSkyline());

        Segment* segment = toAccidental(item)->note()->chord()->segment();
        track_idx_t startTrack = trackZeroVoice(item->track());
        track_idx_t endTrack = startTrack + VOICES;
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            EngravingItem* elem = segment->elementAt(track);
            if (!elem || !elem->isChord()) {
                continue;
            }
            for (Note* note : toChord(elem)->notes()) {
                if (note->accidental() && note->accidental()->ldata()->column > 0) {
                    setIsStackingOrderAvailable(true);
                    return;
                }
            }
        }
    }

    setIsStackingOrderAvailable(false);
}

void AccidentalSettingsModel::setIsStackingOrderAvailable(bool available)
{
    if (m_isStackinOrderAvailable == available) {
        return;
    }

    m_isStackinOrderAvailable = available;
    emit isStackingOrderAvailableChanged(m_isStackinOrderAvailable);
}

void AccidentalSettingsModel::setIsStackingOrderEnabled(bool enabled)
{
    if (m_isStackingOrderEnabled == enabled) {
        return;
    }

    m_isStackingOrderEnabled = enabled;
    emit isStackingOrderEnabledChanged(m_isStackingOrderEnabled);
}
