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
#include "tremolosettingsmodel.h"

#include <QList>

#include "translation.h"

using namespace mu::inspector;

TremoloSettingsModel::TremoloSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_TREMOLO);
    setTitle(qtrc("inspector", "Tremolos"));
    setIcon(ui::IconCode::Code::TREMOLO_TWO_NOTES);
    createProperties();
}

void TremoloSettingsModel::createProperties()
{
    m_style = buildPropertyItem(Ms::Pid::TREMOLO_STYLE);
}

void TremoloSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::TREMOLO);
}

void TremoloSettingsModel::loadProperties()
{
    loadPropertyItem(m_style);
}

void TremoloSettingsModel::resetProperties()
{
    m_style->resetToDefault();
}

PropertyItem* TremoloSettingsModel::style() const
{
    return m_style;
}
