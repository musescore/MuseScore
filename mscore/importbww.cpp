//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: importbww.cpp 5427 2012-03-07 12:41:34Z wschweer $
//
//  Copyright (C) 2010 Werner Schweer and others
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

// TODO LVI 2011-10-30: determine how to report import errors.
// Currently all output (both debug and error reports) are done using qDebug.

#include "bww2mxml/lexer.h"
#include "bww2mxml/writer.h"
#include "bww2mxml/parser.h"

#include "libmscore/barline.h"
#include "libmscore/box.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "libmscore/keysig.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "libmscore/part.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/score.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/staff.h"
#include "libmscore/tempotext.h"
#include "libmscore/timesig.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"
#include "libmscore/segment.h"
#include "musescore.h"
#include "musicxml.h"

//---------------------------------------------------------
//   addText
//   copied from importxml.cpp
//   TODO: remove duplicate code
//---------------------------------------------------------

static void addText(Ms::VBox*& vbx, Ms::Score* s, QString strTxt, Ms::SubStyle stl)
      {
      if (!strTxt.isEmpty()) {
            Ms::Text* text = new Ms::Text(s);
            text->initSubStyle(stl);
            text->setPlainText(strTxt);
            if (vbx == 0)
                  vbx = new Ms::VBox(s);
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

static void xmlSetPitch(Ms::Note* n, char step, int alter, int octave)
      {
      int istep = step - 'A';
      //                       a  b   c  d  e  f  g
      static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };
      if (istep < 0 || istep > 6) {
            qDebug("xmlSetPitch: illegal pitch %d, <%c>", istep, step);
            return;
            }
      int pitch = table[istep] + alter + (octave+1) * 12;

      if (pitch < 0)
            pitch = 0;
      if (pitch > 127)
            pitch = 127;

      n->setPitch(pitch);

      //                        a  b  c  d  e  f  g
      static int table1[7]  = { 5, 6, 0, 1, 2, 3, 4 };
      int tpc  = step2tpc(table1[istep], Ms::AccidentalVal(alter));
      n->setTpc(tpc);
      }

//---------------------------------------------------------
//   setTempo
//   copied and adapted from importxml.cpp
//   TODO: remove duplicate code
//---------------------------------------------------------

static void setTempo(Ms::Score* score, int tempo)
      {
      Ms::TempoText* tt = new Ms::TempoText(score);
      tt->setTempo(double(tempo)/60.0);
      tt->setTrack(0);
      QString tempoText = Ms::TempoText::duration2tempoTextString(Ms::TDuration::DurationType::V_QUARTER);
      tempoText += QString(" = %1").arg(tempo);
      tt->setPlainText(tempoText);
      Ms::Measure* measure = score->firstMeasure();
      Ms::Segment* segment = measure->getSegment(Ms::Segment::Type::ChordRest, 0);
      segment->add(tt);
      }

namespace Bww {

/**
 The writer that imports into MuseScore.
 */

class MsScWriter : public Writer
      {
public:
      MsScWriter();
      void beginMeasure(const Bww::MeasureBeginFlags mbf);
      void endMeasure(const Bww::MeasureEndFlags mef);
      void header(const QString title, const QString type,
                  const QString composer, const QString footer,
                  const unsigned int temp);
      void note(const QString pitch, const QVector<Bww::BeamType> beamList,
                const QString type, const int dots,
                bool tieStart = false, bool tieStop = false,
                StartStop triplet = ST_NONE,
                bool grace = false);
      void setScore(Ms::Score* s) { score = s; }
      void tsig(const int beats, const int beat);
      void trailer();
private:
      void doTriplet(Ms::Chord* cr, StartStop triplet = ST_NONE);
      static const int WHOLE_DUR = 64;                  ///< Whole note duration
      struct StepAlterOct {                             ///< MusicXML step/alter/oct values
            QChar s;
            int a;
            int o;
            StepAlterOct(QChar step = QChar('C'), int alter = 0, int oct = 1)
                  : s(step), a(alter), o(oct) {};
            };
      Ms::Score* score;                                     ///< The score
      int beats;                                        ///< Number of beats
      int beat;                                         ///< Beat type
      QMap<QString, StepAlterOct> stepAlterOctMap;      ///< Map bww pitch to step/alter/oct
      QMap<QString, QString> typeMap;                   ///< Map bww note types to MusicXML
      unsigned int measureNumber;                       ///< Current measure number
      unsigned int tick;                                ///< Current tick
      Ms::Measure* currentMeasure;                          ///< Current measure
      Ms::Tuplet* tuplet;                                   ///< Current tuplet
      Ms::Volta* lastVolta;                                 ///< Current volta
      unsigned int tempo;                               ///< Tempo (0 = not specified)
      unsigned int ending;                              ///< Current ending
      QList<Ms::Chord*> currentGraceNotes;
      };

/**
 MsScWriter constructor.
 */

MsScWriter::MsScWriter()
      : score(0),
      beats(4),
      beat(4),
      measureNumber(0),
      tick(0),
      currentMeasure(0),
      tuplet(0),
      lastVolta(0),
      tempo(0)
      {
      qDebug() << "MsScWriter::MsScWriter()";

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
      qDebug() << "MsScWriter::beginMeasure()";
      ++measureNumber;

      // create a new measure
      currentMeasure  = new Ms::Measure(score);
      currentMeasure->setTick(tick);
      currentMeasure->setTimesig(Ms::Fraction(beats, beat));
      currentMeasure->setNo(measureNumber);
      score->measures()->add(currentMeasure);

      if (mbf.repeatBegin)
            currentMeasure->setRepeatStart(true);

      if (mbf.irregular)
            currentMeasure->setIrregular(true);

      if (mbf.endingFirst || mbf.endingSecond) {
            Ms::Volta* volta = new Ms::Volta(score);
            volta->setTrack(0);
            volta->endings().clear();
            if (mbf.endingFirst) {
                  volta->setText("1");
                  volta->endings().append(1);
                  ending = 1;
                  }
            else {
                  volta->setText("2");
                  volta->endings().append(2);
                  ending = 2;
                  }
            volta->setTick(currentMeasure->tick());
            score->addElement(volta);
            lastVolta = volta;
            }

      // set clef, key and time signature in the first measure
      if (measureNumber == 1) {
            // clef
            Ms::Clef* clef = new Ms::Clef(score);
            clef->setClefType(Ms::ClefType::G);
            clef->setTrack(0);
            Ms::Segment* s = currentMeasure->getSegment(Ms::Segment::Type::Clef, tick);
            s->add(clef);
            // keysig
            Ms::KeySigEvent key;
            key.setKey(Ms::Key::D);
            Ms::KeySig* keysig = new Ms::KeySig(score);
            keysig->setKeySigEvent(key);
            keysig->setTrack(0);
            s = currentMeasure->getSegment(Ms::Segment::Type::KeySig, tick);
            s->add(keysig);
            // timesig
            Ms::TimeSig* timesig = new Ms::TimeSig(score);
            timesig->setSig(Ms::Fraction(beats, beat));
            timesig->setTrack(0);
            s = currentMeasure->getSegment(Ms::Segment::Type::TimeSig, tick);
            s->add(timesig);
            qDebug("tempo %d", tempo);
            }
      }

/**
 End the current measure.
 */

void MsScWriter::endMeasure(const Bww::MeasureEndFlags mef)
      {
      qDebug() << "MsScWriter::endMeasure()";
      if (mef.repeatEnd)
            currentMeasure->setRepeatEnd(true);

      if (mef.endingEnd) {
            if (lastVolta) {
                  qDebug("adding volta");
                  if (ending == 1)
                        lastVolta->setVoltaType(Ms::Volta::Type::CLOSED);
                  else
                        lastVolta->setVoltaType(Ms::Volta::Type::OPEN);
                  lastVolta->setTick2(tick);
                  lastVolta = 0;
                  }
            else {
                  qDebug("lastVolta == 0 on stop");
                  }
            }

      if (mef.lastOfSystem) {
            Ms::LayoutBreak* lb = new Ms::LayoutBreak(score);
            lb->setTrack(0);
            lb->setLayoutBreakType(Ms::LayoutBreak::Type::LINE);
            currentMeasure->add(lb);
            }

      if (mef.lastOfPart && !mef.repeatEnd) {
//TODO            currentMeasure->setEndBarLineType(Ms::BarLineType::END, false, true);
            }
      else if (mef.doubleBarLine) {
//TODO            currentMeasure->setEndBarLineType(Ms::BarLineType::DOUBLE, false, true);
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
      qDebug() << "MsScWriter::note()"
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

      int ticks = 4 * Ms::MScore::division / type.toInt();
      if (dots) ticks = 3 * ticks / 2;
      qDebug() << "ticks:" << ticks;
      Ms::TDuration durationType(Ms::TDuration::DurationType::V_INVALID);
      durationType.setVal(ticks);
      qDebug() << "duration:" << durationType.name();
      if (triplet != ST_NONE) ticks = 2 * ticks / 3;

      Ms::Beam::Mode bm  = (beamList.at(0) == Bww::BM_BEGIN) ? Ms::Beam::Mode::BEGIN : Ms::Beam::Mode::AUTO;
      Ms::Direction sd = Ms::Direction::AUTO;

      // create chord
      Ms::Chord* cr = new Ms::Chord(score);
      //ws cr->setTick(tick);
      cr->setBeamMode(bm);
      cr->setTrack(0);
      if (grace) {
            cr->setNoteType(Ms::NoteType::GRACE32);
            cr->setDurationType(Ms::TDuration::DurationType::V_32ND);
            sd = Ms::Direction::UP;
            }
      else {
            if (durationType.type() == Ms::TDuration::DurationType::V_INVALID)
                  durationType.setType(Ms::TDuration::DurationType::V_QUARTER);
            cr->setDurationType(durationType);
            sd = Ms::Direction::DOWN;
            }
      cr->setDuration(durationType.fraction());
      cr->setDots(dots);
      cr->setStemDirection(sd);
      // add note to chord
      Ms::Note* note = new Ms::Note(score);
      note->setTrack(0);
      xmlSetPitch(note, sao.s.toLatin1(), sao.a, sao.o);
      if (tieStart) {
            Ms::Tie* tie = new Ms::Tie(score);
            note->setTieFor(tie);
            tie->setStartNote(note);
            tie->setTrack(0);
            }
      cr->add(note);
      // add chord to measure
      if (!grace) {
            Ms::Segment* s = currentMeasure->getSegment(Ms::Segment::Type::ChordRest, tick);
            s->add(cr);
            if (!currentGraceNotes.isEmpty()) {
                  for (int i = currentGraceNotes.size() - 1; i >=0; i--)
                        cr->add(currentGraceNotes.at(i));
                  currentGraceNotes.clear();
                  }
            doTriplet(cr, triplet);
            int tickBefore = tick;
            tick += ticks;
            Ms::Fraction nl(Ms::Fraction::fromTicks(tick - currentMeasure->tick()));
            currentMeasure->setLen(nl);
            qDebug() << "MsScWriter::note()"
                     << "tickBefore:" << tickBefore
                     << "tick:" << tick
                     << "nl:" << nl.print()
            ;
            }
      else {
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
      qDebug() << "MsScWriter::header()"
               << "title:" << title
               << "type:" << type
               << "composer:" << composer
               << "footer:" << footer
               << "temp:" << temp
      ;

      // save tempo for later use
      tempo = temp;

      if (!title.isEmpty()) score->setMetaTag("workTitle", title);
      // TODO re-enable following statement
      // currently disabled because it breaks the bww iotest
      // if (!type.isEmpty()) score->setMetaTag("workNumber", type);
      if (!composer.isEmpty()) score->setMetaTag("composer", composer);
      if (!footer.isEmpty()) score->setMetaTag("copyright", footer);

      //  score->setWorkTitle(title);
      Ms::VBox* vbox  = 0;
      addText(vbox, score, title, Ms::SubStyle::TITLE);
      addText(vbox, score, type, Ms::SubStyle::SUBTITLE);
      addText(vbox, score, composer, Ms::SubStyle::COMPOSER);
      // addText(vbox, score, strPoet, Ms::SubStyle::POET);
      // addText(vbox, score, strTranslator, Ms::SubStyle::TRANSLATOR);
      if (vbox) {
            vbox->setTick(0);
            score->measures()->add(vbox);
            }
      if (!footer.isEmpty())
            score->style().set(Ms::StyleIdx::oddFooterC, footer);

      Ms::Part* part = score->staff(0)->part();
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
      qDebug() << "MsScWriter::tsig()"
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
      qDebug() << "MsScWriter::trailer()"
      ;

      if (tempo) setTempo(score, tempo);
      }

/**
 Handle the triplet.
 */

void MsScWriter::doTriplet(Ms::Chord* cr, StartStop triplet)
      {
      qDebug() << "MsScWriter::doTriplet(" << triplet << ")"
      ;

      if (triplet == ST_START) {
            tuplet = new Ms::Tuplet(score);
            tuplet->setTrack(0);
            tuplet->setRatio(Ms::Fraction(3, 2));
            tuplet->setTick(tick);
            currentMeasure->add(tuplet);
            }
      else if (triplet == ST_STOP) {
            if (tuplet) {
                  cr->setTuplet(tuplet);
                  tuplet->add(cr);
                  tuplet = 0;
                  }
            else
                  qDebug("BWW::import: triplet stop without triplet start");
            }
      else if (triplet == ST_CONTINUE) {
            if (!tuplet)
                  qDebug("BWW::import: triplet continue without triplet start");
            }
      else if (triplet == ST_NONE) {
            if (tuplet)
                  qDebug("BWW::import: triplet none inside triplet");
            }
      else
            qDebug("unknown triplet type %d", triplet);
      if (tuplet) {
            cr->setTuplet(tuplet);
            tuplet->add(cr);
            }
      }


} // namespace Bww

namespace Ms {

//---------------------------------------------------------
//   importBww
//---------------------------------------------------------

Score::FileError importBww(MasterScore* score, const QString& path)
      {
      qDebug("Score::importBww(%s)", qPrintable(path));

      QFile fp(path);
      if(!fp.exists())
            return Score::FileError::FILE_NOT_FOUND;
      if (!fp.open(QIODevice::ReadOnly))
            return Score::FileError::FILE_OPEN_ERROR;

      QString id("importBww");
      Part* part = new Part(score);
      part->setId(id);
      score->appendPart(part);
      Staff* staff = new Staff(score);
      staff->setPart(part);
      part->staves()->push_back(staff);
      score->staves().push_back(staff);

      Bww::Lexer lex(&fp);
      Bww::MsScWriter wrt;
      wrt.setScore(score);
      score->style().set(StyleIdx::measureSpacing, 1.0);
      Bww::Parser p(lex, wrt);
      p.parse();

      score->setSaved(false);
      score->setCreated(true);
      score->connectTies();
      qDebug("Score::importBww() done");
      return Score::FileError::FILE_NO_ERROR;      // OK
      }

} // namespace Ms

