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
#include "spacersettingsmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::inspector;

SpacerSettingsModel::SpacerSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_SPACER);
    setTitle(qtrc("inspector", "Spacer"));
    setIcon(ui::IconCode::Code::SPACER);
    createProperties();
}

void SpacerSettingsModel::createProperties()
{
    m_spacerHeight = buildPropertyItem(mu::engraving::Pid::SPACE);
}

void SpacerSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::SPACER);
}

void SpacerSettingsModel::loadProperties()
{
    loadPropertyItem(m_spacerHeight, formatDoubleFunc);
}

void SpacerSettingsModel::resetProperties()
{
    m_spacerHeight->resetToDefault();
}

void SpacerSettingsModel::onNotationChanged()
{
    loadPropertyItem(m_spacerHeight, formatDoubleFunc);
}

PropertyItem* SpacerSettingsModel::spacerHeight() const
{
    return m_spacerHeight;
}
