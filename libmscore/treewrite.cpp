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

#include "chord.h"
#include "element.h"
#include "measure.h"
#include "score.h"
#include "scoreElement.h"
#include "spanner.h"
#include "staff.h"
#include "tie.h"

namespace Ms {
//---------------------------------------------------------
//   shouldWrite
///   Check if property / element / etc should be written
//---------------------------------------------------------

ElementType dontWriteTheseElements[] = {
    ElementType::BEAM,
    ElementType::LEDGER_LINE,
    ElementType::TUPLET,
    ElementType::TEXTLINE,
    ElementType::STAFF_LINES,
    ElementType::LYRICSLINE,
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

// properties to be written into the XML file
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
        ElementType::ARPEGGIO,
        {
            Pid::ARPEGGIO_TYPE, Pid::PLAY, Pid::TIME_STRETCH,
        }
    },
    {
        ElementType::BAR_LINE,
        {
            Pid::BARLINE_TYPE, Pid::BARLINE_SPAN,
        }
    },
    {
        ElementType::DYNAMIC,
        {
            Pid::DYNAMIC_TYPE, Pid::VELOCITY,
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
    },
    {
        ElementType::TIMESIG,
        {
            Pid::TIMESIG_TYPE, Pid::NUMERATOR, Pid::DENOMINATOR, Pid::NUMERATOR_STRING,
            Pid::DENOMINATOR_STRING, Pid::SHOW_COURTESY, Pid::SCALE
        }
    },
    {
        ElementType::TEMPO_TEXT,
        {
            Pid::TEMPO, Pid::TEMPO_FOLLOW_TEXT, Pid::SUB_STYLE, Pid::TEXT,
        }
    },
    {
        ElementType::SYSTEM_TEXT,
        {
            Pid::TEXT,
        }
    },
    {
        ElementType::STAFF_TEXT,
        {
            Pid::TEXT,
        }
    },
    {
        ElementType::FINGERING,
        {
            Pid::TEXT,
        }
    },
    {
        ElementType::REHEARSAL_MARK,
        {
            Pid::TEXT,
        }
    },
    {
        ElementType::SYMBOL,
        {
            Pid::SYMBOL, // Pid:: ScoreFont?
        }
    },
    {
        ElementType::FERMATA,
        {
            Pid::SUBTYPE, Pid::TIME_STRETCH, Pid::PLAY, Pid::MIN_DISTANCE, Pid::OFFSET,
        }
    },
    {
        ElementType::HARMONY,
        {
            Pid::PLAY, Pid::HARMONY_TYPE, Pid::HARMONY_VOICE_LITERAL, Pid::HARMONY_VOICING, Pid::HARMONY_DURATION,
        }
    },
    {
        ElementType::TREMOLOBAR,
        {
            Pid::MAG,   Pid::LINE_WIDTH,   Pid::PLAY,
        }
    },
    {
        ElementType::LYRICS,
        {
            Pid::VERSE, Pid::SYLLABIC, Pid::LYRIC_TICKS, Pid::TEXT,
        }
    },
    {
        ElementType::BREATH,
        {
            Pid::SYMBOL, Pid::PAUSE,
        }
    },
    {
        ElementType::TREMOLO,
        {
            Pid::TREMOLO_TYPE,Pid::TREMOLO_PLACEMENT,Pid::TREMOLO_STROKE_STYLE,
        }
    },
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
    for (const StyledProperty& spp : *(e->styledProperties())) {
        e->writeProperty(xml, spp.pid);
    }
    e->Element::writeProperties(xml);
}

//---------------------------------------------------------
//   writeSpannerEnds
//---------------------------------------------------------

static void writeSpannerEnds(XmlWriter& xml, Element* e, int track)
{
    for (auto i : e->score()->spanner()) {
        Spanner* s = i.second;
        if (s->track() == track && s->endParent() == e) {
            s->writeSpannerEnd(xml, e, track);
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
    if (generated() || (systemFlag() && staffIdx() != 0)) {
        return;
    }
    if (isUserModified() || shouldWrite(this)) {
        xml.stag(this);
        writeAllProperties(xml, this);
        writeSpannerEnds(xml, this, track());
        for (ScoreElement* ch : *this) {
            ch->treeWrite(xml);
        }
        xml.etag();
    }
}

//---------------------------------------------------------
//   for Spanners
//---------------------------------------------------------

void Spanner::treeWrite(XmlWriter& xml)
{
    if (shouldWrite(this)) {
        writeSpannerStart(xml, toElement(treeParent()), track());
    }
}

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

void Score::treeWrite(XmlWriter& xml)
{
    dumpScoreTree(); // TODO: remove
    for (auto i : score()->spanner()) {
        Spanner* s = i.second;
        qDebug() << "spanner " << s->name() << "with anchor" << s->anchor()
                 << "parent" << toElement(s->treeParent())->accessibleInfo()
                 << "at" << s->treeParent() << "endParent"
                 << toElement(s->endParent())->accessibleInfo() << "at"
                 << s->endParent() << s->tick() << s->tick2();
    }
    // end debug info
    xml.header();
    xml.stag("museScore version=\"3.01\"");
    xml.stag(this);

    // Write various metadata
    xml.tag("playMode", int(_playMode));
    xml.tag("currentLayer", _currentLayer);
    xml.tag("page-offset", pageNumberOffset(), 0);
    xml.tag("Division", MScore::division);
    style().save(xml, true);
    xml.tag("showInvisible", _showInvisible);
    xml.tag("showUnprintable", _showUnprintable);
    xml.tag("showFrames", _showFrames);
    xml.tag("showMargins", _showPageborders);
    xml.tag("markIrregularMeasures", _markIrregularMeasures, true);
    QMapIterator<QString, QString> i(_metaTags);
    while (i.hasNext()) {
        i.next();
        // do not output "platform" and "creationDate" in test and save template mode
        if ((!MScore::testMode && !MScore::saveTemplateMode) || (i.key() != "platform" && i.key() != "creationDate")) {
            xml.tag(QString("metaTag name=\"%1\"").arg(i.key().toHtmlEscaped()), i.value());
        }
    }

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
    // find last non empty voice
    int lastNonEmptyVoice = 0;
    for (int voice = 0; voice < VOICES; voice++) {
        int track = staffIdx * VOICES + voice;
        // check if voice empty?
        if (anyElementsInTrack(this, track)) {
            lastNonEmptyVoice = voice;
        }
    }
    for (int voice = 0; voice <= lastNonEmptyVoice; voice++) {
        int track = staffIdx * VOICES + voice;
        xml.setCurTrack(track);
        xml.stag("voice");
        for (Segment& s : segments()) {
            writeSpannerEnds(xml, &s, track);
            // write elements associated with this track
            for (ScoreElement* e : s) {
                if (toElement(e)->track() == track) {
                    e->treeWrite(xml);
                }
            }
        }
        xml.etag(); // voice
    }
    xml.etag(); // measure
}

//---------------------------------------------------------
//   Chord::treeWrite
//---------------------------------------------------------

void Chord::treeWrite(XmlWriter& xml)
{
    if (tuplet()) {
        writeTupletStart(xml);
    }
    for (Chord* grace : graceNotes()) {
        grace->treeWrite(xml);
    }
    xml.stag(this);
    writeAllProperties(xml, this);
    writeSpannerEnds(xml, this, track());
    for (ScoreElement* ch : *this) {
        if (!ch->isChord()) { // grace notes already written
            ch->treeWrite(xml);
        }
    }
    xml.etag();
    if (tuplet()) {
        writeTupletEnd(xml);
    }
}

//---------------------------------------------------------
//   Note::treeWrite
//---------------------------------------------------------

void Note::treeWrite(XmlWriter& xml)
{
    xml.stag(this);
    writeSpannerEnds(xml, this, track());
    for (Spanner* s : spannerBack()) {
        s->writeSpannerEnd(xml, this, track());
    }
    if (tieBack()) {
        tieBack()->writeSpannerEnd(xml, this, track());
    }
    for (ScoreElement* ch : *this) {
        if (ch->isSpanner()) {
            toSpanner(ch)->writeSpannerStart(xml, this, track());
        } else {
            ch->treeWrite(xml);
        }
    }
    writeAllProperties(xml, this);
    xml.etag();
}
} // namespace Ms
