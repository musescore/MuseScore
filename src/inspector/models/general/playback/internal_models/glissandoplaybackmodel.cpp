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
#include "dom/glissando.h"

using namespace mu::inspector;
using namespace mu::engraving;

GlissandoPlaybackModel::GlissandoPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(qtrc("inspector", "Glissando"));
    setModelType(InspectorModelType::TYPE_GLISSANDO);

    createProperties();
}

void GlissandoPlaybackModel::createProperties()
{
    m_styleType = buildPropertyItem(mu::engraving::Pid::GLISS_STYLE);
}

void GlissandoPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::GLISSANDO);
}

void GlissandoPlaybackModel::loadProperties()
{
    loadPropertyItem(m_styleType);
}

void GlissandoPlaybackModel::resetProperties()
{
    m_styleType->resetToDefault();
}

bool GlissandoPlaybackModel::isHarpGliss() const
{
    for (mu::engraving::EngravingItem* element : selection()->elements()) {
        if (element->isGlissandoSegment()) {
            Glissando* gliss = toGlissando(element);
            return gliss->isHarpGliss();
        }
    }
    return false;
}

PropertyItem* GlissandoPlaybackModel::styleType() const
{
    return m_styleType;
}
