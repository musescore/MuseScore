/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include "organpedalmarkpopupmodel.h"
#include "engraving/types/symnames.h"

using namespace mu::notation;
using namespace mu::engraving;

OrganPedalMarkPopupModel::OrganPedalMarkPopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_ORGAN_PEDAL_MARK, parent)
{
}

void OrganPedalMarkPopupModel::init()
{
    AbstractElementPopupModel::init();
}

void OrganPedalMarkPopupModel::updatePedalMark(QString pedalMarkName)
{
    QString pedalMark = "<sym>" + pedalMarkName + "</sym>";

    OrganPedalMark* pm = m_item && m_item->isOrganPedalMark() ? toOrganPedalMark(m_item) : nullptr;

    if (!pm || pm->xmlText() == pedalMark) {
        return;
    }

    pm->setSymId(SymNames::symIdByName(pedalMarkName));

    beginCommand();
    pm->undoChangeProperty(Pid::TEXT, pedalMark);
    endCommand();
    updateNotation();
}

bool OrganPedalMarkPopupModel::isAbove()
{
    PropertyValue placement = toOrganPedalMark(m_item)->getProperty(Pid::PLACEMENT);

    return placement == PlacementV::ABOVE ? true : false;
}
