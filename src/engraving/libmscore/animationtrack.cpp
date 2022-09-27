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

#include "animationtrack.h"

#include "animationkey.h"
#include <algorithm>

using namespace mu;
using namespace mu::engraving;

namespace Ms {
AnimationTrack::AnimationTrack(EngravingItem* parent)
    : EngravingItem(ElementType::ANIMATION_TRACK, parent)
{
}

AnimationTrack::~AnimationTrack()
{
    //qDelete(_el);
}

//void AnimationTrack::setDataType(AutomationDataType value)
//{
//    _dataType = value;
//}

//void AnimationTrack::setEnabled(bool value)
//{
//    _enabled = value;
//}

void AnimationTrack::setPropertyName(QString value)
{
    _propertyName = value;
}

//Return index of vertex at or immediately before given tick
int AnimationTrack::keyIndexForTick(Fraction tick)
{
    for (int i = 0; i < _keys.size(); ++i) {
        AnimationKey* v = _keys.at(0);
        if (v->tick() > tick) {
            return i - 1;
        }
    }

    return _keys.size() - 1;
}

bool AnimationTrack::isKeyAt(Fraction tick)
{
    int idx = keyIndexForTick(tick);
    if (idx == -1) {
        return false;
    }

    return _keys.at(idx)->tick() == tick;
}

void AnimationTrack::addKey(AnimationKey* vertex)
{
    int prevIdx = keyIndexForTick(vertex->tick());

    if (prevIdx == -1) {
        _keys.push_front(vertex);
        return;
    }

    AnimationKey* prevVert = _keys.at(prevIdx);
    if (prevVert->tick() == vertex->tick()) {
        prevVert->setValue(vertex->value());
        delete vertex;
        return;
    }

    _keys.insert(prevIdx + 1, vertex);
}
}
