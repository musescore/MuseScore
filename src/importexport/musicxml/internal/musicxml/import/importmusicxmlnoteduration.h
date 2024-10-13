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

#include "engraving/dom/durationtype.h"
#include "engraving/types/fraction.h"
#include "serialization/xmlstreamreader.h"

namespace mu::iex::musicxml {
class MusicXmlLogger;
class MusicXmlParserPass1;

//---------------------------------------------------------
//   MusicXmlNoteDuration
//---------------------------------------------------------

/**
 Parse the note time related part of the /score-partwise/part/measure/note node.
 */

class MusicXmlNoteDuration
{
public:
    MusicXmlNoteDuration(int divs, MusicXmlLogger* logger, MusicXmlParserPass1* pass1)
        : m_divs(divs), m_pass1(pass1), m_logger(logger) { /* nothing so far */ }
    muse::String checkTiming(const muse::String& type, const bool rest, const bool grace);
    engraving::Fraction duration() const { return m_dura; } // duration to use
    engraving::Fraction calculatedDuration() const { return m_calcDura; }   // value calculated from note type etcetera
    engraving::Fraction specifiedDuration() const { return m_specDura; }    // value read from the duration element
    int dots() const { return m_dots; }
    engraving::TDuration normalType() const { return m_normalType; }
    bool readProperties(muse::XmlStreamReader& e);
    engraving::Fraction timeMod() const { return m_timeMod; }

private:
    void duration(muse::XmlStreamReader& e);
    void timeModification(muse::XmlStreamReader& e);
    const int m_divs;                                  // the current divisions value
    int m_dots = 0;
    engraving::Fraction m_calcDura;
    engraving::Fraction m_specDura;
    engraving::Fraction m_dura;
    engraving::TDuration m_normalType;
    engraving::Fraction m_timeMod { 1, 1 };                       // default to no time modification
    MusicXmlParserPass1* m_pass1 = nullptr;
    MusicXmlLogger* m_logger = nullptr;                              ///< Error logger
};
} // namespace Ms
