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
#pragma once

#include "global/types/string.h"

namespace mu::engraving {
class StaffLabel
{
public:
    StaffLabel() = default;
    StaffLabel(const muse::String& longName, const muse::String& shortName)
        : m_longName(longName), m_shortName(shortName) {}

    bool operator==(const StaffLabel& i) const { return m_longName == i.m_longName && m_shortName == i.m_shortName; }

    const muse::String& longName() const { return m_longName; }
    const muse::String& shortName() const { return m_shortName; }
    void setLongName(const muse::String& s) { m_longName = s; }
    void setShortName(const muse::String& s) { m_shortName = s; }

    bool empty() const { return m_longName.empty() && m_shortName.empty(); }

private:
    muse::String m_longName;
    muse::String m_shortName;
};

class InstrumentLabel : public StaffLabel
{
public:
    bool operator==(const InstrumentLabel& i) const;

    int number() const { return m_number; }
    void setNumber(int v) { m_number = v; }

    const muse::String& transposition() const { return m_transposition; }
    void setTransposition(const muse::String& s) { m_transposition = s; }

    bool showNumberLong() const { return m_showNumberLong; }
    void setShowNumberLong(bool v) { m_showNumberLong = v; }

    bool showNumberShort() const { return m_showNumberShort; }
    void setShowNumberShort(bool v) { m_showNumberShort = v; }

    bool showTranspositionLong() const { return m_showTranspositionLong; }
    void setShowTranspositionLong(bool v) { m_showTranspositionLong = v; }

    bool showTranspositionShort() const { return m_showTranspositionShort; }
    void setShowTranspositionShort(bool v) { m_showTranspositionShort = v; }

    bool useCustomName() const { return m_useCustomName; }
    void setUseCustomName(bool v) { m_useCustomName = v; }

    muse::String customNameLong() const { return m_customNameLong; }
    void setCustomNameLong(const muse::String& v) { m_customNameLong = v; }

    muse::String customNameShort() const { return m_customNameShort; }
    void setCustomNameShort(const muse::String& v) { m_customNameShort = v; }

    bool allowGroupName() const { return m_allowGroupName; }
    void setAllowGroupName(bool v) { m_allowGroupName = v; }

    muse::String customNameLongGroup() const { return m_customNameLongGroup; }
    void setCustomNameLongGroup(const muse::String& v) { m_customNameLongGroup = v; }

    muse::String customNameShortGroup() const { return m_customNameShortGroup; }
    void setCustomNameShortGroup(const muse::String& v) { m_customNameShortGroup = v; }

    bool useCustomGroupName() const { return m_useCustomGroupName; }
    void setUseCustomGroupName(bool v) { m_useCustomGroupName = v; }

    muse::String customNameLongIndividual() const { return m_customNameLongIndividual; }
    void setCustomNameLongIndividual(const muse::String& v) { m_customNameLongIndividual = v; }

    muse::String customNameShortIndividual() const { return m_customNameShortIndividual; }
    void setCustomNameShortIndividual(const muse::String& v) { m_customNameShortIndividual = v; }

    bool useCustomIndividualName() const { return m_useCustomIndividualName; }
    void setUseCustomIndividualName(bool v) { m_useCustomIndividualName = v; }

    bool empty() const;

private:
    int m_number = 0;
    bool m_showNumberLong = true;
    bool m_showNumberShort = true;

    muse::String m_transposition;
    bool m_showTranspositionLong = true;
    bool m_showTranspositionShort = true;

    bool m_useCustomName = false;
    muse::String m_customNameLong;
    muse::String m_customNameShort;

    bool m_allowGroupName = true;

    muse::String m_customNameLongGroup;
    muse::String m_customNameShortGroup;
    bool m_useCustomGroupName = false;

    muse::String m_customNameLongIndividual;
    muse::String m_customNameShortIndividual;
    bool m_useCustomIndividualName = false;
};

inline bool InstrumentLabel::operator==(const InstrumentLabel& i) const
{
    return StaffLabel::operator==(i)
           && m_number == i.m_number
           && m_transposition == i.m_transposition
           && m_showNumberLong == i.m_showNumberLong
           && m_showNumberShort == i.m_showNumberShort
           && m_useCustomName == i.m_useCustomName
           && m_customNameLong == i.m_customNameLong
           && m_customNameShort == i.m_customNameShort
           && m_showTranspositionLong == i.m_showTranspositionLong
           && m_showTranspositionShort == i.m_showTranspositionShort
           && m_allowGroupName == i.m_allowGroupName
           && m_customNameLongGroup == i.m_customNameLongGroup
           && m_customNameShortGroup == i.m_customNameShortGroup
           && m_useCustomGroupName == i.m_useCustomGroupName
           && m_customNameLongIndividual == i.m_customNameLongIndividual
           && m_customNameShortIndividual == i.m_customNameShortIndividual
           && m_useCustomIndividualName == i.m_useCustomIndividualName;
}

inline bool InstrumentLabel::empty() const
{
    static const InstrumentLabel EMPTY_LABEL = InstrumentLabel();

    return *this == EMPTY_LABEL;
}
}
