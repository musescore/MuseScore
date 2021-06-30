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

#include "ove.h"

#include <QtMath>

#include "libmscore/sig.h"
#include "libmscore/tempo.h"
#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/box.h"
#include "libmscore/bracket.h"
#include "libmscore/breath.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "libmscore/drumset.h"
#include "libmscore/dynamic.h"
#include "libmscore/hairpin.h"
#include "libmscore/harmony.h"
#include "libmscore/glissando.h"
#include "libmscore/keysig.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/lyrics.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "libmscore/accidental.h"
#include "libmscore/ottava.h"
#include "libmscore/part.h"
#include "libmscore/pedal.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/measurerepeat.h"
#include "libmscore/rest.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/staff.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/timesig.h"
#include "libmscore/tuplet.h"
#include "libmscore/tremolo.h"
#include "libmscore/volta.h"
#include "libmscore/chordlist.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/marker.h"
#include "libmscore/jump.h"
#include "libmscore/symid.h"
#include "libmscore/bracketItem.h"

#include "modularity/ioc.h"
#include "importexport/ove/ioveconfiguration.h"

namespace ove {
static std::shared_ptr<mu::iex::ove::IOveConfiguration> configuration()
{
    return mu::modularity::ioc()->resolve<mu::iex::ove::IOveConfiguration>("iex_ove");
}
}

using namespace Ms;

class MeasureToTick
{
public:
    MeasureToTick();
    ~MeasureToTick();

public:
    void build(ovebase::OveSong* ove, int quarter);

    int getTick(int measure, int tick_pos);
    static int unitToTick(int unit, int quarter);

    struct TimeTick {
        int m_numerator;
        int m_denominator;
        int m_measure;
        int m_tick;
        bool m_isSymbol;

        TimeTick()
            : m_numerator(4), m_denominator(4), m_measure(0), m_tick(0), m_isSymbol(false) {}
    };
    QList<TimeTick> getTimeTicks() const;

private:
    int m_quarter;
    ovebase::OveSong* m_ove;
    QList<TimeTick> m_tts;
};

int getMeasureTick(int quarter, int num, int den)
{
    return quarter * 4 * num / den;
}

MeasureToTick::MeasureToTick()
{
    m_quarter = 480;
    m_ove = 0;
}

MeasureToTick::~MeasureToTick()
{
}

void MeasureToTick::build(ovebase::OveSong* ove, int quarter)
{
    unsigned int i;
    int currentTick = 0;
    unsigned int measureCount = ove->getMeasureCount();

    m_quarter = quarter;
    m_ove = ove;
    m_tts.clear();

    for (i = 0; i < measureCount; ++i) {
        ovebase::Measure* measure = m_ove->getMeasure(i);
        ovebase::TimeSignature* time = measure->getTime();
        TimeTick tt;
        bool change = false;

        tt.m_tick = currentTick;
        tt.m_numerator = time->getNumerator();
        tt.m_denominator = time->getDenominator();
        tt.m_measure = i;
        tt.m_isSymbol = time->getIsSymbol();

        if (i == 0) {
            change = true;
        } else {
            ovebase::TimeSignature* previousTime = m_ove->getMeasure(i - 1)->getTime();

            if (time->getNumerator() != previousTime->getNumerator()
                || time->getDenominator() != previousTime->getDenominator()) {
                change = true;
            } else if (time->getIsSymbol() != previousTime->getIsSymbol()) {
                change = true;
            }
        }

        if (change) {
            m_tts.push_back(tt);
        }

        currentTick += getMeasureTick(m_quarter, tt.m_numerator, tt.m_denominator);
    }
}

int MeasureToTick::getTick(int measure, int tick_pos)
{
    TimeTick tt;

    for (int i = 0; i < m_tts.size(); ++i) {
        if (measure >= m_tts[i].m_measure && (i == m_tts.size() - 1 || measure < m_tts[i + 1].m_measure)) {
            int measuresTick = (measure - m_tts[i].m_measure) * getMeasureTick(m_quarter, m_tts[i].m_numerator,
                                                                               m_tts[i].m_denominator);
            return m_tts[i].m_tick + measuresTick + tick_pos;
        }
    }

    return 0;
}

int MeasureToTick::unitToTick(int unit, int quarter)
{
    // 0x100 correspond to quarter tick
    float ratio = (float)unit / (float)256.0;
    int tick = ratio * quarter;
    return tick;
}

QList<MeasureToTick::TimeTick> MeasureToTick::getTimeTicks() const
{
    return m_tts;
}

class OveToMScore
{
public:
    OveToMScore();
    ~OveToMScore();

public:
    void convert(ovebase::OveSong* oveData, Score* score);

private:
    void createStructure();
    void convertHeader();
    void convertGroups();
    void convertTrackHeader(ovebase::Track* track, Part* part);
    void convertTrackElements(int track);
    void convertLineBreak();
    void convertSignatures();
    void convertMeasures();
    void convertMeasure(Measure* measure);
    void convertMeasureMisc(Measure* measure, int part, int staff, int track);
    void convertNotes(Measure* measure, int part, int staff, int track);
    void convertArticulation(Measure* measure, ChordRest* cr, int track, int absTick, ovebase::Articulation* art);
    void convertLyrics(Measure* measure, int part, int staff, int track);
    void convertHarmonys(Measure* measure, int part, int staff, int track);
    void convertRepeats(Measure* measure, int part, int staff, int track);
    void convertDynamics(Measure* measure, int part, int staff, int track);
    void convertExpressions(Measure* measure, int part, int staff, int track);
    void convertLines(Measure* measure);
    void convertSlurs(Measure* measure, int part, int staff, int track);
    void convertGlissandos(Measure* measure, int part, int staff, int track);
    void convertWedges(Measure* measure, int part, int staff, int track);
    void convertOctaveShifts(Measure* measure, int part, int staff, int track);

    ovebase::NoteContainer* getContainerByPos(int part, int staff, const ovebase::MeasurePos& pos);
    // ovebase::MusicData* getMusicDataByUnit(int part, int staff, int measure, int unit, ovebase::MusicDataType type);
    ovebase::MusicData* getCrossMeasureElementByPos(int part, int staff, const ovebase::MeasurePos& pos, int voice,
                                                    ovebase::MusicDataType type);
    ChordRest* findChordRestByPos(int absTick, int track);

    void clearUp();

private:
    ovebase::OveSong* m_ove;
    Score* m_score;
    MeasureToTick* m_mtt;

    Pedal* m_pedal;
};

OveToMScore::OveToMScore()
{
    m_ove = 0;
    m_mtt = new MeasureToTick();
    m_pedal = 0;
}

OveToMScore::~OveToMScore()
{
    delete m_mtt;
}

void OveToMScore::convert(ovebase::OveSong* ove, Score* score)
{
    m_ove = ove;
    m_score = score;
    m_mtt->build(m_ove, m_ove->getQuarter());

    convertHeader();
    createStructure();
    convertGroups();
    convertSignatures();
    // convertLineBreak();

    int staffCount = 0;
    for (int i = 0; i < m_ove->getPartCount(); ++i) {
        int partStaffCount = m_ove->getStaffCount(i);
        Part* part = m_score->parts().at(i);

        for (int j = 0; j < partStaffCount; ++j) {
            ovebase::Track* track = m_ove->getTrack(i, j);

            convertTrackHeader(track, part);
        }

        staffCount += partStaffCount;
    }

    convertMeasures();

    // convert elements by ove track sequence
    staffCount = 0;
    for (int i = 0; i < m_ove->getPartCount(); ++i) {
        int partStaffCount = m_ove->getStaffCount(i);

        for (int j = 0; j < partStaffCount; ++j) {
            int trackIndex = staffCount + j;

            convertTrackElements(trackIndex);
        }

        staffCount += partStaffCount;
    }

    clearUp();
}

void OveToMScore::createStructure()
{
    int i;
    for (i = 0; i < m_ove->getPartCount(); ++i) {
        int partStaffCount = m_ove->getStaffCount(i);
        Part* part = new Part(m_score);

        for (int j = 0; j < partStaffCount; ++j) {
            //ovebase::Track* track = m_ove->getTrack(i, j);
            Staff* staff = new Staff(m_score);
            staff->setPart(part);

            part->staves()->push_back(staff);
            m_score->staves().push_back(staff);
        }

        m_score->appendPart(part);
        part->setStaves(partStaffCount);
    }

    for (i = 0; i < m_ove->getMeasureCount(); ++i) {
        Measure* measure  = new Measure(m_score);
        int tick = m_mtt->getTick(i, 0);
        measure->setTick(Fraction::fromTicks(tick));
        measure->setNo(i);
        m_score->measures()->add(measure);
    }
}

void OveToMScore::clearUp()
{
    if (m_pedal != NULL) {
        delete m_pedal;
        m_pedal = 0;
    }
}

ovebase::Staff* getStaff(const ovebase::OveSong* ove, int track)
{
    if (ove->getLineCount() > 0) {
        ovebase::Line* line = ove->getLine(0);
        if (line != 0 && line->getStaffCount() > 0) {
            ovebase::Staff* staff = line->getStaff(track);
            return staff;
        }
    }

    return 0;
}

namespace ove {
void addText(VBox*& vbox, Score* s, QString strTxt, Tid stl)
{
    if (!strTxt.isEmpty()) {
        Text* text = new Text(s, stl);
        text->setPlainText(strTxt);
        if (vbox == 0) {
            vbox = new VBox(s);
        }
        vbox->add(text);
    }
}
}

void OveToMScore::convertHeader()
{
    VBox* vbox = 0;
    QList<QString> titles = m_ove->getTitles();
    if (!titles.empty() && !titles[0].isEmpty()) {
        QString title = titles[0];
        m_score->setMetaTag("movementTitle", title);
        ove::addText(vbox, m_score, title, Tid::TITLE);
    }

    QList<QString> copyrights = m_ove->getCopyrights();
    if (!copyrights.empty() && !copyrights[0].isEmpty()) {
        QString copyright = copyrights[0];
        m_score->setMetaTag("copyright", copyright);
    }

    QList<QString> annotates = m_ove->getAnnotates();
    if (!annotates.empty() && !annotates[0].isEmpty()) {
        QString annotate = annotates[0];
        ove::addText(vbox, m_score, annotate, Tid::POET);
    }

    QList<QString> writers = m_ove->getWriters();
    if (!writers.empty()) {
        QString composer = writers[0];
        m_score->setMetaTag("composer", composer);
        ove::addText(vbox, m_score, composer, Tid::COMPOSER);
    }

    if (writers.size() > 1) {
        QString lyricist = writers[1];
        ove::addText(vbox, m_score, lyricist, Tid::POET);
    }

    if (vbox) {
        vbox->setTick(Fraction(0, 1));
        m_score->measures()->add(vbox);
    }
}

