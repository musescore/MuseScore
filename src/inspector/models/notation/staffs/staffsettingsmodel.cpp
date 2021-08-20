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
#include "staffsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

StaffSettingsModel::StaffSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_STAFF);
    setTitle(qtrc("inspector", "Staff"));
    setIcon(ui::IconCode::Code::STAFF_TYPE_CHANGE);
    createProperties();
}

void StaffSettingsModel::createProperties()
{
    m_barlinesSpanFrom = buildPropertyItem(Ms::Pid::STAFF_BARLINE_SPAN_FROM);
    m_barlinesSpanTo = buildPropertyItem(Ms::Pid::STAFF_BARLINE_SPAN_TO);
}

void StaffSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::STAFF);
}

void StaffSettingsModel::loadProperties()
{
    loadPropertyItem(m_barlinesSpanFrom);
    loadPropertyItem(m_barlinesSpanTo);
}

void StaffSettingsModel::resetProperties()
{
    m_barlinesSpanFrom->resetToDefault();
    m_barlinesSpanTo->resetToDefault();
}

PropertyItem* StaffSettingsModel::barlinesSpanFrom() const
{
    return m_barlinesSpanFrom;
}

PropertyItem* StaffSettingsModel::barlinesSpanTo() const
{
    return m_barlinesSpanTo;
}
