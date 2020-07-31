//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "element.h"
#include "score.h"
#include "scoreElement.h"
#include "staff.h"
#include "spanner.h"
#include "measure.h"

namespace Ms {
//---------------------------------------------------------
//   shouldWrite
///   Check if property / element / etc should be written
//---------------------------------------------------------

ElementType dontWriteTheseElements[] = {
    ElementType::BEAM,       ElementType::LEDGER_LINE, ElementType::TUPLET,
    ElementType::SLUR,       ElementType::TIE,         ElementType::GLISSANDO,
    ElementType::LYRICSLINE, ElementType::TEXTLINE,    ElementType::STAFF_LINES,
};

static bool shouldWrite(ScoreElement* e)
{
    for (ElementType t : dontWriteTheseElements) {
        if (e->type() == t) {
            return false;
        }
    }
    return true;
}

std::map<ElementType, std::vector<Pid> > propertiesToWrite = {
    {
        ElementType::NOTE,
        {
            Pid::PITCH, Pid::TPC1, Pid::TPC2, Pid::SMALL, Pid::MIRROR_HEAD,
            Pid::DOT_POSITION, Pid::HEAD_SCHEME, Pid::HEAD_GROUP, Pid::VELO_OFFSET,
            Pid::PLAY, Pid::TUNING, Pid::FRET, Pid::STRING, Pid::GHOST,
            Pid::HEAD_TYPE, Pid::VELO_TYPE, Pid::FIXED, Pid::FIXED_LINE,
        }
    },
    {
        ElementType::CHORD,
        {
            Pid::SMALL, Pid::STAFF_MOVE, Pid::DURATION_TYPE, Pid::DURATION, Pid::STEM_DIRECTION
        }
    },
    {
        ElementType::REST,
        {
            Pid::SMALL, Pid::STAFF_MOVE, Pid::DURATION_TYPE, Pid::DURATION
        }
    },
    {
        ElementType::ACCIDENTAL,
        {
            Pid::ACCIDENTAL_BRACKET, Pid::ROLE, Pid::SMALL, Pid::ACCIDENTAL_TYPE,
        }
    },
    {
        ElementType::LAYOUT_BREAK,
        {
            Pid::LAYOUT_BREAK,
        }
    },
    {
        ElementType::CLEF,
        {
            Pid::CLEF_TYPE_CONCERT,
            Pid::CLEF_TYPE_TRANSPOSING,
        }
    },
    {
        ElementType::CLEF,
        {
            Pid::CLEF_TYPE_CONCERT,
            Pid::CLEF_TYPE_TRANSPOSING,
        }
    },
    {
        ElementType::KEYSIG,
        {
            Pid::KEYSIG_MODE,
            Pid::SHOW_COURTESY,
            Pid::KEY,
        }
    },
    {
        ElementType::VBOX,
        {
            Pid::BOX_HEIGHT, Pid::BOX_WIDTH, Pid::TOP_GAP, Pid::BOTTOM_GAP,
            Pid::LEFT_MARGIN, Pid::RIGHT_MARGIN, Pid::TOP_MARGIN, Pid::BOTTOM_MARGIN,
        }
    },
    {
        ElementType::HBOX,
        {
            Pid::BOX_HEIGHT, Pid::BOX_WIDTH, Pid::TOP_GAP, Pid::BOTTOM_GAP,
            Pid::LEFT_MARGIN, Pid::RIGHT_MARGIN, Pid::TOP_MARGIN, Pid::BOTTOM_MARGIN,
            Pid::CREATE_SYSTEM_HEADER,
        }
    },
    {
        ElementType::TEXT,
        {
            Pid::SUB_STYLE, Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_STYLE, Pid::COLOR,
            Pid::FRAME_TYPE, Pid::FRAME_WIDTH, Pid::FRAME_PADDING, Pid::FRAME_ROUND,
            Pid::FRAME_FG_COLOR, Pid::FRAME_BG_COLOR, Pid::ALIGN,
            Pid::TEXT
        }
    }
};

//---------------------------------------------------------
//   writeAllProperties
//---------------------------------------------------------

static void writeAllProperties(XmlWriter& xml, Element* e)
{
    for (auto p : propertiesToWrite[e->type()]) {
        if (e->getProperty(p).isValid()) {
            e->writeProperty(xml, Pid(p));
        }
    }
}

//---------------------------------------------------------
//   anyElementsInTrack
//---------------------------------------------------------

static bool anyElementsInTrack(Measure* m, int track)
{
    for (const Segment& s : m->segments()) {
        Element* e = s.element(track);
        if (e) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   ScoreElement
//---------------------------------------------------------

void ScoreElement::treeWrite(XmlWriter& xml)
{
    xml.stag(this);
    for (ScoreElement* ch : *this) {
        ch->treeWrite(xml);
    }
    xml.etag();
}

//---------------------------------------------------------
//   Element
//---------------------------------------------------------

void Element::treeWrite(XmlWriter& xml)
{
    if (isTuplet()) {
        qDebug();
    }
    if (generated()) {
        return;
    }
    if (!isUserModified() && !shouldWrite(this)) {
        return;
    }
    xml.stag(this);
    writeAllProperties(xml, this);
    for (ScoreElement* ch : *this) {
        ch->treeWrite(xml);
    }
    xml.etag();
}

//---------------------------------------------------------
//   for Spanners
//---------------------------------------------------------

void Spanner::treeWrite(XmlWriter& xml)
{
    this->writeSpannerStart(xml, toElement(treeParent()), track());
}

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

void Score::treeWrite(XmlWriter& xml)
{
    xml.header();
    xml.stag("museScore version=\"3.01\"");
    xml.stag(this);
    // write all measures in staff 1 first, then staff 2, etc.
    for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
        xml.stag(staff(staffIdx), QString("id=\"%1\"").arg(staffIdx + 1));
        for (MeasureBase* m = measures()->first(); m != nullptr; m = m->next()) {
            if (m->isMeasure()) {
                toMeasure(m)->treeWriteStaff(xml, staffIdx);
            } else {
                // non-measure things (like boxes) to be written only once in staff 1
                if (staffIdx == 0) {
                    m->treeWrite(xml);
                }
            }
        }
        xml.etag();
    }
    xml.etag(); // score
    xml.etag(); // musescore
}

//---------------------------------------------------------
//   Measure::treeWriteStaff
///   Writes one staff in one measure
//---------------------------------------------------------

void Measure::treeWriteStaff(XmlWriter& xml, int staffIdx)
{
    xml.stag(this);
    for (Element* e : el()) {
        e->treeWrite(xml);
    }
    // write voice 1 first, then voice 2, .. upto VOICES
    for (int voice = 0; voice < VOICES; voice++) {
        int track = staffIdx * VOICES + voice;
        xml.setCurTrack(track);
        // check if voice empty?
        if (!anyElementsInTrack(this, track)) {
            continue;
        }
        xml.stag("voice");
        for (const Segment& s : segments()) {
            Element* e = s.element(track);
            if (e) {
                e->treeWrite(xml);
            }
        }
        xml.etag(); // voice
    }
    xml.etag(); // measure
}
} // namespace Ms
