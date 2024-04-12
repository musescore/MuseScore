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

#ifndef MU_ENGRAVING_POINT_H
#define MU_ENGRAVING_POINT_H

#include <climits>

#include "types/propertyvalue.h"

namespace mu::engraving {
class EngravingItem;

enum class Pid;

//---------------------------------------------------------
//   Location
//---------------------------------------------------------

class Location
{
public:
    constexpr Location(int staff, int voice, int measure, Fraction frac, int graceIndex, int note, bool rel)
        : m_staff(staff), m_voice(voice), m_measure(measure), m_frac(frac), m_graceIndex(graceIndex), m_note(note), m_rel(rel)
    {
    }

    static constexpr Location absolute()
    {
        return Location(INT_MIN, INT_MIN, INT_MIN, Fraction(INT_MIN, 1), INT_MIN, INT_MIN, false);
    }

    static constexpr Location relative() { return Location(0, 0, 0, Fraction(0, 1), INT_MIN, 0, true); }

    void toAbsolute(const Location& ref);
    void toRelative(const Location& ref);

    bool isAbsolute() const { return !m_rel; }
    bool isRelative() const { return m_rel; }

    int staff() const { return m_staff; }
    void setStaff(int staff) { m_staff = staff; }
    int voice() const { return m_voice; }
    void setVoice(int voice) { m_voice = voice; }
    int track() const;
    void setTrack(int track);
    int measure() const { return m_measure; }
    void setMeasure(int measure) { m_measure = measure; }
    Fraction frac() const { return m_frac; }
    void setFrac(Fraction frac) { m_frac = frac; }
    int graceIndex() const { return m_graceIndex; }
    void setGraceIndex(int index) { m_graceIndex = index; }
    int note() const { return m_note; }
    void setNote(int note) { m_note = note; }

    void fillForElement(const EngravingItem* e, bool absfrac = true);
    void fillPositionForElement(const EngravingItem* e, bool absfrac = true);
    static Location forElement(const EngravingItem* e, bool absfrac = true);
    static Location positionForElement(const EngravingItem* e, bool absfrac = true);
    static PropertyValue getLocationProperty(Pid pid, const EngravingItem* start, const EngravingItem* end);

    void setIsTimeTick(bool v) { m_isTimeTick = v; }
    bool isTimeTick() const { return m_isTimeTick; }

    bool operator==(const Location& other) const;
    bool operator!=(const Location& other) const { return !(*this == other); }

private:

    static int track(const EngravingItem* e);
    static int measure(const EngravingItem* e);
    static int graceIndex(const EngravingItem* e);
    static int note(const EngravingItem* e);

    int m_staff = 0;
    int m_voice = 0;
    int m_measure = 0;
    Fraction m_frac;
    int m_graceIndex = 0;
    int m_note = 0;
    bool m_rel = false;
    bool m_isTimeTick = false;
};
} // namespace mu::engraving
#endif
