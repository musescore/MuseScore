/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "staffvisibilitypopupmodel.h"

#include "containers.h"

#include "engraving/dom/staffvisibilityindicator.h"
#include "engraving/dom/system.h"

using namespace mu::notation;

StaffVisibilityPopupModel::StaffVisibilityPopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_STAFF_VISIBILITY, parent)
    , m_emptyStavesVisibilityModel(std::make_unique<EmptyStavesVisibilityModel>(this))
{
}

void StaffVisibilityPopupModel::classBegin()
{
    init();
}

void StaffVisibilityPopupModel::init()
{
    AbstractElementPopupModel::init();

    System* system = m_item && m_item->isStaffVisibilityIndicator()
                     ? toStaffVisibilityIndicator(m_item)->system()
                     : nullptr;
    if (!system) {
        return;
    }

    m_emptyStavesVisibilityModel->load(currentNotation(), system);

    m_systemIndex = muse::indexOf(system->score()->systems(), system);
    emit systemIndexChanged();
}
