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

#ifndef __AUTOMATIONTRACK_H__
#define __AUTOMATIONTRACK_H__

#include "engravingitem.h"
#include "mscore.h"

namespace Ms {
class Staff;
class AutomationVertex;

enum class AutomationDataType : char {
    PAN,
    EXPRESSION,

    INVALID
};

class AutomationTrack : public EngravingItem
{
    AutomationDataType _dataType = AutomationDataType::INVALID;
    bool _enabled;

    QList<AutomationVertex*> _vertices;

public:
    AutomationTrack(Staff* parent);

    virtual ~AutomationTrack();

    AutomationTrack& operator=(const BarLine&) = delete;

    AutomationDataType dataType() { return _dataType; }
    void setDataType(AutomationDataType value);
    bool enabled() { return _enabled; }
    void setEnabled(bool value);

    const QList<AutomationVertex*>& vertices() const { return _vertices; }
    QList<AutomationVertex*>& vertices() { return _vertices; }

    void addVertex(AutomationVertex* vertex);
};
}

#endif // __AUTOMATIONTRACK_H__
