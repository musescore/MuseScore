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
#include "glissandoplaybackmodel.h"

#include "translation.h"

#include "engraving/dom/glissando.h"

using namespace mu::inspector;
using namespace mu::engraving;

GlissandoPlaybackModel::GlissandoPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(muse::qtrc("inspector", "Glissando"));
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
    updateIsHarpGliss();
}

void GlissandoPlaybackModel::loadProperties()
{
    loadPropertyItem(m_styleType);
}

void GlissandoPlaybackModel::resetProperties()
{
    m_styleType->resetToDefault();
}

void GlissandoPlaybackModel::updateIsHarpGliss()
{
    bool isHarpGliss = false;
    for (const EngravingItem* element : m_elementList) {
        if (element->isGlissando()) {
            if (toGlissando(element)->isHarpGliss().value_or(false)) {
                isHarpGliss = true;
                break;
            }
        }
    }
    setIsHarpGliss(isHarpGliss);
}

bool GlissandoPlaybackModel::isHarpGliss() const
{
    return m_isHarpGliss;
}

void GlissandoPlaybackModel::setIsHarpGliss(bool isHarpGliss)
{
    if (isHarpGliss == m_isHarpGliss) {
        return;
    }

    m_isHarpGliss = isHarpGliss;
    emit isHarpGlissChanged(m_isHarpGliss);
}

PropertyItem* GlissandoPlaybackModel::styleType() const
{
    return m_styleType;
}
