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
#include "jumpsettingsmodel.h"

#include "translation.h"

using namespace mu::inspector;

JumpSettingsModel::JumpSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_JUMP);
    setTitle(muse::qtrc("inspector", "Jump"));
    setIcon(muse::ui::IconCode::Code::JUMP);
    createProperties();
}

void JumpSettingsModel::createProperties()
{
    m_jumpTo = buildPropertyItem(mu::engraving::Pid::JUMP_TO);
    m_playUntil = buildPropertyItem(mu::engraving::Pid::PLAY_UNTIL);
    m_continueAt = buildPropertyItem(mu::engraving::Pid::CONTINUE_AT);
    m_hasToPlayRepeats = buildPropertyItem(mu::engraving::Pid::PLAY_REPEATS);
}

void JumpSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::JUMP);
}

void JumpSettingsModel::loadProperties()
{
    loadPropertyItem(m_jumpTo);
    loadPropertyItem(m_playUntil);
    loadPropertyItem(m_continueAt);
    loadPropertyItem(m_hasToPlayRepeats);
}

void JumpSettingsModel::resetProperties()
{
    m_jumpTo->resetToDefault();
    m_playUntil->resetToDefault();
    m_continueAt->resetToDefault();
    m_hasToPlayRepeats->resetToDefault();
}

PropertyItem* JumpSettingsModel::jumpTo() const
{
    return m_jumpTo;
}

PropertyItem* JumpSettingsModel::playUntil() const
{
    return m_playUntil;
}

PropertyItem* JumpSettingsModel::continueAt() const
{
    return m_continueAt;
}

PropertyItem* JumpSettingsModel::hasToPlayRepeats() const
{
    return m_hasToPlayRepeats;
}
