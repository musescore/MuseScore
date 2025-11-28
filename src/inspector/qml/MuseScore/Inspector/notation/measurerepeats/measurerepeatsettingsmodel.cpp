/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "measurerepeatsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

MeasureRepeatSettingsModel::MeasureRepeatSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_MEASURE_REPEAT);
    setTitle(muse::qtrc("inspector", "Measure repeat"));
    setIcon(muse::ui::IconCode::Code::MEASURE_REPEAT);
    createProperties();
}

void MeasureRepeatSettingsModel::createProperties()
{
    m_numberPosition = buildPropertyItem(mu::engraving::Pid::MEASURE_REPEAT_NUMBER_POS);
}

void MeasureRepeatSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::MEASURE_REPEAT);
}

void MeasureRepeatSettingsModel::loadProperties()
{
    loadPropertyItem(m_numberPosition);
}

void MeasureRepeatSettingsModel::resetProperties()
{
    m_numberPosition->resetToDefault();
}

PropertyItem* MeasureRepeatSettingsModel::numberPosition() const
{
    return m_numberPosition;
}
