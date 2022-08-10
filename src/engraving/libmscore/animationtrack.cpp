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
#include "property.h"
#include <algorithm>

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

//Return index of vertex at or immediately before given tick
int AnimationTrack::keyIndexForTick(Fraction tick)
{
    for (int i = 0; i < _keys.size(); ++i) {
        AnimationKey* v = _keys.at(i);
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

float AnimationTrack::evaluate(Fraction tick)
{
    int index = keyIndexForTick(tick);
    if (index == -1) {
        Pid id = propertyId(_propertyName);
        double defaultValue = propertyDefaultValue(id);
        return defaultValue;
    }

    if (index == _keys.size() - 1) {
        AnimationKey* k0 = _keys[index];
        return k0->value();
    }

    AnimationKey* k0 = _keys[index];
    AnimationKey* k1 = _keys[index + 1];

    Fraction t0 = k0->tick();
    Fraction t1 = k1->tick();

    double t0d = t0.toDouble();
    double t1d = t1.toDouble();

    double tmd = tick.numerator() / (double)tick.denominator();
    double ratio = (tmd - t0d) / (t1d - t0d);
    return (k1->value() - k0->value()) * ratio + k0->value();
}
}
