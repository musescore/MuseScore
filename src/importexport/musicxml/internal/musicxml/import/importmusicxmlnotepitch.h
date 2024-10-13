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

#include "dom/accidental.h"
#include "serialization/xmlstreamreader.h"

namespace mu::iex::musicxml {
class MusicXmlLogger;

//---------------------------------------------------------
//   musicXmlNotePitch
//---------------------------------------------------------

/**
 Parse the note pitch related part of the /score-partwise/part/measure/note node.
 */

class MusicXmlNotePitch
{
public:
    MusicXmlNotePitch(MusicXmlLogger* logger)
        : m_logger(logger) { /* nothing so far */ }
    void pitch(muse::XmlStreamReader& e);
    bool readProperties(muse::XmlStreamReader& e, engraving::Score* score);
    engraving::Accidental* acc() const { return m_acc; }
    engraving::AccidentalType accType() const { return m_accType; }
    int alter() const { return m_alter; }
    double tuning() const { return m_tuning; }
    int displayOctave() const { return m_displayOctave; }
    int displayStep() const { return m_displayStep; }
    void displayStepOctave(muse::XmlStreamReader& e);
    int octave() const { return m_octave; }
    int step() const { return m_step; }
    bool unpitched() const { return m_unpitched; }

private:
    engraving::Accidental* m_acc = nullptr;                             // created based on accidental element
    engraving::AccidentalType m_accType = engraving::AccidentalType::NONE;         // set by pitch() based on alter value (can be microtonal)
    int m_alter = 0;
    double m_tuning = 0.0;
    int m_displayStep = -1;                                  // invalid
    int m_displayOctave = -1;                                // invalid
    int m_octave = -1;
    int m_step = 0;
    bool m_unpitched = false;
    MusicXmlLogger* m_logger = nullptr;                          // Error logger
};
} // namespace Ms
