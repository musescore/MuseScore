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
#include "clefsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

ClefSettingsModel::ClefSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_CLEF);
    setTitle(qtrc("inspector", "Clef"));
    setIcon(ui::IconCode::Code::CLEF_BASS);
    createProperties();
}

void ClefSettingsModel::createProperties()
{
    m_shouldShowCourtesy = buildPropertyItem(Ms::Pid::SHOW_COURTESY);
}

void ClefSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::CLEF);
}

void ClefSettingsModel::loadProperties()
{
    loadPropertyItem(m_shouldShowCourtesy);
}

void ClefSettingsModel::resetProperties()
{
    m_shouldShowCourtesy->resetToDefault();
}

PropertyItem* ClefSettingsModel::shouldShowCourtesy() const
{
    return m_shouldShowCourtesy;
}
