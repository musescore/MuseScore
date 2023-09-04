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
    int span;
    int start;
    BracketType type;
    bool barlineSpan;
    int column;
};

const int MAX_LYRICS       = 16;
const int MAX_PART_GROUPS  = 8;
const int MAX_NUMBER_LEVEL = 16; // maximum number of overlapping MusicXML objects

//---------------------------------------------------------
//   CreditWords
//    a single parsed MusicXML credit-words element
//---------------------------------------------------------

struct CreditWords {
    int page;
    QString type;
    double defaultX;
    double defaultY;
    double fontSize;
    QString justify;
    QString hAlign;
    QString vAlign;
    QString words;
    CreditWords(int p, QString tp, double dx, double dy, double fs, QString j, QString ha, QString va, QString w)
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

typedef  QList<CreditWords*> CreditWordsList;
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
    EngravingItem* _el;
    Measure* _meas;

public:
    JumpMarkerDesc(EngravingItem* el, Measure* meas)
        : _el(el), _meas(meas) {}
    EngravingItem* el() const { return _el; }
    Measure* meas() const { return _meas; }
};

typedef QList<JumpMarkerDesc> JumpMarkerDescList;

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
        : _slur(0), _state(State::NONE) {}
    Slur* slur() const { return _slur; }
    void start(Slur* slur) { _slur = slur; _state = State::START; }
    void stop(Slur* slur) { _slur = slur; _state = State::STOP; }
    bool isStart() const { return _state == State::START; }
    bool isStop() const { return _state == State::STOP; }
private:
    Slur* _slur;
    State _state;
};

//---------------------------------------------------------
//   MusicXml
//---------------------------------------------------------

typedef std::vector<MusicXmlPartGroup*> MusicXmlPartGroupList;
typedef QMap<SLine*, QPair<int, int> > MusicXmlSpannerMap;
} // namespace Ms
#endif
