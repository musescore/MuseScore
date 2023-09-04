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
#include "articulationsettingsmodel.h"

#include "engraving/dom/articulation.h"

#include "log.h"
#include "translation.h"

using namespace mu::inspector;

ArticulationSettingsModel::ArticulationSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_ARTICULATION);
    setTitle(qtrc("inspector", "Articulation"));
    setIcon(ui::IconCode::Code::ARTICULATION);
    createProperties();
}

void ArticulationSettingsModel::createProperties()
{
    m_placement = buildPropertyItem(mu::engraving::Pid::ARTICULATION_ANCHOR);
}

void ArticulationSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::ARTICULATION);
}

void ArticulationSettingsModel::loadProperties()
{
    loadPropertyItem(m_placement);
}

void ArticulationSettingsModel::resetProperties()
{
    m_placement->resetToDefault();
}

PropertyItem* ArticulationSettingsModel::placement() const
{
    return m_placement;
}
