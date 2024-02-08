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

#ifndef __MUSICXML_H__
#define __MUSICXML_H__

/**
 \file
 Definition of class MusicXML
*/

#include "engraving/types/fraction.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/line.h"
#include "importxmlfirstpass.h"
#include "musicxmlsupport.h"

namespace mu::engraving {
//---------------------------------------------------------
//   MusicXmlPartGroup
//---------------------------------------------------------

struct MusicXmlPartGroup {
    int span = 0;
    int start = 0;
    BracketType type = BracketType::NO_BRACKET;
    bool barlineSpan = false;
    int column = 0;
};

const int MAX_LYRICS       = 16;
const int MAX_PART_GROUPS  = 8;
const int MAX_NUMBER_LEVEL = 16; // maximum number of overlapping MusicXML objects

//---------------------------------------------------------
//   CreditWords
//    a single parsed MusicXML credit-words element
//---------------------------------------------------------

struct CreditWords {
    int page = 0;
    String type;
    double defaultX = 0.0;
    double defaultY = 0.0;
    double fontSize = 0.0;
    String justify;
    String hAlign;
    String vAlign;
    String words;
    CreditWords(int p, String tp, double dx, double dy, double fs, String j, String ha, String va, String w)
    {
        page = p;
        type = tp;
        defaultX = dx;
        defaultY = dy;
        fontSize = fs;
        justify  = j;
        hAlign   = ha;
        vAlign   = va;
        words    = w;
    }
};

typedef  std::vector<CreditWords*> CreditWordsList;
typedef  CreditWordsList::iterator iCreditWords;
typedef  CreditWordsList::const_iterator ciCreditWords;

//---------------------------------------------------------
//   JumpMarkerDesc
//---------------------------------------------------------

/**
 The description of Jumps and Markers to be added later
*/

class JumpMarkerDesc
{
public:
    JumpMarkerDesc(EngravingItem* el, Measure* meas)
        : m_el(el), m_meas(meas) {}
    EngravingItem* el() const { return m_el; }
    Measure* meas() const { return m_meas; }

private:
    EngravingItem* m_el = nullptr;
    Measure* m_meas = nullptr;
};

typedef std::vector<JumpMarkerDesc> JumpMarkerDescList;

//---------------------------------------------------------
//   SlurDesc
//---------------------------------------------------------

/**
 The description of Slurs being handled
 */

class SlurDesc
{
public:
    enum class State : char {
        NONE, START, STOP
    };
    SlurDesc()
        : m_slur(0), m_state(State::NONE) {}
    Slur* slur() const { return m_slur; }
    void start(Slur* slur) { m_slur = slur; m_state = State::START; }
    void stop(Slur* slur) { m_slur = slur; m_state = State::STOP; }
    bool isStart() const { return m_state == State::START; }
    bool isStop() const { return m_state == State::STOP; }
private:
    Slur* m_slur = nullptr;
    State m_state;
};

//---------------------------------------------------------
//   MusicXml
//---------------------------------------------------------

typedef std::vector<MusicXmlPartGroup*> MusicXmlPartGroupList;
typedef std::map<SLine*, std::pair<int, int> > MusicXmlSpannerMap;
} // namespace Ms
#endif
