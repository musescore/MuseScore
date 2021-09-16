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

#include "ottavasettingsmodel.h"

#include "translation.h"
#include "types/hairpintypes.h"

using namespace mu::inspector;

OttavaSettingsModel::OttavaSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : LineSettingsModel(parent, repository)
{
    setTitle(qtrc("inspector", "Ottava"));
    createProperties();
}

PropertyItem* OttavaSettingsModel::ottavaType() const
{
    return m_ottavaType;
}

PropertyItem* OttavaSettingsModel::showNumbersOnly() const
{
    return m_showNumbersOnly;
}

void OttavaSettingsModel::createProperties()
{
    LineSettingsModel::createProperties();

    m_ottavaType = buildPropertyItem(Ms::Pid::OTTAVA_TYPE);
    m_showNumbersOnly = buildPropertyItem(Ms::Pid::NUMBERS_ONLY);
}

void OttavaSettingsModel::loadProperties()
{
    LineSettingsModel::loadProperties();

    loadPropertyItem(m_ottavaType);
    loadPropertyItem(m_showNumbersOnly);
}

void OttavaSettingsModel::resetProperties()
{
    LineSettingsModel::resetProperties();

    m_ottavaType->resetToDefault();
    m_showNumbersOnly->resetToDefault();
}

void OttavaSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::OTTAVA);
}
