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
#include "pedalsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

PedalSettingsModel::PedalSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : LineSettingsModel(parent, repository, Ms::ElementType::PEDAL)
{
    setModelType(InspectorModelType::TYPE_PEDAL);
    setTitle(qtrc("inspector", "Pedal"));
    setIcon(ui::IconCode::Code::PEDAL_MARKING);

    createProperties();
}

PropertyItem* PedalSettingsModel::showPedalSymbol() const
{
    return m_showPedalSymbol;
}

void PedalSettingsModel::createProperties()
{
    LineSettingsModel::createProperties();

    m_showPedalSymbol = buildPropertyItem(Ms::Pid::SYMBOL);
}

void PedalSettingsModel::loadProperties()
{
    LineSettingsModel::loadProperties();

    loadPropertyItem(m_showPedalSymbol);
}

void PedalSettingsModel::resetProperties()
{
    LineSettingsModel::resetProperties();

    m_showPedalSymbol->resetToDefault();
}
