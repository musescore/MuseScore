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

#ifndef __CHANGEMAP_H__
#define __CHANGEMAP_H__

#include <map>

#include "global/allocator.h"
#include "types/types.h"

/**
 \file
 Definition of class ChangeMap.
*/

namespace mu::engraving {
//---------------------------------------------------------
///   ChangeEvent
///   item in ChangeMap
//---------------------------------------------------------

enum class ChangeEventType : char {
    FIX, RAMP, INVALID
};

class ChangeEvent
{
    int value { 0 };
    ChangeEventType type { ChangeEventType::INVALID };
    Fraction length;
    ChangeMethod method { ChangeMethod::NORMAL };
    ChangeDirection direction { ChangeDirection::INCREASING };
    int cachedStartVal   { -1 };
    int cachedEndVal     { -1 };

public:
    ChangeEvent() {}
    ChangeEvent(int vel)
        : value(vel), type(ChangeEventType::FIX) {}
    ChangeEvent(Fraction s, Fraction e, int diff, ChangeMethod m, ChangeDirection d)
        : value(diff), type(ChangeEventType::RAMP), length(e - s), method(m), direction(d) {}

    bool operator==(const ChangeEvent& event) const;
    bool operator!=(const ChangeEvent& event) const { return !(operator==(event)); }

    friend class ChangeMap;
};

//---------------------------------------------------------
//  ChangeMap
///  List of changes in a value.
//---------------------------------------------------------

typedef std::vector<std::pair<Fraction, Fraction> > EndPointsVector;

class ChangeMap : public std::multimap<Fraction, ChangeEvent>
{
    OBJECT_ALLOCATOR(engraving, ChangeMap)

    bool cleanedUp    { false };
    static const int DEFAULT_VALUE  { 80 };

    struct ChangeMethodItem {
        ChangeMethod method;
        const char* name;
    };

    static bool compareRampEvents(ChangeEvent& a, ChangeEvent& b) { return a.length > b.length; }

    void cleanupStage0();
    void cleanupStage1();
    void cleanupStage2(std::vector<bool>& startsInRamp, EndPointsVector& endPoints);
    void cleanupStage3();

public:
    ChangeMap() {}
    int val(Fraction tick);
    std::vector<std::pair<Fraction, Fraction> > changesInRange(Fraction stick, Fraction etick);

    void addFixed(Fraction tick, int value);
    void addRamp(Fraction stick, Fraction etick, int change, ChangeMethod method, ChangeDirection direction);
    void cleanup();

    static int interpolate(Fraction& eventTick, ChangeEvent& event, Fraction& tick);
};
} // namespace mu::engraving
#endif