void OveToMScore::convertGroups()
{
    int i;
    int staffCount = 0;
    const QList<Part*>& parts = m_score->parts();
    for (i = 0; i < m_ove->getPartCount(); ++i) {
        int partStaffCount = m_ove->getStaffCount(i);
        //if(parts == 0)
        //	continue;
        Part* part = parts.at(i);
        if (part == 0) {
            continue;
        }

        for (int j = 0; j < partStaffCount; ++j) {
            int staffIndex = staffCount + j;
            Staff* staff = m_score->staff(staffIndex);
            if (staff == 0) {
                continue;
            }

            // brace
            if (j == 0 && partStaffCount == 2) {
                staff->setBracketType(0, BracketType::BRACE);
                staff->setBracketSpan(0, 2);
                staff->setBarLineSpan(2);
            }

            // bracket
            ovebase::Staff* staffPtr = getStaff(m_ove, staffIndex);
            if (staffPtr != 0 && staffPtr->getGroupType() == ovebase::GroupType::Bracket) {
                int span = staffPtr->getGroupStaffCount() + 1;
                int endStaff = staffIndex + span;
                if (span > 0 && endStaff >= staffIndex && endStaff <= m_ove->getTrackCount()) {
                    staff->addBracket(new BracketItem(staff->score(), BracketType::NORMAL, span));
                    staff->setBarLineSpan(span);
                }
            }
        }

        staffCount += partStaffCount;
    }
}

ClefType OveClefToClef(ovebase::ClefType type)
{
    ClefType clef = ClefType::G;
    switch (type) {
    case ovebase::ClefType::Treble: {
        clef = ClefType::G;
        break;
    }
    case ovebase::ClefType::Bass: {
        clef = ClefType::F;
        break;
    }
    case ovebase::ClefType::Alto: {
        clef = ClefType::C3;
        break;
    }
    case ovebase::ClefType::UpAlto: {
        clef = ClefType::C4;
        break;
    }
    case ovebase::ClefType::DownDownAlto: {
        clef = ClefType::C1;
        break;
    }
    case ovebase::ClefType::DownAlto: {
        clef = ClefType::C2;
        break;
    }
    case ovebase::ClefType::UpUpAlto: {
        clef = ClefType::C5;
        break;
    }
    case ovebase::ClefType::Treble8va: {
        clef = ClefType::G8_VA;
        break;
    }
    case ovebase::ClefType::Bass8va: {
        clef = ClefType::F_8VA;
        break;
    }
    case ovebase::ClefType::Treble8vb: {
        clef = ClefType::G8_VB;
        break;
    }
    case ovebase::ClefType::Bass8vb: {
        clef = ClefType::F8_VB;
        break;
    }
    case ovebase::ClefType::Percussion1: {
        clef = ClefType::PERC;
        break;
    }
    case ovebase::ClefType::Percussion2: {
        clef = ClefType::PERC2;
        break;
    }
    case ovebase::ClefType::TAB: {
        clef = ClefType::TAB;
        break;
    }
    default:
        break;
    }
    return clef;
}

NoteHead::Group getHeadGroup(ovebase::NoteHeadType type)
{
    NoteHead::Group headGroup = NoteHead::Group::HEAD_NORMAL;
    switch (type) {
    case ovebase::NoteHeadType::Standard: {
        headGroup = NoteHead::Group::HEAD_NORMAL;
        break;
    }
    case ovebase::NoteHeadType::Invisible: {
        break;
    }
    case ovebase::NoteHeadType::Rhythmic_Slash: {
        headGroup = NoteHead::Group::HEAD_SLASH;
        break;
    }
    case ovebase::NoteHeadType::Percussion: {
        headGroup = NoteHead::Group::HEAD_XCIRCLE;
        break;
    }
    case ovebase::NoteHeadType::Closed_Rhythm: {
        headGroup = NoteHead::Group::HEAD_CROSS;
        break;
    }
    case ovebase::NoteHeadType::Open_Rhythm: {
        headGroup = NoteHead::Group::HEAD_CROSS;
        break;
    }
    case ovebase::NoteHeadType::Closed_Slash: {
        headGroup = NoteHead::Group::HEAD_SLASH;
        break;
    }
    case ovebase::NoteHeadType::Open_Slash: {
        headGroup = NoteHead::Group::HEAD_SLASH;
        break;
    }
    case ovebase::NoteHeadType::Closed_Do: {
        headGroup = NoteHead::Group::HEAD_DO;
        break;
    }
    case ovebase::NoteHeadType::Open_Do: {
        headGroup = NoteHead::Group::HEAD_DO;
        break;
    }
    case ovebase::NoteHeadType::Closed_Re: {
        headGroup = NoteHead::Group::HEAD_RE;
        break;
    }
    case ovebase::NoteHeadType::Open_Re: {
        headGroup = NoteHead::Group::HEAD_RE;
        break;
    }
    case ovebase::NoteHeadType::Closed_Mi: {
        headGroup = NoteHead::Group::HEAD_MI;
        break;
    }
    case ovebase::NoteHeadType::Open_Mi: {
        headGroup = NoteHead::Group::HEAD_MI;
        break;
    }
    case ovebase::NoteHeadType::Closed_Fa: {
        headGroup = NoteHead::Group::HEAD_FA;
        break;
    }
    case ovebase::NoteHeadType::Open_Fa: {
        headGroup = NoteHead::Group::HEAD_FA;
        break;
    }
    case ovebase::NoteHeadType::Closed_Sol: {
        break;
    }
    case ovebase::NoteHeadType::Open_Sol: {
        break;
    }
    case ovebase::NoteHeadType::Closed_La: {
        headGroup = NoteHead::Group::HEAD_LA;
        break;
    }
    case ovebase::NoteHeadType::Open_La: {
        headGroup = NoteHead::Group::HEAD_LA;
        break;
    }
    case ovebase::NoteHeadType::Closed_Ti: {
        headGroup = NoteHead::Group::HEAD_TI;
        break;
    }
    case ovebase::NoteHeadType::Open_Ti: {
        headGroup = NoteHead::Group::HEAD_TI;
        break;
    }
    default: {
        break;
    }
    }

    return headGroup;
}

void OveToMScore::convertTrackHeader(ovebase::Track* track, Part* part)
{
    if (track == 0 || part == 0) {
        return;
    }

    QString longName = track->getName();
    if (longName != QString() && track->getShowName()) {
        part->setPlainLongName(longName);
    }

    QString shortName = track->getBriefName();
    if (shortName != QString() && track->getShowBriefName()) {
        part->setPlainShortName(shortName);
    }

    part->setMidiProgram(track->getPatch());

    if (m_ove->getShowTransposeTrack() && track->getTranspose() != 0) {
        Ms::Interval interval = part->instrument()->transpose();
        interval.diatonic = -track->getTranspose();
        part->instrument()->setTranspose(interval);
    }

    // DrumSet
    if (track->getStartClef() == ovebase::ClefType::Percussion1 || track->getStartClef() == ovebase::ClefType::Percussion2) {
        // use overture drumset
        Drumset* drumset = new Drumset();
        for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
            drumset->drum(i).name     = smDrumset->drum(i).name;
            drumset->drum(i).notehead = smDrumset->drum(i).notehead;
            drumset->drum(i).line     = smDrumset->drum(i).line;
            drumset->drum(i).stemDirection = smDrumset->drum(i).stemDirection;
            drumset->drum(i).voice     = smDrumset->drum(i).voice;
            drumset->drum(i).shortcut = 0;
        }
        QList<ovebase::Track::DrumNode> nodes = track->getDrumKit();
        for (int i = 0; i < nodes.size(); ++i) {
            int pitch = nodes[i].m_pitch;
            ovebase::Track::DrumNode node = nodes[i];
            if (pitch < DRUM_INSTRUMENTS) {
                drumset->drum(pitch).line = node.m_line;
                drumset->drum(pitch).notehead = getHeadGroup(ovebase::NoteHeadType(node.m_headType));
                drumset->drum(pitch).voice = node.m_voice;
            }
        }

        part->instrument()->channel(0)->setBank(128);
        part->setMidiProgram(0);
        part->instrument()->setDrumset(smDrumset);
        part->instrument()->setDrumset(drumset);
    }
}

static OttavaType OctaveShiftTypeToInt(ovebase::OctaveShiftType type)
{
    OttavaType subtype = OttavaType::OTTAVA_8VA;
    switch (type) {
    case ovebase::OctaveShiftType::OS_8: {
        subtype = OttavaType::OTTAVA_8VA;
        break;
    }
    case ovebase::OctaveShiftType::OS_15: {
        subtype = OttavaType::OTTAVA_15MA;
        break;
    }
    case ovebase::OctaveShiftType::OS_Minus_8: {
        subtype = OttavaType::OTTAVA_8VB;
        break;
    }
    case ovebase::OctaveShiftType::OS_Minus_15: {
        subtype = OttavaType::OTTAVA_15MB;
        break;
    }
    default:
        break;
    }

    return subtype;
}

