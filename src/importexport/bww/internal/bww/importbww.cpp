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

// TODO LVI 2011-10-30: determine how to report import errors.
// Currently all output (both debug and error reports) are done using LOGD.

#include <QFile>

#include "lexer.h"
#include "writer.h"
#include "parser.h"

#include "engraving/engravingerrors.h"
#include "engraving/types/fraction.h"

#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/volta.h"

#include "log.h"

using namespace mu::engraving;

namespace Bww {
/**
 The writer that imports into MuseScore.
 */

//---------------------------------------------------------
//   addText
//   copied from importxml.cpp
//   TODO: remove duplicate code
//---------------------------------------------------------

static void addText(mu::engraving::VBox*& vbx, mu::engraving::Score* s, QString strTxt, mu::engraving::TextStyleType stl)
{
    if (!strTxt.isEmpty()) {
        if (vbx == 0) {
            vbx = new mu::engraving::VBox(s->dummy()->system());
        }
        mu::engraving::Text* text = Factory::createText(vbx, stl);
        text->setPlainText(strTxt);
        vbx->add(text);
    }
}

//---------------------------------------------------------
//   xmlSetPitch
//   copied and adapted from importxml.cpp
//   TODO: remove duplicate code
//---------------------------------------------------------

/**
 Convert MusicXML \a step / \a alter / \a octave to midi pitch,
 set pitch and tpc.
 */

static void xmlSetPitch(mu::engraving::Note* n, char step, int alter, int octave)
{
    int istep = step - 'A';
    //                       a  b   c  d  e  f  g
    static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };
    if (istep < 0 || istep > 6) {
        LOGD("xmlSetPitch: illegal pitch %d, <%c>", istep, step);
        return;
    }
    int pitch = table[istep] + alter + (octave + 1) * 12;

    if (pitch < 0) {
        pitch = 0;
    }
    if (pitch > 127) {
        pitch = 127;
    }

    n->setPitch(pitch);

    //                        a  b  c  d  e  f  g
    static int table1[7]  = { 5, 6, 0, 1, 2, 3, 4 };
    int tpc  = step2tpc(table1[istep], mu::engraving::AccidentalVal(alter));
    n->setTpc(tpc);
}

//---------------------------------------------------------
//   setTempo
//   copied and adapted from importxml.cpp
//   TODO: remove duplicate code
//---------------------------------------------------------

static void setTempo(mu::engraving::Score* score, int tempo)
{
    mu::engraving::Measure* measure = score->firstMeasure();
    mu::engraving::Segment* segment = measure->getSegment(mu::engraving::SegmentType::ChordRest, mu::engraving::Fraction(0, 1));
    mu::engraving::TempoText* tt = new mu::engraving::TempoText(segment);
    tt->setTempo(double(tempo) / 60.0);
    tt->setTrack(0);
    tt->setFollowText(true);
    muse::String tempoText = mu::engraving::TempoText::duration2tempoTextString(mu::engraving::DurationType::V_QUARTER);
    tempoText += u" = ";
    tempoText += muse::String::number(tempo);
    tt->setXmlText(tempoText);
    segment->add(tt);
}

class MsScWriter : public Writer
{
public:
    MsScWriter();
    void beginMeasure(const Bww::MeasureBeginFlags mbf);
    void endMeasure(const Bww::MeasureEndFlags mef);
    void header(const QString title, const QString type, const QString composer, const QString footer, const unsigned int temp);
    void note(const QString pitch, const QVector<Bww::BeamType> beamList, const QString type, const int dots, bool tieStart = false,
              bool tieStop = false, StartStop triplet = StartStop::ST_NONE, bool grace = false);
    void setScore(mu::engraving::Score* s) { score = s; }
    void tsig(const int beats, const int beat);
    void trailer();
private:
    void doTriplet(mu::engraving::Chord* cr, StartStop triplet = StartStop::ST_NONE);
    static const int WHOLE_DUR = 64;                    ///< Whole note duration
    struct StepAlterOct {                               ///< MusicXML step/alter/oct values
        QChar s;
        int a;
        int o;
        StepAlterOct(QChar step = QChar('C'), int alter = 0, int oct = 1)
            : s(step), a(alter), o(oct) {}
    };
    mu::engraving::Score* score;                                       ///< The score
    int beats;                                          ///< Number of beats
    int beat;                                           ///< Beat type
    QMap<QString, StepAlterOct> stepAlterOctMap;        ///< Map bww pitch to step/alter/oct
    QMap<QString, QString> typeMap;                     ///< Map bww note types to MusicXML
    unsigned int measureNumber;                         ///< Current measure number
    mu::engraving::Fraction tick;                                      ///< Current tick
    mu::engraving::Measure* currentMeasure;                            ///< Current measure
    mu::engraving::Tuplet* tuplet;                                     ///< Current tuplet
    mu::engraving::Volta* lastVolta;                                   ///< Current volta
    unsigned int tempo;                                 ///< Tempo (0 = not specified)
    unsigned int ending;                                ///< Current ending
    QList<mu::engraving::Chord*> currentGraceNotes;
};

