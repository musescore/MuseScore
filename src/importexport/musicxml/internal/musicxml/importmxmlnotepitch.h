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

#ifndef __IMPORTMXMLNOTEPITCH_H__
#define __IMPORTMXMLNOTEPITCH_H__

#include "global/serialization/xmlstreamreader.h"
#include "engraving/dom/accidental.h"

namespace mu::engraving {
class MxmlLogger;
class Score;

//---------------------------------------------------------
//   mxmlNotePitch
//---------------------------------------------------------

/**
 Parse the note pitch related part of the /score-partwise/part/measure/note node.
 */

class MxmlNotePitch
{
public:
    MxmlNotePitch(MxmlLogger* logger)
        : m_logger(logger) { /* nothing so far */ }
    void pitch(XmlStreamReader& e);
    bool readProperties(XmlStreamReader& e, Score* score);
    Accidental* acc() const { return m_acc; }
    AccidentalType accType() const { return m_accType; }
    int alter() const { return m_alter; }
    int displayOctave() const { return m_displayOctave; }
    int displayStep() const { return m_displayStep; }
    void displayStepOctave(XmlStreamReader& e);
    int octave() const { return m_octave; }
    int step() const { return m_step; }
    bool unpitched() const { return m_unpitched; }

private:
    Accidental* m_acc = nullptr;                             // created based on accidental element
    AccidentalType m_accType = AccidentalType::NONE;         // set by pitch() based on alter value (can be microtonal)
    int m_alter = 0;
    int m_displayStep = -1;                                  // invalid
    int m_displayOctave = -1;                                // invalid
    int m_octave = -1;
    int m_step = 0;
    bool m_unpitched = false;
    MxmlLogger* m_logger = nullptr;                          // Error logger
};
} // namespace Ms

#endif