void OveToMScore::convertTrackElements(int track)
{
    Ottava* ottava = 0;

    for (int i = 0; i < m_ove->getTrackBarCount(); ++i) {
        ovebase::MeasureData* measureData = m_ove->getMeasureData(track, i);
        if (measureData == 0) {
            continue;
        }

        // octave shift
        QList<ovebase::MusicData*> octaves = measureData->getMusicDatas(ovebase::MusicDataType::OctaveShift_EndPoint);
        for (int j = 0; j < octaves.size(); ++j) {
            ovebase::OctaveShiftEndPoint* octave = static_cast<ovebase::OctaveShiftEndPoint*>(octaves[j]);
            int absTick = m_mtt->getTick(i, octave->getTick());

            if (octave->getOctaveShiftPosition() == ovebase::OctaveShiftPosition::Start) {
                if (ottava == 0) {
                    ottava = new Ottava(m_score);
                    ottava->setTrack(track * VOICES);
                    ottava->setOttavaType(OctaveShiftTypeToInt(octave->getOctaveShiftType()));

                    int y_off = 0;
                    switch (octave->getOctaveShiftType()) {
                    case ovebase::OctaveShiftType::OS_8:
                    case ovebase::OctaveShiftType::OS_15: {
                        y_off = -3;
                        break;
                    }
                    case ovebase::OctaveShiftType::OS_Minus_8:
                    case ovebase::OctaveShiftType::OS_Minus_15: {
                        y_off = 8;
                        break;
                    }
                    default: {
                        break;
                    }
                    }

                    if (y_off != 0) {
                        ottava->setOffset(mu::PointF(0, y_off * m_score->spatium()));
                    }

                    ottava->setTick(Fraction::fromTicks(absTick));
                } else {
                    qDebug("overlapping octave-shift not supported");
                    delete ottava;
                    ottava = 0;
                }
            } else if (octave->getOctaveShiftPosition() == ovebase::OctaveShiftPosition::Stop) {
                if (ottava != 0) {
                    ottava->setTick2(Fraction::fromTicks(absTick));
                    m_score->addSpanner(ottava);
                    ottava->staff()->updateOttava();
                    ottava = 0;
                } else {
                    qDebug("octave-shift stop without start");
                }
            }
        }
    }
}

void OveToMScore::convertLineBreak()
{
    for (MeasureBase* mb = m_score->measures()->first(); mb; mb = mb->next()) {
        if (mb->type() != ElementType::MEASURE) {
            continue;
        }
        Measure* measure = static_cast<Measure*>(mb);

        for (int i = 0; i < m_ove->getLineCount(); ++i) {
            ovebase::Line* line = m_ove->getLine(i);
            if (measure->no() > 0) {
                if ((int)line->getBeginBar() + (int)line->getBarCount() - 1 == measure->no()) {
                    LayoutBreak* lb = new LayoutBreak(m_score);
                    lb->setTrack(0);
                    lb->setLayoutBreakType(LayoutBreak::Type::LINE);
                    measure->add(lb);
                }
            }
        }
    }
}

void OveToMScore::convertSignatures()
{
    int i;
    int j;
    int k;

    // Time
    const QList<MeasureToTick::TimeTick> tts = m_mtt->getTimeTicks();
    for (i = 0; i < (int)tts.size(); ++i) {
        MeasureToTick::TimeTick tt = tts[i];
        Fraction f(tt.m_numerator, tt.m_denominator);

        TimeSigMap* sigmap = m_score->sigmap();
        sigmap->add(tt.m_tick, f);

        Measure* measure = m_score->tick2measure(Fraction::fromTicks(tt.m_tick));
        if (measure) {
            for (int staffIdx = 0; staffIdx < m_score->nstaves(); ++staffIdx) {
                TimeSigType subtype = TimeSigType::NORMAL;
                if (tt.m_numerator == 4 && tt.m_denominator == 4 && tt.m_isSymbol) {
                    subtype = TimeSigType::FOUR_FOUR;
                } else if (tt.m_numerator == 2 && tt.m_denominator == 2 && tt.m_isSymbol) {
                    subtype = TimeSigType::ALLA_BREVE;
                }

                TimeSig* ts = new TimeSig(m_score);
                ts->setTrack(staffIdx * VOICES);
                ts->setSig(Fraction(tt.m_numerator, tt.m_denominator), subtype);

                Segment* seg = measure->getSegment(SegmentType::TimeSig, Fraction::fromTicks(tt.m_tick));
                seg->add(ts);
            }
        }
    }

    // Key
    int staffCount = 0;
    bool createKey = false;
    for (i = 0; i < m_ove->getPartCount(); ++i) {
        int partStaffCount = m_ove->getStaffCount(i);

        for (j = 0; j < partStaffCount; ++j) {
            for (k = 0; k < m_ove->getMeasureCount(); ++k) {
                ovebase::MeasureData* measureData = m_ove->getMeasureData(i, j, k);

                if (measureData != 0) {
                    ovebase::Key* keyPtr = measureData->getKey();

                    if (k == 0 || keyPtr->getKey() != keyPtr->getPreviousKey()) {
                        int tick = m_mtt->getTick(k, 0);
                        int keyValue = keyPtr->getKey();
                        Measure* measure = m_score->tick2measure(Fraction::fromTicks(tick));
                        if (measure) {
                            KeySigEvent ke;
                            ke.setKey(Key(keyValue));
                            m_score->staff(staffCount + j)->setKey(Fraction::fromTicks(tick), ke);

                            KeySig* keysig = new KeySig(m_score);
                            keysig->setTrack((staffCount + j) * VOICES);
                            keysig->setKeySigEvent(ke);

                            Segment* s = measure->getSegment(SegmentType::KeySig, Fraction::fromTicks(tick));
                            s->add(keysig);

                            createKey = true;
                        }
                    }
                }
            }
        }

        staffCount += partStaffCount;
    }

    if (!createKey) {
        staffCount = 0;
        for (i = 0; i < m_ove->getPartCount(); ++i) {
            int partStaffCount = m_ove->getStaffCount(i);

            for (j = 0; j < partStaffCount; ++j) {
                Measure* measure = m_score->tick2measure(Fraction::fromTicks(m_mtt->getTick(0, 0)));
                if (measure) {
                    KeySig* keysig = new KeySig(m_score);
                    keysig->setTrack((staffCount + j) * VOICES);
                    keysig->setKeySigEvent(KeySigEvent());

                    Segment* s = measure->getSegment(SegmentType::KeySig, Fraction(0, 1));
                    s->add(keysig);
                }
            }
            staffCount += partStaffCount;
        }
    }

    // Clef
    staffCount = 0;
    for (i = 0; i < m_ove->getPartCount(); ++i) {
        int partStaffCount = m_ove->getStaffCount(i);
        for (j = 0; j < partStaffCount; ++j) {
            // start clef
            Staff* staff = m_score->staff(staffCount + j);
            if (staff) {
                ovebase::Track* track = m_ove->getTrack(i, j);
                ClefType clefType = OveClefToClef(track->getStartClef());
                Measure* measure = m_score->tick2measure(Fraction(0, 1));
                // staff->setClef(0, clefType);

                // note: also generate symbol for tick 0
                // was not necessary before 0.9.6
                Clef* clef = new Clef(m_score);
                clef->setClefType(clefType);
                clef->setTrack((staffCount + j) * VOICES);

                Segment* s = measure->getSegment(SegmentType::HeaderClef, Fraction(0, 1));
                s->add(clef);
            }

            // clef in measure
            for (k = 0; k < m_ove->getMeasureCount(); ++k) {
                ovebase::MeasureData* measureData = m_ove->getMeasureData(i, j, k);
                QList<ovebase::MusicData*> clefs = measureData->getMusicDatas(ovebase::MusicDataType::Clef);
                Measure* measure = m_score->tick2measure(Fraction::fromTicks(m_mtt->getTick(k, 0)));

                for (int l = 0; l < clefs.size(); ++l) {
                    if (measure != 0) {
                        ovebase::Clef* clefPtr = static_cast<ovebase::Clef*>(clefs[l]);
                        int absTick = m_mtt->getTick(k, clefPtr->getTick());
                        ClefType clefType = OveClefToClef(clefPtr->getClefType());

                        Clef* clef = new Clef(m_score);
                        clef->setClefType(clefType);
                        clef->setTrack((staffCount + j) * VOICES);

                        Segment* s = measure->getSegment(SegmentType::Clef, Fraction::fromTicks(absTick));
                        s->add(clef);
                    }
                }
            }
        }

        staffCount += partStaffCount;
    }

    // Tempo
    std::map<int, double> tempos;
    for (i = 0; i < m_ove->getPartCount(); ++i) {
        int partStaffCount = m_ove->getStaffCount(i);

        for (j = 0; j < partStaffCount; ++j) {
            for (k = 0; k < m_ove->getMeasureCount(); ++k) {
                ovebase::Measure* measure = m_ove->getMeasure(k);
                ovebase::MeasureData* measureData = m_ove->getMeasureData(i, j, k);
                QList<ovebase::MusicData*> tempoPtrs = measureData->getMusicDatas(ovebase::MusicDataType::Tempo);

                if (k == 0
                    || (k > 0 && qAbs(measure->getTypeTempo() - m_ove->getMeasure(k - 1)->getTypeTempo()) > 0.01)) {
                    int tick = m_mtt->getTick(k, 0);
                    tempos[tick] = measure->getTypeTempo();
                }

                for (int l = 0; l < tempoPtrs.size(); ++l) {
                    ovebase::Tempo* ptr = static_cast<ovebase::Tempo*>(tempoPtrs[l]);
                    int tick = m_mtt->getTick(measure->getBarNumber()->getIndex(), ptr->getTick());
                    double tempo = ptr->getQuarterTempo() > 0 ? ptr->getQuarterTempo() : 1.0;

                    tempos[tick] = tempo;
                }
            }
        }
    }

    std::map<int, double>::iterator it;
    int lastTempo = 0;
    for (it=tempos.begin(); it != tempos.end(); ++it) {
        if (it == tempos.begin() || (*it).second != lastTempo) {
            double tpo = ((*it).second) / 60.0;
            m_score->setTempo(Fraction::fromTicks((*it).first), tpo);
        }

        lastTempo = (*it).second;
    }
}

int ContainerToTick(ovebase::NoteContainer* container, int quarter)
{
    int tick = ovebase::NoteTypeToTick(container->getNoteType(), quarter);

    int dotLength = tick;
    for (int i = 0; i < container->getDot(); ++i) {
        dotLength /= 2;
    }
    dotLength = tick - dotLength;

    if (container->getTuplet() > 0) {
        tick = tick * container->getSpace() / container->getTuplet();
    }

    tick += dotLength;

    return tick;
}

