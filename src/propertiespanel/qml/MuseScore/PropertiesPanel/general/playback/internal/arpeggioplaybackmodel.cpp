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
#include "arpeggioplaybackmodel.h"

#include "dataformatter.h"

#include "translation.h"

using namespace mu::propertiespanel;

ArpeggioPlaybackModel::ArpeggioPlaybackModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                             IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setTitle(muse::qtrc("propertiespanel", "Arpeggio"));
    setModelType(PropertiesPanelModelType::TYPE_ARPEGGIO);

    createProperties();
}

void ArpeggioPlaybackModel::createProperties()
{
    m_stretch = buildPropertyItem(mu::engraving::Pid::TIME_STRETCH);
}

void ArpeggioPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::ARPEGGIO);
}

void ArpeggioPlaybackModel::loadProperties()
{
    loadPropertyItem(m_stretch, formatDoubleFunc);
}

PropertyItem* ArpeggioPlaybackModel::stretch() const
{
    return m_stretch;
}
