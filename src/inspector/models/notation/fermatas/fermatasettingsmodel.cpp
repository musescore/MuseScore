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
#include "fermatasettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

FermataSettingsModel::FermataSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_FERMATA);
    setTitle(qtrc("inspector", "Fermata"));
    setIcon(ui::IconCode::Code::FERMATA);
    createProperties();
}

void FermataSettingsModel::createProperties()
{
    m_placementType = buildPropertyItem(Ms::Pid::PLACEMENT);
}

void FermataSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::FERMATA);
}

void FermataSettingsModel::loadProperties()
{
    loadPropertyItem(m_placementType);
}

void FermataSettingsModel::resetProperties()
{
    m_placementType->resetToDefault();
}

PropertyItem* FermataSettingsModel::placementType() const
{
    return m_placementType;
}
