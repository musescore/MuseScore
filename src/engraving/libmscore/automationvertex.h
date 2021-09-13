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

#ifndef __AUTOMATIONVERTEX_H__
#define __AUTOMATIONVERTEX_H__

#include "engravingitem.h"
#include "mscore.h"

namespace Ms {
class AutomationTrack;

class AutomationVertex : public EngravingItem
{
    float _value;
    int _ticks;  //Number of whole notes offset from start of score

public:
    AutomationVertex(AutomationTrack* parent);
    ~AutomationVertex();

    float value() { return _value; }
    void setValue(float value);

//    int ticks() { return _ticks; }
//    void setTicks(int value);
};
}
#endif // __AUTOMATIONVERTEX_H__
