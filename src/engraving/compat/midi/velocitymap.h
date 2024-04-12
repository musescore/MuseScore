/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_VELOCITYMAP_H
#define MU_ENGRAVING_VELOCITYMAP_H

#include <map>

#include "global/allocator.h"

#include "../../types/types.h"

/**
 \file
 Definition of class VelocityMap.
*/

namespace mu::engraving {
//---------------------------------------------------------
///   VelocityEvent
///   item in VelocityMap
//---------------------------------------------------------

enum class VelocityEventType : char {
    DYNAMIC, HAIRPIN, INVALID
};

class VelocityEvent
{
public:
    VelocityEvent() {}
    VelocityEvent(int val)
        : m_value(val), m_type(VelocityEventType::DYNAMIC) {}
    VelocityEvent(Fraction s, Fraction e, int diff, ChangeMethod m, ChangeDirection d)
        : m_value(diff), m_type(VelocityEventType::HAIRPIN), m_length(e - s), m_method(m), m_direction(d) {}

    bool operator==(const VelocityEvent& event) const;
    bool operator!=(const VelocityEvent& event) const { return !(operator==(event)); }

private:
    friend class VelocityMap;

    int m_value = 0;
    VelocityEventType m_type = VelocityEventType::INVALID;
    Fraction m_length;
    ChangeMethod m_method = ChangeMethod::NORMAL;
    ChangeDirection m_direction = ChangeDirection::INCREASING;
    int m_cachedStartVal = -1;
    int m_cachedEndVal = -1;
};

//---------------------------------------------------------
//  VelocityMap
///  List of changes in a value.
//---------------------------------------------------------

typedef std::vector<std::pair<Fraction, Fraction> > EndPointsVector;

class VelocityMap : public std::multimap<Fraction, VelocityEvent>
{
    OBJECT_ALLOCATOR(engraving, VelocityMap)
public:
    VelocityMap() {}
    int val(Fraction tick) const;
    std::vector<std::pair<Fraction, Fraction> > changesInRange(Fraction stick, Fraction etick) const;

    void addDynamic(Fraction tick, int value);
    void addHairpin(Fraction stick, Fraction etick, int change, ChangeMethod method, ChangeDirection direction);
    void setup();

    static int interpolate(Fraction& eventTick, VelocityEvent& event, Fraction& tick);

private:

    struct ChangeMethodItem {
        ChangeMethod method;
        const char* name;
    };

    static bool compareHairpinEvents(VelocityEvent& a, VelocityEvent& b) { return a.m_length > b.m_length; }

    void sortHairpins();
    void resolveHairpinCollisions();
    void resolveDynamicInsideHairpinCollisions();
    void adjustCollidingHairpinsLength(std::vector<bool>& startsInHairpin, EndPointsVector& endPoints);
    bool dynamicExistsOnTick(Fraction tick) const;
    VelocityEvent dynamicEventForTick(Fraction tick) const;
    void addMissingDynamicsAfterHairpins();
    void fillHairpinsCache();

    static constexpr int DEFAULT_VALUE = 80;
    static constexpr int MIN_VALUE = 1;
    static constexpr int MAX_VALUE = 127;
    static constexpr int STEP = 16;
};
} // namespace mu::engraving
#endif
