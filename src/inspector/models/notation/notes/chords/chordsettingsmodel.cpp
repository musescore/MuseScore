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
#include "chordsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

ChordSettingsModel::ChordSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_CHORD);
    setTitle(qtrc("inspector", "Chord"));

    createProperties();
}

void ChordSettingsModel::createProperties()
{
    m_isStemless = buildPropertyItem(Pid::NO_STEM);
}

void ChordSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(ElementType::CHORD);
}

void ChordSettingsModel::loadProperties()
{
    loadPropertyItem(m_isStemless);
}

void ChordSettingsModel::resetProperties()
{
    m_isStemless->resetToDefault();
}

PropertyItem* ChordSettingsModel::isStemless() const
{
    return m_isStemless;
}