const ovebase::Tuplet* getTuplet(const QList<ovebase::MusicData*>& tuplets, int unit)
{
    for (int i = 0; i < tuplets.size(); ++i) {
        const ovebase::MusicData* data = tuplets[i];
        if (unit >= data->start()->getOffset() && unit <= data->stop()->getOffset()) {
            const ovebase::Tuplet* tuplet = static_cast<ovebase::Tuplet*>(tuplets[i]);
            return tuplet;
        }
    }
    return 0;
}

TDuration OveNoteType_To_Duration(ovebase::NoteType noteType)
{
    TDuration d;
    switch (noteType) {
    case ovebase::NoteType::Note_DoubleWhole: {
        d.setType(TDuration::DurationType::V_BREVE);
        break;
    }
    case ovebase::NoteType::Note_Whole: {
        d.setType(TDuration::DurationType::V_WHOLE);
        break;
    }
    case ovebase::NoteType::Note_Half: {
        d.setType(TDuration::DurationType::V_HALF);
        break;
    }
    case ovebase::NoteType::Note_Quarter: {
        d.setType(TDuration::DurationType::V_QUARTER);
        break;
    }
    case ovebase::NoteType::Note_Eight: {
        d.setType(TDuration::DurationType::V_EIGHTH);
        break;
    }
    case ovebase::NoteType::Note_Sixteen: {
        d.setType(TDuration::DurationType::V_16TH);
        break;
    }
    case ovebase::NoteType::Note_32: {
        d.setType(TDuration::DurationType::V_32ND);
        break;
    }
    case ovebase::NoteType::Note_64: {
        d.setType(TDuration::DurationType::V_64TH);
        break;
    }
    case ovebase::NoteType::Note_128: {
        d.setType(TDuration::DurationType::V_128TH);
        break;
    }
    case ovebase::NoteType::Note_256: {
        d.setType(TDuration::DurationType::V_256TH);
        break;
    }
//  case ovebase::NoteType::Note_512: {
//      d.setType(TDuration::DurationType::V_512TH);
//      break;
//  }
//  case ovebase::NoteType::Note_1024: {
//      d.setType(TDuration::DurationType::V_1024TH);
//      break;
//  }
    default:
        d.setType(TDuration::DurationType::V_QUARTER);
        break;
    }

    return d;
}

int accidentalToAlter(ovebase::AccidentalType type)
{
    int alter = 0;

    switch (type) {
    case ovebase::AccidentalType::Normal:
    case ovebase::AccidentalType::Natural:
    case ovebase::AccidentalType::Natural_Caution: {
        alter = 0;
        break;
    }
    case ovebase::AccidentalType::Sharp:
    case ovebase::AccidentalType::Sharp_Caution: {
        alter = 1;
        break;
    }
    case ovebase::AccidentalType::Flat:
    case ovebase::AccidentalType::Flat_Caution: {
        alter = -1;
        break;
    }
    case ovebase::AccidentalType::DoubleSharp:
    case ovebase::AccidentalType::DoubleSharp_Caution: {
        alter = 2;
        break;
    }
    case ovebase::AccidentalType::DoubleFlat:
    case ovebase::AccidentalType::DoubleFlat_Caution: {
        alter = -2;
        break;
    }
    default:
        break;
    }

    return alter;
}

void getMiddleToneOctave(ovebase::ClefType clef, ovebase::ToneType& tone, int& octave)
{
    tone = ovebase::ToneType::B;
    octave = 4;

    switch (clef) {
    case ovebase::ClefType::Treble: {
        tone = ovebase::ToneType::B;
        octave = 4;
        break;
    }
    case ovebase::ClefType::Treble8va: {
        tone = ovebase::ToneType::B;
        octave = 5;
        break;
    }
    case ovebase::ClefType::Treble8vb: {
        tone = ovebase::ToneType::B;
        octave = 3;
        break;
    }
    case ovebase::ClefType::Bass: {
        tone = ovebase::ToneType::D;
        octave = 3;
        break;
    }
    case ovebase::ClefType::Bass8va: {
        tone = ovebase::ToneType::D;
        octave = 4;
        break;
    }
    case ovebase::ClefType::Bass8vb: {
        tone = ovebase::ToneType::D;
        octave = 2;
        break;
    }
    case ovebase::ClefType::Alto: {
        tone = ovebase::ToneType::C;
        octave = 4;
        break;
    }
    case ovebase::ClefType::UpAlto: {
        tone = ovebase::ToneType::A;
        octave = 3;
        break;
    }
    case ovebase::ClefType::DownDownAlto: {
        tone = ovebase::ToneType::G;
        octave = 4;
        break;
    }
    case ovebase::ClefType::DownAlto: {
        tone = ovebase::ToneType::E;
        octave = 4;
        break;
    }
    case ovebase::ClefType::UpUpAlto: {
        tone = ovebase::ToneType::F;
        octave = 3;
        break;
    }
    case ovebase::ClefType::Percussion1: {
        tone = ovebase::ToneType::A;
        octave = 3;
        break;
    }
    case ovebase::ClefType::Percussion2: {
        tone = ovebase::ToneType::A;
        octave = 3;
        break;
    }
    case ovebase::ClefType::TAB: {
        break;
    }
    default:
        break;
    }
}

ovebase::ClefType getClefType(ovebase::MeasureData* measure, int tick)
{
    ovebase::ClefType type = measure->getClef()->getClefType();
    QList<ovebase::MusicData*> clefs = measure->getMusicDatas(ovebase::MusicDataType::Clef);

    for (int i = 0; i < clefs.size(); ++i) {
        if (tick < clefs[i]->getTick()) {
            break;
        }
        if (tick >= clefs[i]->getTick()) {
            ovebase::Clef* clef = static_cast<ovebase::Clef*>(clefs[i]);
            type = clef->getClefType();
        }
    }

    return type;
}

void OveToMScore::convertMeasures()
{
    for (MeasureBase* mb = m_score->measures()->first(); mb; mb = mb->next()) {
        if (mb->type() != ElementType::MEASURE) {
            continue;
        }
        Measure* measure = static_cast<Measure*>(mb);
        int tick = measure->tick().ticks();
        measure->setTicks(m_score->sigmap()->timesig(tick).timesig());
        measure->setTimesig(m_score->sigmap()->timesig(tick).timesig());     //?
        convertMeasure(measure);
    }

    //  convert based on notes
    for (MeasureBase* mb = m_score->measures()->first(); mb; mb = mb->next()) {
        if (mb->type() != ElementType::MEASURE) {
            continue;
        }
        Measure* measure = static_cast<Measure*>(mb);

        convertLines(measure);
    }
}

void OveToMScore::convertMeasure(Measure* measure)
{
    int staffCount = 0;
    int measureCount = m_ove->getMeasureCount();

    for (int i = 0; i < m_ove->getPartCount(); ++i) {
        int partStaffCount = m_ove->getStaffCount(i);

        for (int j = 0; j < partStaffCount; ++j) {
            int measureID = measure->no();

            if (measureID >= 0 && measureID < measureCount) {
                int trackIndex = (staffCount + j) * VOICES;

                convertMeasureMisc(measure, i, j, trackIndex);
                convertNotes(measure, i, j, trackIndex);
                convertLyrics(measure, i, j, trackIndex);
                convertHarmonys(measure, i, j, trackIndex);
                convertRepeats(measure, i, j, trackIndex);
                convertDynamics(measure, i, j, trackIndex);
                convertExpressions(measure, i, j, trackIndex);
            }
        }

        staffCount += partStaffCount;
    }
}

void OveToMScore::convertLines(Measure* measure)
{
    int staffCount = 0;
    int measureCount = m_ove->getMeasureCount();

    for (int i = 0; i < m_ove->getPartCount(); ++i) {
        int partStaffCount = m_ove->getStaffCount(i);

        for (int j = 0; j < partStaffCount; ++j) {
            int measureID = measure->no();

            if (measureID >= 0 && measureID < measureCount) {
                int trackIndex = (staffCount + j) * VOICES;

                convertSlurs(measure, i, j, trackIndex);
                convertGlissandos(measure, i, j, trackIndex);
                convertWedges(measure, i, j, trackIndex);
            }
        }

        staffCount += partStaffCount;
    }
}

