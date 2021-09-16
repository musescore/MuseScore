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

#include "voltasettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

VoltaSettingsModel::VoltaSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : LineSettingsModel(parent, repository)
{
    setTitle(qtrc("inspector", "Volta"));
    createProperties();
}

PropertyItem* VoltaSettingsModel::repeatCount() const
{
    return m_repeatCount;
}

PropertyItem* VoltaSettingsModel::lineType() const
{
    return m_lineType;
}

void VoltaSettingsModel::createProperties()
{
    LineSettingsModel::createProperties();

    m_repeatCount = buildPropertyItem(Ms::Pid::REPEAT_COUNT);
    m_lineType = buildPropertyItem(Ms::Pid::END_HOOK_TYPE);
}

void VoltaSettingsModel::loadProperties()
{
    LineSettingsModel::loadProperties();

    loadPropertyItem(m_repeatCount);
    loadPropertyItem(m_lineType);
}

void VoltaSettingsModel::resetProperties()
{
    LineSettingsModel::resetProperties();

    m_repeatCount->resetToDefault();
    m_lineType->resetToDefault();
}

void VoltaSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::VOLTA);
}

