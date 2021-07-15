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
#include "markersettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

MarkerSettingsModel::MarkerSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_MARKER);
    setTitle(qtrc("inspector", "Marker"));
    setIcon(ui::IconCode::Code::MARKER);
    createProperties();
}

void MarkerSettingsModel::createProperties()
{
    m_type = buildPropertyItem(Ms::Pid::MARKER_TYPE);
    m_label = buildPropertyItem(Ms::Pid::LABEL);
}

void MarkerSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::MARKER);
}

void MarkerSettingsModel::loadProperties()
{
    loadPropertyItem(m_type);
    loadPropertyItem(m_label);
}

void MarkerSettingsModel::resetProperties()
{
    m_type->resetToDefault();
    m_label->resetToDefault();
}

PropertyItem* MarkerSettingsModel::type() const
{
    return m_type;
}

PropertyItem* MarkerSettingsModel::label() const
{
    return m_label;
}
