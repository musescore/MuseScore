/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "musicxmlpart.h"
#include "dom/mscore.h"
#include "dom/interval.h"
#include "log.h"

using namespace mu::engraving;
using namespace mu::iex::musicxml;

static const std::vector<String> vocalInstrumentNames = {
    u"Voice",
    u"Soprano",
    u"Mezzo-Soprano",
    u"Alto",
    u"Tenor",
    u"Baritone",
    u"Bass",
    u"Women",
    u"Men"
};

static const std::vector<String> percussionInstrumentNames = {
    u"Percussion",
    u"Timpani",
    u"Glockenspiel",
    u"Xylophone",
    u"Vibraphone",
    u"Marimba",
    u"Bell",
    u"Drum"
    u"Cymbal",
    u"Triangle",
    u"Claves",
    u"Wood Blocks",
    u"Tambourine",
    u"Finger Snap",
    u"Hand Clap",
    u"Slap",
    u"Stamp"
};

int MusicXmlOctaveShiftList::octaveShift(const Fraction f) const
{
    if (empty()) {
        return 0;
    }
    auto i = upper_bound(f);
    if (i == begin()) {
        return 0;
    }
    --i;
    return i->second;
}

void MusicXmlOctaveShiftList::addOctaveShift(const int shift, const Fraction f)
{
    IF_ASSERT_FAILED(Fraction(0, 1) <= f) {
        return;
    }

    //LOGD("addOctaveShift(shift %d f %s)", shift, muPrintable(f.print()));
    auto i = find(f);
    if (i == end()) {
        //LOGD("addOctaveShift: not found, inserting");
        insert({ f, shift });
    } else {
        //LOGD("addOctaveShift: found %d, adding", (*this)[f]);
        (*this)[f] += shift;
        //LOGD("addOctaveShift: res %d", (*this)[f]);
    }
}

void MusicXmlOctaveShiftList::calcOctaveShiftShifts()
{
    /*
    for (auto i = cbegin(); i != cend(); ++i)
          LOGD(" [%s : %d]", muPrintable((*i).first.print()), (*i).second);
     */

    // to each MusicXmlOctaveShiftList entry, add the sum of all previous ones
    int currentShift = 0;
    for (auto i = begin(); i != end(); ++i) {
        currentShift += i->second;
        i->second = currentShift;
    }

    /*
    for (auto i = cbegin(); i != cend(); ++i)
          LOGD(" [%s : %d]", muPrintable((*i).first.print()), (*i).second);
     */
}

MusicXmlPart::MusicXmlPart(String id, String name)
    : m_id(id), m_name(name)
{
    m_octaveShifts.resize(MAX_STAVES);
}

void MusicXmlPart::addMeasureNumberAndDuration(String measureNumber, Fraction measureDuration)
{
    m_measureNumbers.push_back(measureNumber);
    m_measureDurations.push_back(measureDuration);
}

void MusicXmlPart::setMaxStaff(const int staff)
{
    if (staff > m_maxStaff) {
        m_maxStaff = staff;
    }
}

Fraction MusicXmlPart::measureDuration(size_t i) const
{
    if (i < m_measureDurations.size()) {
        return m_measureDurations.at(i);
    }
    return Fraction(0, 0);   // return invalid fraction
}

String MusicXmlPart::toString() const
{
    String res = String(u"part id '%1' name '%2' print %3 abbr '%4' print %5 maxStaff %6\n")
                 .arg(m_id, m_name).arg(m_printName).arg(m_abbr).arg(m_printAbbr, m_maxStaff);

    for (const auto& p : voicelist) {
        res += String(u"voice %1 map staff data %2\n")
               .arg(String::number(p.first + 1), p.second.toString());
    }

    for (size_t i = 0; i < m_measureNumbers.size(); ++i) {
        if (i > 0) {
            res += u"\n";
        }
        res += String(u"measure %1 duration %2 (%3)")
               .arg(m_measureNumbers.at(i), m_measureDurations.at(i).toString()).arg(m_measureDurations.at(i).ticks());
    }

    return res;
}

Interval MusicXmlPart::interval(const Fraction f) const
{
    return _intervals.interval(f);
}

int MusicXmlPart::octaveShift(const staff_idx_t staff, const Fraction f) const
{
    if (MAX_STAVES <= staff) {
        return 0;
    }
    if (f < Fraction(0, 1)) {
        return 0;
    }
    return m_octaveShifts[staff].octaveShift(f);
}

void MusicXmlPart::addOctaveShift(const staff_idx_t staff, const int shift, const Fraction f)
{
    if (MAX_STAVES <= staff) {
        return;
    }
    if (f < Fraction(0, 1)) {
        return;
    }
    m_octaveShifts[staff].addOctaveShift(shift, f);
}

void MusicXmlPart::calcOctaveShifts()
{
    for (staff_idx_t i = 0; i < MAX_STAVES; ++i) {
        m_octaveShifts[i].calcOctaveShiftShifts();
    }
}

//---------------------------------------------------------
//   staffNumberToIndex
//---------------------------------------------------------

/**
 This handles the mapping from MusicXML staff number to the index
 in a Part's Staff list.
 In most cases, this is a simple decrement from the 1-based staff number
 to the 0-based index.
 However, in some parts some MusicXML staves are discarded, and a mapping
 must be stored from MusicXML staff number to index. When this mapping is
 defined (i.e. size() != 0), it is used. See MusicXmlParserPass1::attributes()
 for more information.
 */

int MusicXmlPart::staffNumberToIndex(const int staffNumber) const
{
    if (m_staffNumberToIndex.size() == 0) {
        return staffNumber - 1;
    } else if (muse::contains(m_staffNumberToIndex, staffNumber)) {
        return m_staffNumberToIndex.at(staffNumber);
    } else {
        return -1;
    }
}

bool MusicXmlPart::isVocalStaff() const
{
    return std::find(vocalInstrumentNames.begin(), vocalInstrumentNames.end(), m_name) != vocalInstrumentNames.end();
}

bool MusicXmlPart::isPercussionStaff() const
{
    for (const String& name : percussionInstrumentNames) {
        if (m_name.contains(name)) {
            return true;
        }
    }
    return false;
}