void OveToMScore::convertMeasureMisc(Measure* measure, int part, int staff, int track)
{
    ovebase::Measure* measurePtr = m_ove->getMeasure(measure->no());
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, measure->no());
    if (measurePtr == 0 || measureData == 0) {
        return;
    }

    // pickup
    if (measurePtr->getIsPickup()) {
        measure->setIrregular(true);
    }

    // multiple measure rest
    if (measurePtr->getIsMultiMeasureRest()) {
        measure->setBreakMultiMeasureRest(true);
    }

    // barline
    BarLineType bartype = BarLineType::NORMAL;

    switch (measurePtr->getRightBarline()) {
    case ovebase::BarLineType::Default: {
        bartype = BarLineType::NORMAL;
        break;
    }
    case ovebase::BarLineType::Double: {
        bartype = BarLineType::DOUBLE;
        break;
    }
    case ovebase::BarLineType::Final: {
        bartype = BarLineType::END;
        break;
    }
    case ovebase::BarLineType::Null: {
        bartype = BarLineType::NORMAL;
        break;
    }
    case ovebase::BarLineType::RepeatLeft: {
        bartype = BarLineType::START_REPEAT;
        measure->setRepeatStart(true);
        break;
    }
    case ovebase::BarLineType::RepeatRight: {
        bartype = BarLineType::END_REPEAT;
        measure->setRepeatEnd(true);
        break;
    }
    case ovebase::BarLineType::Dashed: {
        bartype = BarLineType::BROKEN;
        break;
    }
    default:
        break;
    }

    if (bartype != BarLineType::NORMAL && bartype != BarLineType::END_REPEAT && bartype != BarLineType::START_REPEAT
        && bartype != BarLineType::END_START_REPEAT && bartype != BarLineType::END) {
        measure->setEndBarLineType(bartype, 0);
    }

    if (bartype == BarLineType::END_REPEAT) {
        measure->setRepeatEnd(true);
    }

    if (measurePtr->getLeftBarline() == ovebase::BarLineType::RepeatLeft) {
        //bartype = BarLineType::START_REPEAT;
        measure->setRepeatStart(true);
    }

    // rehearsal
    int i;
    QList<ovebase::MusicData*> texts = measureData->getMusicDatas(ovebase::MusicDataType::Text);
    for (i = 0; i < texts.size(); ++i) {
        ovebase::Text* textPtr = static_cast<ovebase::Text*>(texts[i]);
        if (textPtr->getTextType() == ovebase::Text::Type::Rehearsal) {
            RehearsalMark* text = new RehearsalMark(m_score);
            text->setPlainText(textPtr->getText());
// TODO:ws  text->setAbove(true);
            text->setTrack(track);

            Segment* s = measure->getSegment(SegmentType::ChordRest,
                                             Fraction::fromTicks(m_mtt->getTick(measure->no(), 0)));
            s->add(text);
        }
    }

    // tempo
    QList<ovebase::MusicData*> tempos = measureData->getMusicDatas(ovebase::MusicDataType::Tempo);
    for (i = 0; i < tempos.size(); ++i) {
        ovebase::Tempo* tempoPtr = static_cast<ovebase::Tempo*>(tempos[i]);
        TempoText* t = new TempoText(m_score);
        int absTick = m_mtt->getTick(measure->no(), tempoPtr->getTick());
        double tpo = (tempoPtr->getQuarterTempo()) / 60.0;

        m_score->setTempo(Fraction::fromTicks(absTick), tpo);

        t->setTempo(tpo);
        QString durationTempoL;
        QString durationTempoR;
        if (static_cast<int>(tempoPtr->getLeftNoteType())) {
            durationTempoL = TempoText::duration2tempoTextString(OveNoteType_To_Duration(tempoPtr->getLeftNoteType()));
        }
        if (static_cast<int>(tempoPtr->getRightNoteType())) {
            durationTempoR = TempoText::duration2tempoTextString(OveNoteType_To_Duration(tempoPtr->getRightNoteType()));
        }
        QString textTempo;
        if (tempoPtr->getShowBeforeText()) {
            textTempo += (tempoPtr->getLeftText()).toHtmlEscaped();
        }
        if (tempoPtr->getShowMark()) {
            if (!textTempo.isEmpty()) {
                textTempo += " ";
            }
            if (tempoPtr->getShowParenthesis()) {
                textTempo += "(";
            }
            textTempo += durationTempoL;
            if (tempoPtr->getLeftNoteDot()) {
                textTempo += "<sym>space</sym><sym>metAugmentationDot</sym>";
            }
            textTempo += " = ";
            switch (tempoPtr->getRightSideType()) {
            case 1:
                textTempo += durationTempoR;
                if (tempoPtr->getRightNoteDot()) {
                    textTempo += "<sym>space</sym><sym>metAugmentationDot</sym>";
                }
                break;
            case 2:
                textTempo += (tempoPtr->getRightText()).toHtmlEscaped();
                break;
            case 3:
                textTempo += QString::number(qFloor(tempoPtr->getTypeTempo()));
                break;
            case 0:
            default:
                textTempo += QString::number(tempoPtr->getTypeTempo());
                break;
            }
            if (tempoPtr->getShowParenthesis()) {
                textTempo += ")";
            }
        }
        if (textTempo.isEmpty()) {
            textTempo = durationTempoL;
            if (tempoPtr->getLeftNoteDot()) {
                textTempo += "<sym>space</sym><sym>metAugmentationDot</sym>";
            }
            textTempo += " = " + QString::number(tempoPtr->getTypeTempo());
            t->setVisible(false);
        }
        t->setXmlText(textTempo);
// TODO:ws  t->setAbove(true);
        t->setTrack(track);

        Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
        s->add(t);
    }
}

// beam in grace
int getGraceLevel(const QList<ovebase::NoteContainer*>& containers, int tick, int unit)
{
    int graceCount = 0;
    int level = 0; // normal chord rest

    for (int i = 0; i < containers.size(); ++i) {
        ovebase::NoteContainer* container = containers[i];
        if (container->getTick() > tick) {
            break;
        }

        if (container->getIsGrace() && container->getTick() == tick) {
            ++graceCount;

            if (unit <= container->start()->getOffset()) {
                ++level;
            }
        }
    }

    return level;
}

bool isRestDefaultLine(ovebase::Note* rest, ovebase::NoteType noteType)
{
    bool isDefault = true;
    switch (noteType) {
    case ovebase::NoteType::Note_DoubleWhole:
    case ovebase::NoteType::Note_Whole:
    case ovebase::NoteType::Note_Half:
    case ovebase::NoteType::Note_Quarter: {
        if (rest->getLine() != 0) {
            isDefault = false;
        }
        break;
    }
    case ovebase::NoteType::Note_Eight: {
        if (rest->getLine() != 1) {
            isDefault = false;
        }
        break;
    }
    case ovebase::NoteType::Note_Sixteen:
    case ovebase::NoteType::Note_32: {
        if (rest->getLine() != -1) {
            isDefault = false;
        }
        break;
    }
    case ovebase::NoteType::Note_64: {
        if (rest->getLine() != -3) {
            isDefault = false;
        }
        break;
    }
    case ovebase::NoteType::Note_128: {
        if (rest->getLine() != -4) {
            isDefault = false;
        }
        break;
    }
    default: {
        break;
    }
    }

    return isDefault;
}

Drumset* getDrumset(Score* score, int part)
{
    Part* p = score->parts().at(part);
    return const_cast<Drumset*>(p->instrument()->drumset()); // TODO: remove cast
}