/**
 MsScWriter constructor.
 */

MsScWriter::MsScWriter()
    : score(0),
    beats(4),
    beat(4),
    measureNumber(0),
    tick(mu::engraving::Fraction(0, 1)),
    currentMeasure(0),
    tuplet(0),
    lastVolta(0),
    tempo(0),
    ending(0)
{
    LOGD() << "MsScWriter::MsScWriter()";

    stepAlterOctMap["LG"] = StepAlterOct('G', 0, 4);
    stepAlterOctMap["LA"] = StepAlterOct('A', 0, 4);
    stepAlterOctMap["B"] = StepAlterOct('B', 0, 4);
    stepAlterOctMap["C"] = StepAlterOct('C', 1, 5);
    stepAlterOctMap["D"] = StepAlterOct('D', 0, 5);
    stepAlterOctMap["E"] = StepAlterOct('E', 0, 5);
    stepAlterOctMap["F"] = StepAlterOct('F', 1, 5);
    stepAlterOctMap["HG"] = StepAlterOct('G', 0, 5);
    stepAlterOctMap["HA"] = StepAlterOct('A', 0, 5);

    typeMap["1"] = "whole";
    typeMap["2"] = "half";
    typeMap["4"] = "quarter";
    typeMap["8"] = "eighth";
    typeMap["16"] = "16th";
    typeMap["32"] = "32nd";
}

/**
 Begin a new measure.
 */

void MsScWriter::beginMeasure(const Bww::MeasureBeginFlags mbf)
{
    LOGD() << "MsScWriter::beginMeasure()";
    ++measureNumber;

    // create a new measure
    currentMeasure  = Factory::createMeasure(score->dummy()->system());
    currentMeasure->setTick(tick);
    currentMeasure->setTimesig(mu::engraving::Fraction(beats, beat));
    currentMeasure->setNo(measureNumber);
    score->measures()->add(currentMeasure);

    if (mbf.repeatBegin) {
        currentMeasure->setRepeatStart(true);
    }

    if (mbf.irregular) {
        currentMeasure->setIrregular(true);
    }

    if (mbf.endingFirst || mbf.endingSecond) {
        mu::engraving::Volta* volta = new mu::engraving::Volta(score->dummy());
        volta->setTrack(0);
        volta->endings().clear();
        if (mbf.endingFirst) {
            volta->setText(u"1");
            volta->endings().push_back(1);
            ending = 1;
        } else {
            volta->setText(u"2");
            volta->endings().push_back(2);
            ending = 2;
        }
        volta->setTick(currentMeasure->tick());
        score->addElement(volta);
        lastVolta = volta;
    }

    // set clef, key and time signature in the first measure
    if (measureNumber == 1) {
        // clef
        mu::engraving::Segment* s = currentMeasure->getSegment(mu::engraving::SegmentType::HeaderClef, tick);
        mu::engraving::Clef* clef = Factory::createClef(s);
        clef->setClefType(mu::engraving::ClefType::G);
        clef->setTrack(0);
        s->add(clef);
        // keysig
        mu::engraving::KeySigEvent key;
        key.setConcertKey(mu::engraving::Key::D);
        s = currentMeasure->getSegment(mu::engraving::SegmentType::KeySig, tick);
        mu::engraving::KeySig* keysig = Factory::createKeySig(s);
        keysig->setKeySigEvent(key);
        keysig->setTrack(0);
        s->add(keysig);
        // timesig
        s = currentMeasure->getSegment(mu::engraving::SegmentType::TimeSig, tick);
        mu::engraving::TimeSig* timesig = Factory::createTimeSig(s);
        timesig->setSig(mu::engraving::Fraction(beats, beat));
        timesig->setTrack(0);
        s->add(timesig);
        LOGD("tempo %d", tempo);
    }
}

/**
 End the current measure.
 */

