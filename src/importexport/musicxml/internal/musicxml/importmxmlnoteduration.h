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

namespace mu::engraving {
class MxmlLogger;

//---------------------------------------------------------
//   mxmlNoteDuration
//---------------------------------------------------------

/**
 Parse the note time related part of the /score-partwise/part/measure/note node.
 */

class mxmlNoteDuration
{
public:
    mxmlNoteDuration(int divs, MxmlLogger* logger)
        : _divs(divs), _logger(logger) { /* nothing so far */ }
    QString checkTiming(const QString& type, const bool rest, const bool grace);
    Fraction duration() const { return _dura; } // duration to use
    Fraction calculatedDuration() const { return _calcDura; }   // value calculated from note type etcetera
    Fraction specifiedDuration() const { return _specDura; }    // value read from the duration element
    int dots() const { return _dots; }
    TDuration normalType() const { return _normalType; }
    bool readProperties(QXmlStreamReader& e);
    Fraction timeMod() const { return _timeMod; }

private:
    void duration(QXmlStreamReader& e);
    void timeModification(QXmlStreamReader& e);
    const int _divs;                                  // the current divisions value
    int _dots = 0;
    Fraction _calcDura;
    Fraction _specDura;
    Fraction _dura;
    TDuration _normalType;
    Fraction _timeMod { 1, 1 };                       // default to no time modification
    MxmlLogger* _logger;                              ///< Error logger
};
} // namespace Ms

#endif
