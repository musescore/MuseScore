/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "breathplaybackmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::propertiespanel;

BreathPlaybackModel::BreathPlaybackModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setTitle(muse::qtrc("propertiespanel", "Breaths & pauses"));
    setModelType(PropertiesPanelModelType::TYPE_BREATH);

    createProperties();
}

void BreathPlaybackModel::createProperties()
{
    m_pauseTime = buildPropertyItem(mu::engraving::Pid::PAUSE);
}

void BreathPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::BREATH);
}

void BreathPlaybackModel::loadProperties()
{
    loadPropertyItem(m_pauseTime, formatDoubleFunc);
}

PropertyItem* BreathPlaybackModel::pauseTime() const
{
    return m_pauseTime;
}