void MsScWriter::endMeasure(const Bww::MeasureEndFlags mef)
{
    LOGD() << "MsScWriter::endMeasure()";
    if (mef.repeatEnd) {
        currentMeasure->setRepeatEnd(true);
    }

    if (mef.endingEnd) {
        if (lastVolta) {
            LOGD("adding volta");
            if (ending == 1) {
                lastVolta->setVoltaType(mu::engraving::Volta::Type::CLOSED);
            } else {
                lastVolta->setVoltaType(mu::engraving::Volta::Type::OPEN);
            }
            lastVolta->setTick2(tick);
            lastVolta = 0;
        } else {
            LOGD("lastVolta == 0 on stop");
        }
    }

    if (mef.lastOfSystem) {
        mu::engraving::LayoutBreak* lb = Factory::createLayoutBreak(currentMeasure);
        lb->setTrack(0);
        lb->setLayoutBreakType(mu::engraving::LayoutBreakType::LINE);
        currentMeasure->add(lb);
    }

    if (mef.lastOfPart && !mef.repeatEnd) {
//TODO            currentMeasure->setEndBarLineType(mu::engraving::BarLineType::END, false, true);
    } else if (mef.doubleBarLine) {
//TODO            currentMeasure->setEndBarLineType(mu::engraving::BarLineType::DOUBLE, false, true);
    }
    // BarLine* barLine = new BarLine(score);
    // bool visible = true;
    // barLine->setSubtype(BarLineType::NORMAL);
    // barLine->setTrack(0);
    // currentMeasure->setEndBarLineType(barLine->subtype(), false, visible);
}

/**
 Write a single note.
 */

void MsScWriter::note(const QString pitch, const QVector<Bww::BeamType> beamList,
                      const QString type, const int dots,
                      bool tieStart, bool /*TODO tieStop */,
                      StartStop triplet,
                      bool grace)
{
    LOGD() << "MsScWriter::note()"
           << "type:" << type
           << "dots:" << dots
           << "grace" << grace
    ;

    if (!stepAlterOctMap.contains(pitch)
        || !typeMap.contains(type)) {
        // TODO: error message
        return;
    }
    StepAlterOct sao = stepAlterOctMap.value(pitch);

    int ticks = 4 * mu::engraving::Constants::DIVISION / type.toInt();
    if (dots) {
        ticks = 3 * ticks / 2;
    }
    LOGD() << "ticks:" << ticks;
    mu::engraving::TDuration durationType(mu::engraving::DurationType::V_INVALID);
    durationType.setVal(ticks);
    if (triplet != StartStop::ST_NONE) {
        ticks = 2 * ticks / 3;
    }

    mu::engraving::BeamMode bm
        = (beamList.at(0) == Bww::BeamType::BM_BEGIN) ? mu::engraving::BeamMode::BEGIN : mu::engraving::BeamMode::AUTO;
    mu::engraving::DirectionV sd = mu::engraving::DirectionV::AUTO;

    // create chord
    mu::engraving::Chord* cr = Factory::createChord(score->dummy()->segment());
    //ws cr->setTick(tick);
    cr->setBeamMode(bm);
    cr->setTrack(0);
    if (grace) {
        cr->setNoteType(mu::engraving::NoteType::GRACE32);
        cr->setDurationType(mu::engraving::DurationType::V_32ND);
        sd = mu::engraving::DirectionV::UP;
    } else {
        if (durationType.type() == mu::engraving::DurationType::V_INVALID) {
            durationType.setType(mu::engraving::DurationType::V_QUARTER);
        }
        cr->setDurationType(durationType);
        sd = mu::engraving::DirectionV::DOWN;
    }
    cr->setTicks(durationType.fraction());
    cr->setDots(dots);
    cr->setStemDirection(sd);
    // add note to chord
    mu::engraving::Note* note = Factory::createNote(cr);
    note->setTrack(0);
    xmlSetPitch(note, sao.s.toLatin1(), sao.a, sao.o);
    if (tieStart) {
        mu::engraving::Tie* tie = new mu::engraving::Tie(score->dummy());
        note->setTieFor(tie);
        tie->setStartNote(note);
        tie->setTrack(0);
    }
    cr->add(note);
    // add chord to measure
    if (!grace) {
        mu::engraving::Segment* s = currentMeasure->getSegment(mu::engraving::SegmentType::ChordRest, tick);
        s->add(cr);
        if (!currentGraceNotes.isEmpty()) {
            for (int i = currentGraceNotes.size() - 1; i >= 0; i--) {
                cr->add(currentGraceNotes.at(i));
            }
            currentGraceNotes.clear();
        }
        doTriplet(cr, triplet);
        mu::engraving::Fraction tickBefore = tick;
        tick += mu::engraving::Fraction::fromTicks(ticks);
        mu::engraving::Fraction nl(tick - currentMeasure->tick());
        currentMeasure->setTicks(nl);
        LOGD() << "MsScWriter::note()"
               << "tickBefore:" << tickBefore.toString()
               << "tick:" << tick.toString()
               << "nl:" << nl.toString()
        ;
    } else {
        currentGraceNotes.append(cr);
    }
}