void OveToMScore::convertNotes(Measure* measure, int part, int staff, int track)
{
    int j;
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, measure->no());
    QList<ovebase::NoteContainer*> containers = measureData->getNoteContainers();
    QList<ovebase::MusicData*> tuplets = measureData->getCrossMeasureElements(ovebase::MusicDataType::Tuplet,
                                                                              ovebase::MeasureData::PairType::Start);
    QList<ovebase::MusicData*> beams = measureData->getCrossMeasureElements(ovebase::MusicDataType::Beam,
                                                                            ovebase::MeasureData::PairType::Start);
    Tuplet* tuplet = 0;
    ChordRest* cr = 0;
    int partStaffCount = m_ove->getStaffCount(part);

    if (containers.empty()) {
        int absTick = m_mtt->getTick(measure->no(), 0);

        cr = new Rest(m_score);
        cr->setTicks(measure->ticks());
        cr->setDurationType(TDuration::DurationType::V_MEASURE);
        cr->setTrack(track);
        Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
        s->add(cr);
    }
    QList<Ms::Chord*> graceNotes;
    for (int i = 0; i < containers.size(); ++i) {
        ovebase::NoteContainer* container = containers[i];
        int tick = m_mtt->getTick(measure->no(), container->getTick());
        int noteTrack = track + container->getVoice();

        if (container->getIsRest()) {
            TDuration duration = OveNoteType_To_Duration(container->getNoteType());
            duration.setDots(container->getDot());

            cr = new Rest(m_score);
            cr->setTicks(duration.fraction());
            cr->setDurationType(duration);
            cr->setTrack(noteTrack);
            cr->setVisible(container->getShow());
            Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(tick));
            s->add(cr);

            QList<ovebase::Note*> notes = container->getNotesRests();
            for (j = 0; j < notes.size(); ++j) {
                ovebase::Note* notePtr = notes[j];
                if (!isRestDefaultLine(notePtr, container->getNoteType()) && notePtr->getLine() != 0) {
                    double yOffset = -(double)(notePtr->getLine());
                    int stepOffset = cr->staff()->staffType(cr->tick())->stepOffset();
                    int lineOffset = static_cast<Ms::Rest*>(cr)->computeLineOffset(5);
                    yOffset -= qreal(lineOffset + stepOffset);
                    yOffset *= m_score->spatium() / 2.0;
                    cr->ryoffset() = yOffset;
                    cr->setAutoplace(false);
                }
            }
        } else {
            QList<ovebase::Note*> notes = container->getNotesRests();

            cr = measure->findChord(Fraction::fromTicks(tick), noteTrack);
            if (cr == 0) {
                cr = new Ms::Chord(m_score);
                cr->setTrack(noteTrack);

                // grace
                if (container->getIsGrace()) {
                    TDuration duration = OveNoteType_To_Duration(container->getGraceNoteType());
                    duration.setDots(container->getDot());
                    ((Ms::Chord*)cr)->setNoteType(NoteType::APPOGGIATURA);

                    if (duration.type() == TDuration::DurationType::V_QUARTER) {
                        ((Ms::Chord*)cr)->setNoteType(NoteType::GRACE4);
                        cr->setDurationType(TDuration::DurationType::V_QUARTER);
                    } else if (duration.type() == TDuration::DurationType::V_16TH) {
                        ((Ms::Chord*)cr)->setNoteType(NoteType::GRACE16);
                        cr->setDurationType(TDuration::DurationType::V_16TH);
                    } else if (duration.type() == TDuration::DurationType::V_32ND) {
                        ((Ms::Chord*)cr)->setNoteType(NoteType::GRACE32);
                        cr->setDurationType(TDuration::DurationType::V_32ND);
                    } else {
                        cr->setDurationType(TDuration::DurationType::V_EIGHTH);
                    }

                    // st = SegmentType::Grace;
                } else {
                    TDuration duration = OveNoteType_To_Duration(container->getNoteType());
                    duration.setDots(container->getDot());

                    if (duration.type() == TDuration::DurationType::V_INVALID) {
                        duration.setType(TDuration::DurationType::V_QUARTER);
                    }
                    cr->setDurationType(duration);
                    // append grace notes before
                    int ii = -1;
                    for (ii = graceNotes.size() - 1; ii >= 0; ii--) {
                        Ms::Chord* gc = graceNotes[ii];
                        if (gc->voice() == cr->voice()) {
                            cr->add(gc);
                        }
                    }
                    graceNotes.clear();
                }
                cr->setTicks(cr->durationType().fraction());

                if (!container->getIsGrace()) {
                    Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(tick));
                    s->add(cr);
                } else {
                    graceNotes.append(static_cast<Ms::Chord*>(cr));
                }
            }

            cr->setVisible(container->getShow());
            cr->setSmall(container->getIsCue());
            for (j = 0; j < notes.size(); ++j) {
                ovebase::Note* oveNote = notes[j];
                Note* note = new Note(m_score);
                int pitch = oveNote->getNote();

                // note->setTrack(noteTrack);
                note->setVeloType(Note::ValueType::USER_VAL);
                note->setVeloOffset(oveNote->getOnVelocity());
                note->setPitch(pitch);

                // tpc
                bool setDirection = false;
                ovebase::ClefType clefType = getClefType(measureData, container->getTick());
                if (clefType == ovebase::ClefType::Percussion1 || clefType == ovebase::ClefType::Percussion2) {
                    Drumset* drumset = getDrumset(m_score, part);
                    if (drumset != 0) {
                        if (!drumset->isValid(pitch) || pitch == -1) {
                            qDebug("unmapped drum note 0x%02x %d", note->pitch(), note->pitch());
                        } else {
                            note->setHeadGroup(drumset->noteHead(pitch));
                            int line = drumset->line(pitch);
                            note->setLine(line);
                            note->setTpcFromPitch();
                            ((Ms::Chord*)cr)->setStemDirection(drumset->stemDirection(pitch));
                            setDirection = true;
                        }
                    } else {
                        // no drumset, we don't allow mid staff percussion
                        note->setTpc(14);
                    }
                } else {
                    const int OCTAVE = 7;
                    ovebase::ToneType clefMiddleTone;
                    int clefMiddleOctave;
                    getMiddleToneOctave(clefType, clefMiddleTone, clefMiddleOctave);
                    int absLine = static_cast<int>(clefMiddleTone) + clefMiddleOctave * OCTAVE + oveNote->getLine();
                    if ((partStaffCount == 2) && oveNote->getOffsetStaff()) {
                        absLine += 2 * (oveNote->getOffsetStaff());
                    }
                    int tone = absLine % OCTAVE;
                    int alter = accidentalToAlter(oveNote->getAccidental());
                    NoteVal nv(pitch);
                    note->setTrack(cr->track());
                    note->setNval(nv, Fraction::fromTicks(tick));
                    // note->setTpcFromPitch();
                    note->setTpc(step2tpc(tone, AccidentalVal(alter)));
                    if (oveNote->getShowAccidental()) {
                        Ms::Accidental* a = new Accidental(m_score);
                        bool bracket = static_cast<int>(oveNote->getAccidental()) & 0x8;
                        AccidentalType at = Ms::AccidentalType::NONE;
                        switch (alter) {
                        case 0: at = Ms::AccidentalType::NATURAL;
                            break;
                        case 1: at = Ms::AccidentalType::SHARP;
                            break;
                        case -1: at = Ms::AccidentalType::FLAT;
                            break;
                        case 2: at = Ms::AccidentalType::SHARP2;
                            break;
                        case -2: at = Ms::AccidentalType::FLAT2;
                            break;
                        }
                        a->setAccidentalType(at);
                        a->setBracket(AccidentalBracket(bracket));
                        a->setRole(Ms::AccidentalRole::USER);
                        note->add(a);
                    }
                    note->setHeadGroup(getHeadGroup(oveNote->getHeadType()));
                }
                if ((oveNote->getHeadType() == ovebase::NoteHeadType::Invisible) || !(oveNote->getShow())) {
                    note->setVisible(false);
                }
                // tie
                if ((int(oveNote->getTiePos()) & int(ovebase::TiePos::LeftEnd)) == int(ovebase::TiePos::LeftEnd)) {
                    Tie* tie = new Tie(m_score);
                    note->setTieFor(tie);
                    tie->setStartNote(note);
                    tie->setTrack(noteTrack);
                }

                // pitch must be set before adding note to chord as note
                // is inserted into pitch sorted list (ws)
                cr->add(note);

                // cr->setVisible(oveNote->getShow());
                ((Ms::Chord*)cr)->setNoStem(int(container->getNoteType()) <= int(ovebase::NoteType::Note_Whole));
                if (!setDirection) {
                    ((Ms::Chord*)cr)->setStemDirection(container->getStemUp() ? Direction::UP : Direction::DOWN);
                }

                // cross staff
                int staffMove = 0;
                if (partStaffCount == 2) { // treble-bass
                    staffMove = oveNote->getOffsetStaff();
                }
                cr->setStaffMove(staffMove);
            }
        }

        // beam
        // Beam::Mode bm = container->getIsRest() ? Beam::Mode::NONE : Beam::Mode::AUTO;
        Beam::Mode bm = Beam::Mode::NONE;
        if (container->getInBeam()) {
            ovebase::MeasurePos pos = container->start()->shiftMeasure(0);
            ovebase::MusicData* data = getCrossMeasureElementByPos(part, staff, pos,
                                                                   container->getVoice(), ovebase::MusicDataType::Beam);

            if (data != 0) {
                ovebase::Beam* beam = static_cast<ovebase::Beam*>(data);
                ovebase::MeasurePos startPos = beam->start()->shiftMeasure(0);
                ovebase::MeasurePos stopPos = beam->stop()->shiftMeasure(beam->start()->getMeasure());

                if (startPos == pos) {
                    bm = Beam::Mode::BEGIN;
                } else if (stopPos == pos) {
                    bm = Beam::Mode::END;
                } else {
                    bm = Beam::Mode::MID;
                }
            }
        }
        cr->setBeamMode(bm);

        // tuplet
        if (container->getTuplet() > 0) {
            if (tuplet == 0) {
                bool create = true;

                // check tuplet start
                if (container->getNoteType() < ovebase::NoteType::Note_Eight) {
                    const ovebase::Tuplet* oveTuplet = getTuplet(tuplets, container->start()->getOffset());
                    if (oveTuplet == 0) {
                        create = false;
                    }
                }

                if (create) {
                    tuplet = new Tuplet(m_score);
                    tuplet->setTrack(noteTrack);
                    tuplet->setRatio(Fraction(container->getTuplet(), container->getSpace()));
                    TDuration duration = OveNoteType_To_Duration(container->getNoteType());
                    tuplet->setBaseLen(duration);
                    // tuplet->setTick(tick);
                    tuplet->setParent(measure);
                    // measure->add(tuplet);
                }
            }

            if (tuplet != 0) {
                cr->setTuplet(tuplet);
                tuplet->add(cr);
            }

            if (tuplet != 0) {
                // check tuplet end
                const ovebase::Tuplet* oveTuplet = getTuplet(tuplets, container->start()->getOffset());
                if (oveTuplet != 0) {
                    // set direction
                    tuplet->setDirection(
                        oveTuplet->getLeftShoulder()->getYOffset() < 0 ? Direction::UP : Direction::DOWN);

                    if (container->start()->getOffset() == oveTuplet->stop()->getOffset()) {
                        tuplet = 0;
                    }
                }
            }
        }

        // articulation
        QList<ovebase::Articulation*> articulations = container->getArticulations();
        for (j = 0; j < articulations.size(); ++j) {
            convertArticulation(measure, cr, noteTrack, tick, articulations[j]);
        }
    }
}

