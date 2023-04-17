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

#ifndef __POINT_H__
#define __POINT_H__

#include <climits>

#include "types/propertyvalue.h"

namespace mu::engraving {
class EngravingItem;
class XmlReader;
class XmlWriter;

enum class Pid;

//---------------------------------------------------------
//   Location
//---------------------------------------------------------

class Location
{
    int _staff;
    int _voice;
    int _measure;
    Fraction _frac;
    int _graceIndex;
    int _note;
    bool _rel;

    static int track(const EngravingItem* e);
    static int measure(const EngravingItem* e);
    static int graceIndex(const EngravingItem* e);
    static int note(const EngravingItem* e);

public:
    constexpr Location(int staff, int voice, int measure, Fraction frac, int graceIndex, int note, bool rel)
        : _staff(staff), _voice(voice), _measure(measure), _frac(frac), _graceIndex(graceIndex), _note(note), _rel(rel)
    {
    }

    static constexpr Location absolute()
    {
        return Location(INT_MIN, INT_MIN, INT_MIN, Fraction(INT_MIN, 1), INT_MIN, INT_MIN, false);
    }

    static constexpr Location relative() { return Location(0, 0, 0, Fraction(0, 1), INT_MIN, 0, true); }

    void toAbsolute(const Location& ref);
    void toRelative(const Location& ref);

    void write(XmlWriter& xml) const;

    bool isAbsolute() const { return !_rel; }
    bool isRelative() const { return _rel; }

    int staff() const { return _staff; }
    void setStaff(int staff) { _staff = staff; }
    int voice() const { return _voice; }
    void setVoice(int voice) { _voice = voice; }
    int track() const;
    void setTrack(int track);
    int measure() const { return _measure; }
    void setMeasure(int measure) { _measure = measure; }
    Fraction frac() const { return _frac; }
    void setFrac(Fraction frac) { _frac = frac; }
    int graceIndex() const { return _graceIndex; }
    void setGraceIndex(int index) { _graceIndex = index; }
    int note() const { return _note; }
    void setNote(int note) { _note = note; }

    void fillForElement(const EngravingItem* e, bool absfrac = true);
    void fillPositionForElement(const EngravingItem* e, bool absfrac = true);
    static Location forElement(const EngravingItem* e, bool absfrac = true);
    static Location positionForElement(const EngravingItem* e, bool absfrac = true);
    static PropertyValue getLocationProperty(Pid pid, const EngravingItem* start, const EngravingItem* end);

    bool operator==(const Location& other) const;
    bool operator!=(const Location& other) const { return !(*this == other); }
};
} // namespace mu::engraving
#endif
