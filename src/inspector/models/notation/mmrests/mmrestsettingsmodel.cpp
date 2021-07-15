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
#include "mmrestsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

MMRestSettingsModel::MMRestSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_MMREST);
    setTitle(qtrc("inspector", "Multimeasure rest"));
    setIcon(ui::IconCode::Code::MULTIMEASURE_REST);
    createProperties();
}

void MMRestSettingsModel::createProperties()
{
    m_isNumberVisible = buildPropertyItem(Ms::Pid::MMREST_NUMBER_VISIBLE);
    m_numberPosition = buildPropertyItem(Ms::Pid::MMREST_NUMBER_POS);
}

void MMRestSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::MMREST);
}

void MMRestSettingsModel::loadProperties()
{
    loadPropertyItem(m_isNumberVisible);
    loadPropertyItem(m_numberPosition);
}

void MMRestSettingsModel::resetProperties()
{
    m_isNumberVisible->resetToDefault();
    m_numberPosition->resetToDefault();
}

PropertyItem* MMRestSettingsModel::isNumberVisible() const
{
    return m_isNumberVisible;
}

PropertyItem* MMRestSettingsModel::numberPosition() const
{
    return m_numberPosition;
}