void OveToMScore::convertArticulation(
    Measure* measure, ChordRest* cr,
    int track, int absTick, ovebase::Articulation* art)
{
    switch (art->getArtType()) {
    case ovebase::ArticulationType::Major_Trill:
    case ovebase::ArticulationType::Minor_Trill: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::ornamentTrill);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Trill_Section: {
        break;
    }
    case ovebase::ArticulationType::Inverted_Short_Mordent:
    case ovebase::ArticulationType::Inverted_Long_Mordent: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::ornamentMordent);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Short_Mordent: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::ornamentShortTrill);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Turn: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::ornamentTurn);
        cr->add(a);
        break;
    }
    // case ovebase::ArticulationType::Flat_Accidental_For_Trill:
    // case ovebase::ArticulationType::Sharp_Accidental_For_Trill:
    // case ovebase::ArticulationType::Natural_Accidental_For_Trill:
    case ovebase::ArticulationType::Tremolo_Eighth: {
        Tremolo* t = new Tremolo(m_score);
        t->setTremoloType(TremoloType::R8);
        cr->add(t);
        break;
    }
    case ovebase::ArticulationType::Tremolo_Sixteenth: {
        Tremolo* t = new Tremolo(m_score);
        t->setTremoloType(TremoloType::R16);
        cr->add(t);
        break;
    }
    case ovebase::ArticulationType::Tremolo_Thirty_Second: {
        Tremolo* t = new Tremolo(m_score);
        t->setTremoloType(TremoloType::R32);
        cr->add(t);
        break;
    }
    case ovebase::ArticulationType::Tremolo_Sixty_Fourth: {
        Tremolo* t = new Tremolo(m_score);
        t->setTremoloType(TremoloType::R64);
        cr->add(t);
        break;
    }
    case ovebase::ArticulationType::Marcato: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::articAccentAbove);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Marcato_Dot: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::articAccentAbove);
        cr->add(a);

        a = new Articulation(m_score);
        a->setSymId(SymId::articStaccatoAbove);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Heavy_Attack: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::articAccentAbove);
        cr->add(a);

        a = new Articulation(m_score);
        a->setSymId(SymId::articTenutoAbove);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::SForzando: {
        Articulation* a = new Articulation(m_score);
        a->setUp(true);
        a->setSymId(SymId::articMarcatoAbove);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::SForzando_Inverted: {
        Articulation* a = new Articulation(m_score);
        a->setUp(false);
        a->setSymId(SymId::articMarcatoAbove);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::SForzando_Dot: {
        Articulation* a = new Articulation(m_score);
        a->setUp(true);
        a->setSymId(SymId::articMarcatoAbove);
        cr->add(a);

        a = new Articulation(m_score);
        a->setSymId(SymId::articStaccatoAbove);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::SForzando_Dot_Inverted: {
        Articulation* a = new Articulation(m_score);
        a->setUp(false);
        a->setSymId(SymId::articMarcatoAbove);
        cr->add(a);

        a = new Articulation(m_score);
        a->setSymId(SymId::articStaccatoAbove);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Heavier_Attack: {
        Articulation* a = new Articulation(m_score);
        a->setUp(true);
        a->setSymId(SymId::articMarcatoAbove);
        cr->add(a);

        a = new Articulation(m_score);
        a->setSymId(SymId::articTenutoAbove);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Staccatissimo: {
        Articulation* a = new Articulation(m_score);
        a->setUp(true);
        a->setSymId(SymId::articStaccatissimoAbove);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Staccato: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::articStaccatoAbove);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Tenuto: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::articTenutoAbove);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Pause: {
        Breath* b = new Breath(m_score);
        b->setTrack(track);
        Segment* seg = measure->getSegment(SegmentType::Breath,
                                           Fraction::fromTicks(absTick + (cr ? cr->actualTicks().ticks() : 0)));
        seg->add(b);
        break;
    }
    case ovebase::ArticulationType::Grand_Pause: {
        // TODO?
        break;
    }
    case ovebase::ArticulationType::Up_Bow: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::stringsUpBow);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Down_Bow: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::stringsDownBow);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Up_Bow_Inverted: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::stringsUpBow);
        a->ryoffset() = 5.3;
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Down_Bow_Inverted: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::stringsDownBow);
        a->ryoffset() = 5.3;
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Natural_Harmonic: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::stringsHarmonic);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Artificial_Harmonic: {
        break;
    }
    case ovebase::ArticulationType::Finger_1:
    case ovebase::ArticulationType::Finger_2:
    case ovebase::ArticulationType::Finger_3:
    case ovebase::ArticulationType::Finger_4:
    case ovebase::ArticulationType::Finger_5: {
        break;
    }
    case ovebase::ArticulationType::Plus_Sign: {
        Articulation* a = new Articulation(m_score);
        a->setSymId(SymId::brassMuteClosed);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Arpeggio: {
        // there can be only one
        if (!(static_cast<Ms::Chord*>(cr))->arpeggio()) {
            Arpeggio* a = new Arpeggio(m_score);
            a->setArpeggioType(ArpeggioType::NORMAL);
            /*
            if (art->getPlacementAbove()){
                a->setSubtype(ArpeggioType::UP);
            } else {
                a->setSubtype(ARpeggioType::DOWN);
            }
            */
            cr->add(a);
        }

        break;
    }
    case ovebase::ArticulationType::Fermata: {
        Articulation* a = new Articulation(m_score);
        a->setUp(true);
        a->setSymId(SymId::fermataAbove);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Fermata_Inverted: {
        Articulation* a = new Articulation(m_score);
        a->setDirection(Direction::DOWN);
        a->setSymId(SymId::fermataBelow);
        cr->add(a);
        break;
    }
    case ovebase::ArticulationType::Pedal_Down: {
        if (m_pedal) {
            delete m_pedal;
            m_pedal = 0;
        } else {
            m_pedal = new Pedal(m_score);
            m_pedal->setTrack(track);
            Segment* seg = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
            m_pedal->setTick(seg->tick());
        }
        break;
    }
    case ovebase::ArticulationType::Pedal_Up: {
        if (m_pedal) {
            Segment* seg = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
            m_pedal->setTick2(seg->tick());
            m_score->addSpanner(m_pedal);
            m_pedal = 0;
        }
        break;
    }
    // case ovebase::ArticulationType::Toe_Pedal:
    // case ovebase::ArticulationType::Heel_Pedal:
    // case ovebase::ArticulationType::Toe_To_Heel_Pedal:
    // case ovebase::ArticulationType::Heel_To_Toe_Pedal:
    // case ovebase::ArticulationType::Open_String:
    default:
        break;
    }
}

void OveToMScore::convertLyrics(Measure* measure, int part, int staff, int track)
{
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, measure->no());
    if (measureData == 0) {
        return;
    }

    QList<ovebase::MusicData*> lyrics = measureData->getMusicDatas(ovebase::MusicDataType::Lyric);

    for (int i = 0; i < lyrics.size(); ++i) {
        ovebase::Lyric* oveLyric = static_cast<ovebase::Lyric*>(lyrics[i]);
        int tick = m_mtt->getTick(measure->no(), oveLyric->getTick());

        Lyrics* lyric = new Lyrics(m_score);
        lyric->setNo(oveLyric->getVerse());
        lyric->setPlainText(oveLyric->getLyric());
        lyric->setTrack(track);
        Segment* segment = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(tick));
        if (segment->element(track)) {
            static_cast<ChordRest*>(segment->element(track))->add(lyric);
        }
    }
}

void OveToMScore::convertHarmonys(Measure* measure, int part, int staff, int track)
{
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, measure->no());
    if (measureData == 0) {
        return;
    }

    QList<ovebase::MusicData*> harmonys = measureData->getMusicDatas(ovebase::MusicDataType::Harmony);

    for (int i = 0; i < harmonys.size(); ++i) {
        ovebase::Harmony* harmonyPtr = static_cast<ovebase::Harmony*>(harmonys[i]);
        int absTick = m_mtt->getTick(measure->no(), harmonyPtr->getTick());

        Harmony* harmony = new Harmony(m_score);

        // TODO - does this need to be key-aware?
        harmony->setTrack(track);
        harmony->setRootTpc(step2tpc(harmonyPtr->getRoot(), AccidentalVal(harmonyPtr->getAlterRoot())));
        if (harmonyPtr->getBass() != ovebase::INVALID_NOTE
            && (harmonyPtr->getBass() != harmonyPtr->getRoot()
                || (harmonyPtr->getBass() == harmonyPtr->getRoot()
                    && harmonyPtr->getAlterBass() != harmonyPtr->getAlterRoot()))) {
            harmony->setBaseTpc(step2tpc(harmonyPtr->getBass(), AccidentalVal(harmonyPtr->getAlterBass())));
        }
        const ChordDescription* d = harmony->fromXml(harmonyPtr->getHarmonyType());
        if (d != 0) {
            harmony->setId(d->id);
            harmony->setTextName(d->names.front());
        } else {
            harmony->setId(-1);
            harmony->setTextName(harmonyPtr->getHarmonyType());
        }
        harmony->render();

        Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
        s->add(harmony);
    }
}

/*
ovebase::MusicData* OveToMScore::getMusicDataByUnit(int part, int staff, int measure, int unit, ovebase::MusicDataType type)
{
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, measure);
    if (measureData != 0) {
        const QList<ovebase::MusicData*>& datas = measureData->getMusicDatas(type);
        for (unsigned int i = 0; i < datas.size(); ++i){
            if (datas[i]->getTick() == unit) { // different measurement
                return datas[i];
            }
        }
    }

    return 0;
}
*/

ovebase::MusicData* OveToMScore::getCrossMeasureElementByPos(int part, int staff, const ovebase::MeasurePos& pos, int voice,
                                                             ovebase::MusicDataType type)
{
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, pos.getMeasure());
    if (measureData != 0) {
        const QList<ovebase::MusicData*>& datas
            = measureData->getCrossMeasureElements(type, ovebase::MeasureData::PairType::All);
        for (int i = 0; i < datas.size(); ++i) {
            ovebase::MeasurePos dataStart = datas[i]->start()->shiftMeasure(0);
            ovebase::MeasurePos dataStop = datas[i]->stop()->shiftMeasure(datas[i]->start()->getMeasure());

            if (dataStart <= pos && dataStop >= pos && (int)datas[i]->getVoice() == voice) {
                return datas[i];
            }
        }
    }

    return 0;
}

ovebase::NoteContainer* OveToMScore::getContainerByPos(int part, int staff, const ovebase::MeasurePos& pos)
{
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, pos.getMeasure());
    if (measureData != 0) {
        const QList<ovebase::NoteContainer*>& containers = measureData->getNoteContainers();
        for (int i = 0; i < containers.size(); ++i) {
            if (pos == containers[i]->start()->shiftMeasure(0)) {
                return containers[i];
            }
        }
    }

    return 0;
}

void OveToMScore::convertRepeats(Measure* measure, int part, int staff, int track)
{
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, measure->no());
    if (measureData == 0) {
        return;
    }

    int i;
    QList<ovebase::MusicData*> repeats = measureData->getMusicDatas(ovebase::MusicDataType::Repeat);

    for (i = 0; i < repeats.size(); ++i) {
        ovebase::RepeatSymbol* repeatPtr = static_cast<ovebase::RepeatSymbol*>(repeats[i]);
        ovebase::RepeatType type = repeatPtr->getRepeatType();
        Element* e = 0;

        switch (type) {
        case ovebase::RepeatType::Segno: {
            Marker* marker = new Marker(m_score);
            marker->setMarkerType(Marker::Type::SEGNO);
            e = marker;
            break;
        }
        case ovebase::RepeatType::Coda: {
            Marker* marker = new Marker(m_score);
            marker->setMarkerType(Marker::Type::CODA);
            e = marker;
            break;
        }
        case ovebase::RepeatType::DSAlCoda: {
            Jump* jp = new Jump(m_score);
            jp->setJumpType(Jump::Type::DS_AL_CODA);
            e = jp;
            break;
        }
        case ovebase::RepeatType::DSAlFine: {
            Jump* jp = new Jump(m_score);
            jp->setJumpType(Jump::Type::DS_AL_FINE);
            e = jp;
            break;
        }
        case ovebase::RepeatType::DCAlCoda: {
            Jump* jp = new Jump(m_score);
            jp->setJumpType(Jump::Type::DC_AL_CODA);
            e = jp;
            break;
        }
        case ovebase::RepeatType::DCAlFine: {
            Jump* jp = new Jump(m_score);
            jp->setJumpType(Jump::Type::DC_AL_FINE);
            e = jp;
            break;
        }
        case ovebase::RepeatType::ToCoda: {
            Marker* m = new Marker(m_score);
            m->setMarkerType(Marker::Type::TOCODA);
            e = m;
            break;
        }
        case ovebase::RepeatType::Fine: {
            Marker* m = new Marker(m_score);
            m->setMarkerType(Marker::Type::FINE);
            e = m;
            break;
        }
        default:
            break;
        }

        if (e != 0) {
            e->setTrack(track);
            measure->add(e);
        }
    }

    QList<ovebase::MusicData*> endings = measureData->getCrossMeasureElements(
        ovebase::MusicDataType::Numeric_Ending,
        ovebase::MeasureData::PairType::Start);

    for (i = 0; i < endings.size(); ++i) {
        ovebase::NumericEnding* ending = static_cast<ovebase::NumericEnding*>(endings[i]);
        int absTick1 = m_mtt->getTick(measure->no(), 0);
        int absTick2 = m_mtt->getTick(measure->no() + ending->stop()->getMeasure(), 0);

        if (absTick1 < absTick2) {
            Volta* volta = new Volta(m_score);
            volta->setTrack(track);
            volta->setTick(Fraction::fromTicks(absTick1));
            volta->setTick2(Fraction::fromTicks(absTick2));
            m_score->addElement(volta);
            volta->setVoltaType(Volta::Type::CLOSED);
            volta->setText(ending->getText());

            volta->endings().clear();
            QList<int> numbers = ending->getNumbers();
            for (int j = 0; j < numbers.size(); ++j) {
                volta->endings().append(numbers[j]);
            }
        }
    }
}

