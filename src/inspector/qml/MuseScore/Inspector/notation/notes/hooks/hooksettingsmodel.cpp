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
#include "hooksettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

HookSettingsModel::HookSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_HOOK);
    setTitle(muse::qtrc("inspector", "Flag")); // internally called "Hook", but "Flag" in SMuFL, so here externally too

    createProperties();
}

void HookSettingsModel::createProperties()
{
    m_offset = buildPointFPropertyItem(mu::engraving::Pid::OFFSET);
}

void HookSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::HOOK);
}

void HookSettingsModel::loadProperties()
{
    loadPropertyItem(m_offset);
}

void HookSettingsModel::resetProperties()
{
    m_offset->resetToDefault();
}

PropertyItem* HookSettingsModel::offset() const
{
    return m_offset;
}
