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
#include "glissandosettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

GlissandoSettingsModel::GlissandoSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_GLISSANDO);
    setTitle(qtrc("inspector", "Glissando"));
    setIcon(ui::IconCode::Code::GLISSANDO);
    createProperties();
}

void GlissandoSettingsModel::createProperties()
{
    m_lineType = buildPropertyItem(Ms::Pid::GLISS_TYPE);
}

void GlissandoSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::GLISSANDO);
}

void GlissandoSettingsModel::loadProperties()
{
    loadPropertyItem(m_lineType);
}

void GlissandoSettingsModel::resetProperties()
{
    m_lineType->resetToDefault();
}

PropertyItem* GlissandoSettingsModel::lineType() const
{
    return m_lineType;
}
