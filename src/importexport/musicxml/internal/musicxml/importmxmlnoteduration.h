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

#ifndef __IMPORTMXMLNOTEDURATION_H__
#define __IMPORTMXMLNOTEDURATION_H__

#include "engraving/dom/durationtype.h"
#include "engraving/types/fraction.h"
#include "importmxmlpass1.h"

namespace mu::engraving {
class MxmlLogger;

//---------------------------------------------------------
//   mxmlNoteDuration
//---------------------------------------------------------

/**
 Parse the note time related part of the /score-partwise/part/measure/note node.
 */

class MxmlNoteDuration
{
public:
    MxmlNoteDuration(int divs, MxmlLogger* logger, MusicXMLParserPass1* pass1)
        : m_divs(divs), m_pass1(pass1), m_logger(logger) { /* nothing so far */ }
    String checkTiming(const String& type, const bool rest, const bool grace);
    Fraction duration() const { return m_dura; } // duration to use
    Fraction calculatedDuration() const { return m_calcDura; }   // value calculated from note type etcetera
    Fraction specifiedDuration() const { return m_specDura; }    // value read from the duration element
    int dots() const { return m_dots; }
    TDuration normalType() const { return m_normalType; }
    bool readProperties(XmlStreamReader& e);
    Fraction timeMod() const { return m_timeMod; }

private:
    void duration(XmlStreamReader& e);
    void timeModification(XmlStreamReader& e);
    const int m_divs;                                  // the current divisions value
    int m_dots = 0;
    Fraction m_calcDura;
    Fraction m_specDura;
    Fraction m_dura;
    TDuration m_normalType;
    Fraction m_timeMod { 1, 1 };                       // default to no time modification
    MusicXMLParserPass1* m_pass1 = nullptr;
    MxmlLogger* m_logger = nullptr;                              ///< Error logger
};
} // namespace Ms

#endif
