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
#include "articulationsettingsmodel.h"

#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"

#include "log.h"
#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

ArticulationSettingsModel::ArticulationSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_ARTICULATION);
    setTitle(muse::qtrc("inspector", "Articulation"));
    setIcon(muse::ui::IconCode::Code::ARTICULATION);
    createProperties();
}

void ArticulationSettingsModel::createProperties()
{
    m_placement = buildPropertyItem(Pid::ARTICULATION_ANCHOR,
                                    [this](const Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        updateIsArticStemHAlignAvailable();
    });
    m_articStemHAlign = buildPropertyItem(Pid::ARTIC_STEM_H_ALIGN);
}

void ArticulationSettingsModel::requestElements()
{
    m_elementList.clear();
    for (EngravingItem* it : m_repository->findElementsByType(ElementType::ARTICULATION)) {
        if (toArticulation(it)->chordRest()->isChord()) {
            m_elementList << it;
        }
    }
    for (EngravingItem* it : m_repository->findElementsByType(ElementType::CHORD)) {
        if (!toChord(it)->articulations().empty()) {
            m_elementList << it;
        }
    }
}

void ArticulationSettingsModel::loadProperties()
{
    loadPropertyItem(m_placement);
    loadPropertyItem(m_articStemHAlign);
    updateIsArticStemHAlignAvailable();
}

void ArticulationSettingsModel::resetProperties()
{
    m_placement->resetToDefault();
    m_articStemHAlign->resetToDefault();
}

void ArticulationSettingsModel::updateIsArticStemHAlignAvailable()
{
    bool available = false;
    for (EngravingItem* item : m_elementList) {
        if (!item->isArticulation()) {
            continue;
        }
        Articulation* a = toArticulation(item);
        Chord* chord = a->chordRest()->isChord() ? toChord(a->chordRest()) : nullptr;
        if (chord && a->up() == chord->up()) {
            available = true;
            break;
        }
    }
    setIsArticStemHAlignAvailable(available);
}

void ArticulationSettingsModel::setIsArticStemHAlignAvailable(bool isArticStemHAlignAvailable)
{
    if (m_isArticStemHAlignAvailable == isArticStemHAlignAvailable) {
        return;
    }

    m_isArticStemHAlignAvailable = isArticStemHAlignAvailable;
    emit isArticStemHAlignAvailableChanged(isArticStemHAlignAvailable);
}

bool ArticulationSettingsModel::isArticStemHAlignAvailable() const
{
    return m_isArticStemHAlignAvailable;
}

PropertyItem* ArticulationSettingsModel::placement() const
{
    return m_placement;
}

PropertyItem* ArticulationSettingsModel::articStemHAlign() const
{
    return m_articStemHAlign;
}