void OveToMScore::convertSlurs(Measure* measure, int part, int staff, int track)
{
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, measure->no());
    if (measureData == 0) {
        return;
    }

    QList<ovebase::MusicData*> slurs = measureData->getCrossMeasureElements(ovebase::MusicDataType::Slur,
                                                                            ovebase::MeasureData::PairType::Start);

    for (int i = 0; i < slurs.size(); ++i) {
        ovebase::Slur* slurPtr = static_cast<ovebase::Slur*>(slurs[i]);

        ovebase::NoteContainer* startContainer = getContainerByPos(part, staff, slurPtr->start()->shiftMeasure(0));
        ovebase::NoteContainer* endContainer = getContainerByPos(
            part,
            staff,
            slurPtr->stop()->shiftMeasure(slurPtr->start()->getMeasure()));

        if (startContainer != 0 && endContainer != 0) {
            int absStartTick = m_mtt->getTick(slurPtr->start()->getMeasure(), startContainer->getTick());
            int absEndTick = m_mtt->getTick(
                slurPtr->start()->getMeasure() + slurPtr->stop()->getMeasure(), endContainer->getTick());

            Slur* slur = new Slur(m_score);
            slur->setSlurDirection(slurPtr->getShowOnTop() ? Direction::UP : Direction::DOWN);
            slur->setTick(Fraction::fromTicks(absStartTick));
            slur->setTick2(Fraction::fromTicks(absEndTick));
            slur->setTrack(track);
            slur->setTrack2(track + endContainer->getOffsetStaff());

            m_score->addSpanner(slur);
        }
    }
}

QString OveDynamics_To_Dynamics(ovebase::DynamicsType type)
{
    QString dynamic = "other-dynamics";

    switch (type) {
    case ovebase::DynamicsType::PPPP:
        dynamic = "pppp";
        break;
    case ovebase::DynamicsType::PPP:
        dynamic = "ppp";
        break;
    case ovebase::DynamicsType::PP:
        dynamic = "pp";
        break;
    case ovebase::DynamicsType::P:
        dynamic = "p";
        break;
    case ovebase::DynamicsType::MP:
        dynamic = "mp";
        break;
    case ovebase::DynamicsType::MF:
        dynamic = "mf";
        break;
    case ovebase::DynamicsType::F:
        dynamic = "f";
        break;
    case ovebase::DynamicsType::FF:
        dynamic = "ff";
        break;
    case ovebase::DynamicsType::FFF:
        dynamic = "fff";
        break;
    case ovebase::DynamicsType::FFFF:
        dynamic = "ffff";
        break;
    case ovebase::DynamicsType::SF:
        dynamic = "sf";
        break;
    case ovebase::DynamicsType::FZ:
        dynamic = "fz";
        break;
    case ovebase::DynamicsType::SFZ:
        dynamic = "sfz";
        break;
    case ovebase::DynamicsType::SFFZ:
        dynamic = "sffz";
        break;
    case ovebase::DynamicsType::FP:
        dynamic = "fp";
        break;
    case ovebase::DynamicsType::SFP:
        dynamic = "sfp";
        break;
    default:
        break;
    }

    return dynamic;
}

void OveToMScore::convertDynamics(Measure* measure, int part, int staff, int track)
{
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, measure->no());
    if (measureData == 0) {
        return;
    }

    QList<ovebase::MusicData*> dynamics = measureData->getMusicDatas(ovebase::MusicDataType::Dynamics);

    for (int i = 0; i < dynamics.size(); ++i) {
        ovebase::Dynamics* dynamicPtr = static_cast<ovebase::Dynamics*>(dynamics[i]);
        int absTick = m_mtt->getTick(measure->no(), dynamicPtr->getTick());
        Dynamic* dynamic = new Dynamic(m_score);

        dynamic->setDynamicType(OveDynamics_To_Dynamics(dynamicPtr->getDynamicsType()));
        dynamic->setTrack(track);

        Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
        s->add(dynamic);
    }
}

void OveToMScore::convertExpressions(Measure* measure, int part, int staff, int track)
{
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, measure->no());
    if (measureData == 0) {
        return;
    }

    QList<ovebase::MusicData*> expressions = measureData->getMusicDatas(ovebase::MusicDataType::Expressions);

    for (int i = 0; i < expressions.size(); ++i) {
        ovebase::Expressions* expressionPtr = static_cast<ovebase::Expressions*>(expressions[i]);
        int absTick = m_mtt->getTick(measure->no(), expressionPtr->getTick());
        Text* t = new Text(m_score, Tid::EXPRESSION);

        t->setPlainText(expressionPtr->getText());
        t->setTrack(track);

        Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
        s->add(t);
    }
}

void OveToMScore::convertGlissandos(Measure* measure, int part, int staff, int track)
{
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, measure->no());
    if (measureData == 0) {
        return;
    }

    QList<ovebase::MusicData*> glissandos = measureData->getCrossMeasureElements(ovebase::MusicDataType::Glissando,
                                                                                 ovebase::MeasureData::PairType::All);

    for (int i = 0; i < glissandos.size(); ++i) {
        ovebase::Glissando* glissandoPtr = static_cast<ovebase::Glissando*>(glissandos[i]);
        ovebase::NoteContainer* startContainer = getContainerByPos(part, staff, glissandoPtr->start()->shiftMeasure(0));
        ovebase::NoteContainer* endContainer = getContainerByPos(
            part,
            staff,
            glissandoPtr->stop()->shiftMeasure(glissandoPtr->start()->getMeasure()));

        if (startContainer != 0 && endContainer != 0) {
            int absTick = m_mtt->getTick(measure->no(), glissandoPtr->getTick());
            ChordRest* cr = measure->findChordRest(Fraction::fromTicks(absTick), track);
            if (cr != 0) {
                Glissando* g = new Glissando(m_score);
                g->setGlissandoType(GlissandoType::WAVY);
                cr->add(g);
            }
        }
    }
}

static HairpinType OveWedgeType_To_Type(ovebase::WedgeType type)
{
    HairpinType subtype = HairpinType::CRESC_HAIRPIN;
    switch (type) {
    case ovebase::WedgeType::Cres_Line: {
        subtype = HairpinType::CRESC_HAIRPIN;
        break;
    }
    case ovebase::WedgeType::Double_Line: {
        subtype = HairpinType::CRESC_HAIRPIN;
        break;
    }
    case ovebase::WedgeType::Decresc_Line: {
        subtype = HairpinType::DECRESC_HAIRPIN;
        break;
    }
    case ovebase::WedgeType::Cres: {
        subtype = HairpinType::CRESC_HAIRPIN;
        break;
    }
    case ovebase::WedgeType::Decresc: {
        subtype = HairpinType::DECRESC_HAIRPIN;
        break;
    }
    default:
        break;
    }

    return subtype;
}

void OveToMScore::convertWedges(Measure* measure, int part, int staff, int track)
{
    ovebase::MeasureData* measureData = m_ove->getMeasureData(part, staff, measure->no());
    if (measureData == 0) {
        return;
    }

    QList<ovebase::MusicData*> wedges = measureData->getCrossMeasureElements(ovebase::MusicDataType::Wedge,
                                                                             ovebase::MeasureData::PairType::All);

    for (int i = 0; i < wedges.size(); ++i) {
        ovebase::Wedge* wedgePtr = static_cast<ovebase::Wedge*>(wedges[i]);
        int absTick = m_mtt->getTick(
            measure->no(),
            MeasureToTick::unitToTick(wedgePtr->start()->getOffset(), m_ove->getQuarter()));
        int absTick2 = m_mtt->getTick(
            measure->no() + wedgePtr->stop()->getMeasure(),
            MeasureToTick::unitToTick(wedgePtr->stop()->getOffset(), m_ove->getQuarter()));

        if (absTick2 > absTick) {
            Hairpin* hp = new Hairpin(m_score);

            hp->setHairpinType(OveWedgeType_To_Type(wedgePtr->getWedgeType()));
            // hp->setYoff(wedgePtr->getYOffset());
            hp->setTrack(track);

            hp->setTick(Fraction::fromTicks(absTick));
            hp->setTick2(Fraction::fromTicks(absTick2));
            hp->setAnchor(Spanner::Anchor::SEGMENT);
            m_score->addSpanner(hp);
            m_score->updateHairpin(hp);
        }
    }
}

Score::FileError importOve(MasterScore* score, const QString& name)
{
    ovebase::IOVEStreamLoader* oveLoader = ovebase::createOveStreamLoader();
    ovebase::OveSong oveSong;

    QFile oveFile(name);
    if (!oveFile.exists()) {
        return Score::FileError::FILE_NOT_FOUND;
    }
    if (!oveFile.open(QFile::ReadOnly)) {
        // messageOutString(QString("can't read file!"));
        return Score::FileError::FILE_OPEN_ERROR;
    }

    QByteArray buffer = oveFile.readAll();

    oveFile.close();

    oveSong.setTextCodecName(QString::fromStdString(ove::configuration()->importOvertuneCharset()));
    oveLoader->setOve(&oveSong);
    oveLoader->setFileStream((unsigned char*)buffer.data(), buffer.size());
    bool result = oveLoader->load();
    oveLoader->release();

    if (result) {
        OveToMScore otm;
        otm.convert(&oveSong, score);

        // score->connectSlurs();
    }

    return result ? Score::FileError::FILE_NO_ERROR : Score::FileError::FILE_ERROR;
}
