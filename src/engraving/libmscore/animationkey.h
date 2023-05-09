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

#ifndef __ANIMATIONKEY_H__
#define __ANIMATIONKEY_H__

#include "engravingitem.h"
#include "mscore.h"

namespace mu::engraving {
class AnimationTrack;

class AnimationKey
{
    float _value = 0;
    Fraction _tick;
    AnimationTrack* _parent;

public:
    AnimationKey(AnimationTrack* parent);
    ~AnimationKey();

    float value() { return _value; }
    void setValue(float value);
    Fraction tick() { return _tick; }
    void setTick(Fraction tick);
};
}
#endif // __ANIMATIONKEY_H__
