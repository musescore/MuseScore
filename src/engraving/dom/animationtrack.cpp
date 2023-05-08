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

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
AnimationTrack::AnimationTrack(EngravingItem* parent)
{
}

AnimationTrack::~AnimationTrack()
{
}

void AnimationTrack::setPropertyName(std::string value)
{
    _propertyName = value;
}

void AnimationTrack::addKey(Fraction tick, float value)
{
    int index = keyIndexForTick(tick);
    if (index == -1) {
        AnimationKey* key = new AnimationKey(this);
        key->setValue(value);
        key->setTick(tick);
        _keys.emplace(_keys.begin(), key);
        return;
    }

    AnimationKey* keyIndex = _keys[index];
    if (keyIndex->tick() == tick) {
        keyIndex->setValue(value);
    }

    AnimationKey* key = new AnimationKey(this);
    key->setValue(value);
    key->setTick(tick);
    _keys.emplace(_keys.begin() + index + 1, key);
}

void AnimationTrack::removeKey(Fraction tick)
{
    int index = keyIndexForTick(tick);
    if (index == -1) {
        return;
    }

    AnimationKey* key = _keys[index];
    if (key->tick() == tick) {
        _keys.erase(_keys.begin() + index);
        delete key;
    }
}
}