/**
 Write the header.
 */

void MsScWriter::header(const QString title, const QString type,
                        const QString composer, const QString footer,
                        const unsigned int temp)
{
    LOGD() << "MsScWriter::header()"
           << "title:" << title
           << "type:" << type
           << "composer:" << composer
           << "footer:" << footer
           << "temp:" << temp
    ;

    // save tempo for later use
    tempo = temp;

    if (!title.isEmpty()) {
        score->setMetaTag(u"workTitle", title);
    }
    // TODO re-enable following statement
    // currently disabled because it breaks the bww iotest
    // if (!type.isEmpty()) score->setMetaTag("workNumber", type);
    if (!composer.isEmpty()) {
        score->setMetaTag(u"composer", composer);
    }
    if (!footer.isEmpty()) {
        score->setMetaTag(u"copyright", footer);
    }

    //  score->setWorkTitle(title);
    mu::engraving::VBox* vbox  = 0;
    Bww::addText(vbox, score, title, mu::engraving::TextStyleType::TITLE);
    Bww::addText(vbox, score, type, mu::engraving::TextStyleType::SUBTITLE);
    Bww::addText(vbox, score, composer, mu::engraving::TextStyleType::COMPOSER);
    // addText(vbox, score, strPoet, mu::engraving::TextStyleName::POET);
    // addText(vbox, score, strTranslator, mu::engraving::TextStyleName::TRANSLATOR);
    if (vbox) {
        vbox->setTick(mu::engraving::Fraction(0, 1));
        score->measures()->add(vbox);
    }
    if (!footer.isEmpty()) {
        score->style().set(mu::engraving::Sid::oddFooterC, footer);
    }

    mu::engraving::Part* part = score->staff(0)->part();
    part->setPlainLongName(instrumentName());
    part->setPartName(instrumentName());
    part->instrument()->setTrackName(instrumentName());
    part->setMidiProgram(midiProgram() - 1);
}

/**
 Store beats and beat type for later use.
 */

void MsScWriter::tsig(const int bts, const int bt)
{
    LOGD() << "MsScWriter::tsig()"
           << "beats:" << bts
           << "beat:" << bt
    ;

    beats = bts;
    beat  = bt;
}

/**
 Write the trailer.
 */

void MsScWriter::trailer()
{
    LOGD() << "MsScWriter::trailer()"
    ;

    if (tempo) {
        setTempo(score, tempo);
    }
}

/**
 Handle the triplet.
 */

void MsScWriter::doTriplet(mu::engraving::Chord* cr, StartStop triplet)
{
    LOGD() << "MsScWriter::doTriplet(" << static_cast<int>(triplet) << ")"
    ;

    if (triplet == StartStop::ST_START) {
        tuplet = new mu::engraving::Tuplet(currentMeasure);
        tuplet->setTrack(0);
        tuplet->setRatio(mu::engraving::Fraction(3, 2));
//            tuplet->setTick(tick);
        currentMeasure->add(tuplet);
    } else if (triplet == StartStop::ST_STOP) {
        if (tuplet) {
            cr->setTuplet(tuplet);
            tuplet->add(cr);
            tuplet = 0;
        } else {
            LOGD("BWW::import: triplet stop without triplet start");
        }
    } else if (triplet == StartStop::ST_CONTINUE) {
        if (!tuplet) {
            LOGD("BWW::import: triplet continue without triplet start");
        }
    } else if (triplet == StartStop::ST_NONE) {
        if (tuplet) {
            LOGD("BWW::import: triplet none inside triplet");
        }
    } else {
        LOGD("unknown triplet type %d", static_cast<int>(triplet));
    }
    if (tuplet) {
        cr->setTuplet(tuplet);
        tuplet->add(cr);
    }
}
} // namespace Bww

namespace mu::iex::bww {
//---------------------------------------------------------
//   importBww
//---------------------------------------------------------

Err importBww(MasterScore* score, const QString& path)
{
    LOGD("Score::importBww(%s)", qPrintable(path));

    QFile fp(path);
    if (!fp.exists()) {
        return engraving::Err::FileNotFound;
    }
    if (!fp.open(QIODevice::ReadOnly)) {
        return engraving::Err::FileOpenError;
    }

    Part* part = new Part(score);
    score->appendPart(part);
    Staff* staff = Factory::createStaff(part);
    score->appendStaff(staff);

    Bww::Lexer lex(&fp);
    Bww::MsScWriter wrt;
    wrt.setScore(score);
    score->resetStyleValue(Sid::measureSpacing);
    Bww::Parser p(lex, wrt);
    p.parse();

    score->connectTies();
    LOGD("Score::importBww() done");
    return engraving::Err::NoError; // OK
}
}
