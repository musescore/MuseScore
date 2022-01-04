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
#include "ornamentsettingsmodel.h"

#include "libmscore/articulation.h"

#include "log.h"
#include "translation.h"

using namespace mu::inspector;

OrnamentSettingsModel::OrnamentSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_ORNAMENT);
    setTitle(qtrc("inspector", "Ornament"));
    setIcon(ui::IconCode::Code::ORNAMENT);
    createProperties();
}

void OrnamentSettingsModel::createProperties()
{
    m_performanceType = buildPropertyItem(Ms::Pid::ORNAMENT_STYLE);
    m_placement = buildPropertyItem(Ms::Pid::ARTICULATION_ANCHOR);
}

void OrnamentSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::ARTICULATION, [](const Ms::EngravingItem* element) -> bool {
        IF_ASSERT_FAILED(element) {
            return false;
        }

        const Ms::Articulation* articulation = Ms::toArticulation(element);
        IF_ASSERT_FAILED(articulation) {
            return false;
        }

        return articulation->isOrnament();
    });
}

void OrnamentSettingsModel::loadProperties()
{
    loadPropertyItem(m_performanceType);
    loadPropertyItem(m_placement);
}

void OrnamentSettingsModel::resetProperties()
{
    m_performanceType->resetToDefault();
    m_placement->resetToDefault();
}

PropertyItem* OrnamentSettingsModel::performanceType() const
{
    return m_performanceType;
}

PropertyItem* OrnamentSettingsModel::placement() const
{
    return m_placement;
}
