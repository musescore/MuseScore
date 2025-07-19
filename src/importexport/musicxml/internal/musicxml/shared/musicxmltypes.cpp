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

#include "musicxmltypes.h"
#include "dom/interval.h"
#include "log.h"

using namespace mu::engraving;
using namespace mu::iex::musicxml;

String MusicXmlInstrument::toString() const
{
    return String(u"chan %1 prog %2 vol %3 pan %4 unpitched %5 name '%6' sound '%7' head %8 line %9 stemDir %10")
           .arg(midiChannel)
           .arg(midiProgram)
           .arg(midiVolume)
           .arg(midiPan)
           .arg(unpitched)
           .arg(name, sound)
           .arg(int(notehead))
           .arg(line)
           .arg(int(stemDirection));
}

//---------------------------------------------------------
//   interval
//---------------------------------------------------------

Interval MusicXmlIntervalList::interval(const Fraction f) const
{
    if (empty()) {
        return {};
    }
    auto i = upper_bound(f);
    if (i == begin()) {
        return {};
    }
    --i;
    return i->second;
}

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

const String MusicXmlInstrList::instrument(const Fraction f) const
{
    if (empty()) {
        return String();
    }
    auto i = upper_bound(f);
    if (i == begin()) {
        return String();
    }
    --i;
    return i->second;
}

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void MusicXmlInstrList::setInstrument(const String instr, const Fraction f)
{
    // TODO determine how to handle multiple instrument changes at the same time
    // current implementation keeps the first one
    if (!insert({ f, instr }).second) {
        LOGD() << "element already exists, instr: " << instr
               << ", tick: " << f.toString() << "(" << f.ticks() << ")";
    }
    //(*this)[f] = instr;
}

//---------------------------------------------------------
//   LyricNumberHandler
//   collect lyric numbering information and determine order
//
//   MusicXML lyrics may contain name and number attributes,
//   plus position information (typically default-y).
//   Name and number are simply tokens with no specified usage.
//   Default-y cannot easily be used to determine the lyrics
//   line, as it tends to differ per system depending on the
//   actual notes present.
//
//   Simply collecting all possible lyric number attributes
//   within a MusicXML part and assigning lyrics position
//   based on alphabetically sorting works well for all
//   common MusicXML files.
//---------------------------------------------------------

//---------------------------------------------------------
//   addNumber
//---------------------------------------------------------

void LyricNumberHandler::addNumber(const String& number)
{
    if (m_numberToNo.find(number) == m_numberToNo.end()) {
        m_numberToNo[number] = -1;           // unassigned
    }
}

//---------------------------------------------------------
//   toString
//---------------------------------------------------------

String LyricNumberHandler::toString() const
{
    String res;
    for (const auto& p : m_numberToNo) {
        if (!res.isEmpty()) {
            res += u" ";
        }
        res += String(u"%1:%2").arg(p.first).arg(p.second);
    }
    return res;
}

//---------------------------------------------------------
//   getLyricNo
//---------------------------------------------------------

int LyricNumberHandler::getLyricNo(const String& number) const
{
    const auto it = m_numberToNo.find(number);
    return it == m_numberToNo.end() ? 0 : it->second;
}

//---------------------------------------------------------
//   determineLyricNos
//---------------------------------------------------------

void LyricNumberHandler::determineLyricNos()
{
    int i = 0;
    for (auto& p : m_numberToNo) {
        p.second = i;
        ++i;
    }
}
