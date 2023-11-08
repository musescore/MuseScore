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

#ifndef MU_ENGRAVING_CHANGEMAP_H
#define MU_ENGRAVING_CHANGEMAP_H

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
public:
    ChangeEvent() {}
    ChangeEvent(int val)
        : m_value(val), m_type(ChangeEventType::FIX) {}
    ChangeEvent(Fraction s, Fraction e, int diff, ChangeMethod m, ChangeDirection d)
        : m_value(diff), m_type(ChangeEventType::RAMP), m_length(e - s), m_method(m), m_direction(d) {}

    bool operator==(const ChangeEvent& event) const;
    bool operator!=(const ChangeEvent& event) const { return !(operator==(event)); }

private:
    friend class ChangeMap;

    int m_value = 0;
    ChangeEventType m_type = ChangeEventType::INVALID;
    Fraction m_length;
    ChangeMethod m_method = ChangeMethod::NORMAL;
    ChangeDirection m_direction = ChangeDirection::INCREASING;
    int m_cachedStartVal = -1;
    int m_cachedEndVal = -1;
};

//---------------------------------------------------------
//  ChangeMap
///  List of changes in a value.
//---------------------------------------------------------

typedef std::vector<std::pair<Fraction, Fraction> > EndPointsVector;

class ChangeMap : public std::multimap<Fraction, ChangeEvent>
{
    OBJECT_ALLOCATOR(engraving, ChangeMap)
public:
    ChangeMap() {}
    int val(Fraction tick);
    std::vector<std::pair<Fraction, Fraction> > changesInRange(Fraction stick, Fraction etick);

    void addFixed(Fraction tick, int value);
    void addRamp(Fraction stick, Fraction etick, int change, ChangeMethod method, ChangeDirection direction);
    void cleanup();

    static int interpolate(Fraction& eventTick, ChangeEvent& event, Fraction& tick);

private:

    struct ChangeMethodItem {
        ChangeMethod method;
        const char* name;
    };

    static bool compareRampEvents(ChangeEvent& a, ChangeEvent& b) { return a.m_length > b.m_length; }

    void sortRamps();
    void resolveRampsCollisions();
    void resolveFixInsideRampCollisions();
    void adjustCollidingRampsLength(std::vector<bool>& startsInRamp, EndPointsVector& endPoints);
    bool fixExistsOnTick(Fraction tick) const;
    ChangeEvent fixEventForTick(Fraction tick) const;
    void addMissingFixesAfterRamps();
    void fillRampsCache();

    bool m_cleanedUp = false;
    static constexpr int DEFAULT_VALUE = 80;
    static constexpr int MIN_VALUE = 1;
    static constexpr int MAX_VALUE = 127;
    static constexpr int STEP = 16;
};
} // namespace mu::engraving
#endif
