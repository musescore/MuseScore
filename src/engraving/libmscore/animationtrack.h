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

#ifndef __ANIMATIONTRACK_H__
#define __ANIMATIONTRACK_H__

#include "engravingitem.h"
#include "mscore.h"

namespace Ms {
class Staff;
class AnimationKey;


class AnimationTrack : public EngravingItem
{
//    AutomationDataType _dataType = AutomationDataType::INVALID;
    QString _propertyName;
//    bool _enabled = true;

    QList<AnimationKey*> _keys;

public:
    AnimationTrack(EngravingItem* parent);

    virtual ~AnimationTrack();

    AnimationTrack& operator=(const BarLine&) = delete;

//    AutomationDataType dataType() const { return _dataType; }
//    void setDataType(AutomationDataType value);
//    bool enabled() const { return _enabled; }
//    void setEnabled(bool value);
    QString propertyName() const { return _propertyName; }
    void setPropertyName(QString value);

    const QList<AnimationKey*>& vertices() const { return _keys; }
    QList<AnimationKey*>& vertices() { return _keys; }
    void addKey(AnimationKey* vertex);
    bool isKeyAt(Fraction tick);

    int keyIndexForTick(Fraction tick);
};
}

#endif // __ANIMATIONTRACK_H__
