/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "modelwithstavecenteringoptions.h"

using namespace mu::propertiespanel;
using namespace mu::engraving;

ModelWithStaveCenteringOptions::ModelWithStaveCenteringOptions(QObject* parent,
                                                               const muse::modularity::ContextPtr& iocCtx,
                                                               IElementRepositoryService* repository,
                                                               ElementType elementType)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository, elementType)
{
}

void ModelWithStaveCenteringOptions::createProperties()
{
    m_centerBetweenStaves = buildPropertyItem(Pid::CENTER_BETWEEN_STAVES);
    updateStaveCenteringFlags();
}

void ModelWithStaveCenteringOptions::loadProperties()
{
    loadPropertyItem(m_centerBetweenStaves);
    updateStaveCenteringFlags();
}

void ModelWithStaveCenteringOptions::onNotationChanged(const PropertyIdSet&, const StyleIdSet&)
{
    loadProperties();
}

bool ModelWithStaveCenteringOptions::centeringSideIsRelevant(const EngravingItem* item, bool above) const
{
    return item->placeAbove() == above;
}

void ModelWithStaveCenteringOptions::updateStaveCenteringFlags()
{
    bool isApplicable = true;
    bool isAvailable = true;

    for (EngravingItem* item : m_elementList) {
        const bool otherStaffAbove = item->staffToCenterAgainst(true) != nullptr;
        const bool otherStaffBelow = item->staffToCenterAgainst(false) != nullptr;
        if (!otherStaffAbove && !otherStaffBelow) {
            isApplicable = false;
            isAvailable = false;
            break;
        }

        const bool otherStaffOnRelevantSide = (otherStaffAbove && centeringSideIsRelevant(item, true))
                                              || (otherStaffBelow && centeringSideIsRelevant(item, false));
        if (!otherStaffOnRelevantSide) {
            isAvailable = false;
        }
    }

    setIsStaveCenteringApplicable(isApplicable);
    setIsStaveCenteringAvailable(isAvailable);
}

PropertyItem* ModelWithStaveCenteringOptions::centerBetweenStaves() const
{
    return m_centerBetweenStaves;
}

bool ModelWithStaveCenteringOptions::isStaveCenteringApplicable() const
{
    return m_isStaveCenteringApplicable;
}

bool ModelWithStaveCenteringOptions::isStaveCenteringAvailable() const
{
    return m_isStaveCenteringAvailable;
}

void ModelWithStaveCenteringOptions::setIsStaveCenteringApplicable(bool v)
{
    if (v == m_isStaveCenteringApplicable) {
        return;
    }

    m_isStaveCenteringApplicable = v;
    emit isStaveCenteringApplicableChanged(m_isStaveCenteringApplicable);
}

void ModelWithStaveCenteringOptions::setIsStaveCenteringAvailable(bool v)
{
    if (v == m_isStaveCenteringAvailable) {
        return;
    }

    m_isStaveCenteringAvailable = v;
    emit isStaveCenteringAvailableChanged(m_isStaveCenteringAvailable);
}
