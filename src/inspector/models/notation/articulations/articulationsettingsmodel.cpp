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
#include "articulationsettingsmodel.h"

#include "engraving/dom/articulation.h"
#include "engraving/dom/tapping.h"

#include "log.h"
#include "translation.h"

using namespace mu::inspector;
using namespace mu::engraving;

ArticulationSettingsModel::ArticulationSettingsModel(QObject* parent, IElementRepositoryService* repository, InspectorModelType type)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(type);
    setTitle(type == InspectorModelType::TYPE_ARTICULATION ? muse::qtrc("inspector", "Articulation") : muse::qtrc("inspector", "Tapping"));
    setIcon(muse::ui::IconCode::Code::ARTICULATION);
    createProperties();
}

void ArticulationSettingsModel::createProperties()
{
    m_placement = buildPropertyItem(mu::engraving::Pid::ARTICULATION_ANCHOR);
}

void ArticulationSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::ARTICULATION);

    QList<mu::engraving::EngravingItem*> tappings = m_repository->findElementsByType(mu::engraving::ElementType::TAPPING);
    for (mu::engraving::EngravingItem* tapping : tappings) {
        m_elementList.push_back(tapping);
    }

    QList<mu::engraving::EngravingItem*> halfSlurSegs
        = m_repository->findElementsByType(mu::engraving::ElementType::TAPPING_HALF_SLUR_SEGMENT);
    for (mu::engraving::EngravingItem* halfSlurSeg : halfSlurSegs) {
        TappingHalfSlur* halfSlur = toTappingHalfSlur(toTappingHalfSlurSegment(halfSlurSeg)->spanner());
        Tapping* tapping = toTapping(halfSlur->parentItem());
        m_elementList.push_back(tapping);
    }
}

void ArticulationSettingsModel::loadProperties()
{
    loadPropertyItem(m_placement);
    updateIsPlacementAvailable();
}

void ArticulationSettingsModel::resetProperties()
{
    m_placement->resetToDefault();
}

void ArticulationSettingsModel::updateIsPlacementAvailable()
{
    bool available = false;
    for (EngravingItem* item : m_elementList) {
        if (!item->isTapping()) {
            available = true;
            break;
        }

        Tapping* tapping = toTapping(item);
        bool hasSymbol = tapping->hand() == TappingHand::RIGHT || tapping->lhShowItems() != LHTappingShowItems::HALF_SLUR;
        if (hasSymbol) {
            available = true;
            break;
        }
    }

    if (available != m_isPlacementAvailable) {
        m_isPlacementAvailable = available;
        emit isPlacementAvailableChanged(m_isPlacementAvailable);
    }
}

PropertyItem* ArticulationSettingsModel::placement() const
{
    return m_placement;
}

bool ArticulationSettingsModel::isPlacementAvailable() const
{
    return m_isPlacementAvailable;
}
