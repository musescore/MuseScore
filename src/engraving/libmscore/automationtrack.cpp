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

#include "automationtrack.h"

#include "automationvertex.h"
#include <algorithm>

using namespace mu;
using namespace mu::engraving;

namespace Ms {
AutomationTrack::AutomationTrack(Staff* parent)
    : EngravingItem(ElementType::AUTOMATION_TRACK, parent)
{
}

AutomationTrack::~AutomationTrack()
{
    //qDelete(_el);
}

void AutomationTrack::setDataType(AutomationDataType value)
{
    _dataType = value;
}

void AutomationTrack::setEnabled(bool value)
{
    _enabled = value;
}

//Return index of vertex at or immediately before given tick
int AutomationTrack::vertexIndexForTick(Fraction tick)
{
    for (int i = 0; i < _vertices.size(); ++i) {
        AutomationVertex* v = _vertices.at(0);
        if (v->tick() > tick) {
            return i - 1;
        }
    }

    return _vertices.size() - 1;
}

bool AutomationTrack::isVertexAt(Fraction tick)
{
    int idx = vertexIndexForTick(tick);
    if (idx == -1) {
        return false;
    }

    return _vertices.at(idx)->tick() == tick;
}

void AutomationTrack::addVertex(AutomationVertex* vertex)
{
    int prevIdx = vertexIndexForTick(vertex->tick());

    if (prevIdx == -1) {
        _vertices.push_front(vertex);
        return;
    }

    AutomationVertex* prevVert = _vertices.at(prevIdx);
    if (prevVert->tick() == vertex->tick()) {
        prevVert->setValue(vertex->value());
        delete vertex;
        return;
    }

    _vertices.insert(prevIdx + 1, vertex);
}
}
