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
#include "glissandoplaybackmodel.h"

#include "translation.h"

using namespace mu::inspector;

GlissandoPlaybackModel::GlissandoPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(qtrc("inspector", "Glissando"));

    createProperties();
}

void GlissandoPlaybackModel::createProperties()
{
    m_styleType = buildPropertyItem(Ms::Pid::GLISS_STYLE);
}

void GlissandoPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::GLISSANDO);
}

void GlissandoPlaybackModel::loadProperties()
{
    loadPropertyItem(m_styleType);
}

void GlissandoPlaybackModel::resetProperties()
{
    m_styleType->resetToDefault();
}

PropertyItem* GlissandoPlaybackModel::styleType() const
{
    return m_styleType;
}
