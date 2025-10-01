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

#include "editharppedaldiagram.h"

#include <vector>

#include "../dom/harppedaldiagram.h"
#include "../dom/part.h"
#include "../dom/score.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   ChangeHarpPedalState
//---------------------------------------------------------

void ChangeHarpPedalState::flip(EditData*)
{
    std::array<PedalPosition, HARP_STRING_NO> f_state = diagram->getPedalState();
    if (f_state == pedalState) {
        return;
    }

    diagram->setPedalState(pedalState);
    pedalState = f_state;

    diagram->triggerLayout();
}

std::vector<EngravingObject*> ChangeHarpPedalState::objectItems() const
{
    Part* part = diagram->part();
    std::vector<EngravingObject*> objs{ diagram };
    if (!part) {
        return objs;
    }

    HarpPedalDiagram* nextDiagram = part->nextHarpDiagram(diagram->tick());
    if (nextDiagram) {
        objs.push_back(nextDiagram);
    } else {
        objs.push_back(diagram->score()->lastElement());
    }
    return objs;
}

//---------------------------------------------------------
//   ChangeSingleHarpPedal
//---------------------------------------------------------

void ChangeSingleHarpPedal::flip(EditData*)
{
    HarpStringType f_type = type;
    PedalPosition f_pos = diagram->getPedalState()[type];
    if (f_pos == pos) {
        return;
    }

    diagram->setPedal(type, pos);
    type = f_type;
    pos = f_pos;

    diagram->triggerLayout();
}

std::vector<EngravingObject*> ChangeSingleHarpPedal::objectItems() const
{
    Part* part = diagram->part();
    std::vector<EngravingObject*> objs{ diagram };
    if (!part) {
        return objs;
    }

    HarpPedalDiagram* nextDiagram = part->nextHarpDiagram(diagram->tick());
    if (nextDiagram) {
        objs.push_back(nextDiagram);
    } else {
        objs.push_back(diagram->score()->lastElement());
    }
    return objs;
}
