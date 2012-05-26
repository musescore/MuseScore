
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

static void addText(VBox*& vbx, Score* s, QString strTxt, int stl)
      {
      if (!strTxt.isEmpty()) {
            Text* text = new Text(s);
            text->setTextStyleType(stl);
            text->setText(strTxt);
            if (vbx == 0)
                  vbx = new VBox(s);
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

static void xmlSetPitch(Note* n, char step, int alter, int octave)
      {
      int istep = step - 'A';
      //                       a  b   c  d  e  f  g
      static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };
      if (istep < 0 || istep > 6) {
            qDebug("xmlSetPitch: illegal pitch %d, <%c>\n", istep, step);
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
      int tpc  = step2tpc(table1[istep], alter);
      n->setTpc(tpc);
      }

//---------------------------------------------------------
//   setTempo
//   copied and adapted from importxml.cpp
//   TODO: remove duplicate code
//---------------------------------------------------------

static void addSymbolToText(const SymCode& s, QTextCursor* cur)
      {
      QTextCharFormat oFormat = cur->charFormat();
      if (s.fontId >= 0) {
            QTextCharFormat oFormat = cur->charFormat();
            QTextCharFormat nFormat(oFormat);
            nFormat.setFontFamily(fontId2font(s.fontId).family());
            cur->setCharFormat(nFormat);
            cur->insertText(QChar(s.code));
            cur->setCharFormat(oFormat);
            }
      else
            cur->insertText(QChar(s.code));
      }

static void setTempo(Score* score, int tempo)
      {
      TempoText* tt = new TempoText(score);
      tt->setTempo(double(tempo)/60.0);
      tt->setTrack(0);

      QTextCursor* c = tt->startCursorEdit();
      c->movePosition(QTextCursor::EndOfLine);
      addSymbolToText(SymCode(0xe105, 1), c);
      c->insertText(" = ");
      c->insertText(QString("%1").arg(tempo));
      tt->endEdit();

      Measure* measure = score->firstMeasure();
      Segment* segment = measure->getSegment(SegChordRest, 0);
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
      void setScore(Score* s) { score = s; }
      void tsig(const int beats, const int beat);
      void trailer();
private:
      void doTriplet(Chord* cr, StartStop triplet = ST_NONE);
      static const int WHOLE_DUR = 64;                  ///< Whole note duration
      struct StepAlterOct {                             ///< MusicXML step/alter/oct values
            QChar s;
            int a;
            int o;
            StepAlterOct(QChar step = 'C', int alter = 0, int oct = 1)
                  : s(step), a(alter), o(oct) {};
            };
      Score* score;                                     ///< The score
      int beats;                                        ///< Number of beats
      int beat;                                         ///< Beat type
      QMap<QString, StepAlterOct> stepAlterOctMap;      ///< Map bww pitch to step/alter/oct
      QMap<QString, QString> typeMap;                   ///< Map bww note types to MusicXML
      unsigned int measureNumber;                       ///< Current measure number
      unsigned int tick;                                ///< Current tick
      Measure* currentMeasure;                          ///< Current measure
      Tuplet* tuplet;                                   ///< Current tuplet
      Volta* lastVolta;                                 ///< Current volta
      unsigned int tempo;                               ///< Tempo (0 = not specified)
      unsigned int ending;                              ///< Current ending
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
      currentMeasure  = new Measure(score);
      currentMeasure->setTick(tick);
      currentMeasure->setTimesig(Fraction(beats, beat));
      currentMeasure->setNo(measureNumber);
      score->measures()->add(currentMeasure);

      if (mbf.repeatBegin)
            currentMeasure->setRepeatFlags(RepeatStart);

      if (mbf.irregular)
            currentMeasure->setIrregular(true);

      if (mbf.endingFirst || mbf.endingSecond) {
            Volta* volta = new Volta(score);
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
            volta->setStartElement(currentMeasure);
            currentMeasure->add(volta);
            lastVolta = volta;
            }

      // set clef, key and time signature in the first measure
      if (measureNumber == 1) {
            // clef
            Clef* clef = new Clef(score);
            clef->setClefType(CLEF_G);
            clef->setTrack(0);
            Segment* s = currentMeasure->getSegment(clef, tick);
            s->add(clef);
            // keysig
            KeySigEvent key;
            key.setAccidentalType(2);
            KeySig* keysig = new KeySig(score);
            keysig->setKeySigEvent(key);
            keysig->setTrack(0);
            s = currentMeasure->getSegment(keysig, tick);
            s->add(keysig);
            // timesig
            TimeSig* timesig = new TimeSig(score);
            timesig->setSig(Fraction(beats, beat));
            timesig->setTrack(0);
            s = currentMeasure->getSegment(timesig, tick);
            s->add(timesig);
            }
      }

/**
 End the current measure.
 */

void MsScWriter::endMeasure(const Bww::MeasureEndFlags mef)
      {
      qDebug() << "MsScWriter::endMeasure()";
      if (mef.repeatEnd)
            currentMeasure->setRepeatFlags(RepeatEnd);

      if (mef.endingEnd) {
            if (lastVolta) {
                  qDebug("adding volta\n");
                  if (ending == 1)
                        lastVolta->setSubtype(VOLTA_CLOSED);
                  else
                        lastVolta->setSubtype(VOLTA_OPEN);
                  lastVolta->setEndElement(currentMeasure);
                  currentMeasure->addSpannerBack(lastVolta);
                  lastVolta = 0;
                  }
            else {
                  qDebug("lastVolta == 0 on stop\n");
                  }
            }

      if (mef.lastOfSystem) {
            LayoutBreak* lb = new LayoutBreak(score);
            lb->setTrack(0);
            lb->setSubtype(LAYOUT_BREAK_LINE);
            currentMeasure->add(lb);
            }

      if (mef.lastOfPart && !mef.repeatEnd) {
            currentMeasure->setEndBarLineType(END_BAR, false, true);
            }
      else if (mef.doubleBarLine) {
            currentMeasure->setEndBarLineType(DOUBLE_BAR, false, true);
            }
      // BarLine* barLine = new BarLine(score);
      // bool visible = true;
      // barLine->setSubtype(NORMAL_BAR);
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

      int ticks = 4 * MScore::division / type.toInt();
      if (dots) ticks = 3 * ticks / 2;
      qDebug() << "ticks:" << ticks;
      TDuration durationType(TDuration::V_INVALID);
      durationType.setVal(ticks);
      qDebug() << "duration:" << durationType.name();
      if (triplet != ST_NONE) ticks = 2 * ticks / 3;

      BeamMode bm  = (beamList.at(0) == Bww::BM_BEGIN) ? BEAM_BEGIN : BEAM_AUTO;
      Direction sd = AUTO;

      // create chord
      Chord* cr = new Chord(score);
      //ws cr->setTick(tick);
      cr->setBeamMode(bm);
      cr->setTrack(0);
      if (grace) {
            cr->setNoteType(NOTE_GRACE32);
            cr->setDurationType(TDuration::V_32ND);
            sd = UP;
            }
      else {
            if (durationType.type() == TDuration::V_INVALID)
                  durationType.setType(TDuration::V_QUARTER);
            cr->setDurationType(durationType);
            sd = DOWN;
            }
      cr->setDuration(durationType.fraction());
      cr->setDots(dots);
      cr->setStemDirection(sd);
      // add note to chord
      Note* note = new Note(score);
      note->setTrack(0);
      xmlSetPitch(note, sao.s.toAscii(), sao.a, sao.o);
      if (tieStart) {
            Tie* tie = new Tie(score);
            note->setTieFor(tie);
            tie->setStartNote(note);
            tie->setTrack(0);
            }
      cr->add(note);
      // add chord to measure
      Segment* s = currentMeasure->getSegment(cr, tick);
      s->add(cr);
      if (!grace) {
            doTriplet(cr, triplet);
            int tickBefore = tick;
            tick += ticks;
            Fraction nl(Fraction::fromTicks(tick - currentMeasure->tick()));
            currentMeasure->setLen(nl);
            qDebug() << "MsScWriter::note()"
                     << "tickBefore:" << tickBefore
                     << "tick:" << tick
                     << "nl:" << nl.print()
            ;
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
      QString strType = "composer";
      QString strComposer = composer; // TODO: const parameters ctor MusicXmlCreator
      score->addCreator(new MusicXmlCreator(strType, strComposer));
      if (!footer.isEmpty()) score->setMetaTag("copyright", footer);

      //  score->setWorkTitle(title);
      VBox* vbox  = 0;
      addText(vbox, score, title, TEXT_STYLE_TITLE);
      addText(vbox, score, type, TEXT_STYLE_SUBTITLE);
      addText(vbox, score, composer, TEXT_STYLE_COMPOSER);
      // addText(vbox, score, strPoet, TEXT_STYLE_POET);
      // addText(vbox, score, strTranslator, TEXT_STYLE_TRANSLATOR);
      if (vbox) {
            vbox->setTick(0);
            score->measures()->add(vbox);
            }
      if (!footer.isEmpty())
            score->style()->set(ST_oddFooterC, footer);

      Part* part = score->part(0);
      part->setLongName(instrumentName());
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

void MsScWriter::doTriplet(Chord* cr, StartStop triplet)
      {
      qDebug() << "MsScWriter::doTriplet(" << triplet << ")"
      ;

      if (triplet == ST_START) {
            tuplet = new Tuplet(score);
            tuplet->setTrack(0);
            tuplet->setRatio(Fraction(3, 2));
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
                  qDebug("BWW::import: triplet stop without triplet start\n");
            }
      else if (triplet == ST_CONTINUE) {
            if (!tuplet)
                  qDebug("BWW::import: triplet continue without triplet start\n");
            }
      else if (triplet == ST_NONE) {
            if (tuplet)
                  qDebug("BWW::import: triplet none inside triplet\n");
            }
      else
            qDebug("unknown triplet type %d\n", triplet);
      if (tuplet) {
            cr->setTuplet(tuplet);
            tuplet->add(cr);
            }
      }


} // namespace Bww

//---------------------------------------------------------
//   importBww
//---------------------------------------------------------

bool MuseScore::importBww(Score* score, const QString& path)
      {
      qDebug("Score::importBww(%s)\n", qPrintable(path));

      if (path.isEmpty())
            return false;
      QFile fp(path);
      if (!fp.open(QIODevice::ReadOnly))
            return false;

      QString id("importBww");
      Part* part = new Part(score);
      part->setId(id);
      score->appendPart(part);
      Staff* staff = new Staff(score, part, 0);
      part->staves()->push_back(staff);
      score->staves().push_back(staff);

      Bww::Lexer lex(&fp);
      Bww::MsScWriter wrt;
      wrt.setScore(score);
      Bww::Parser p(lex, wrt);
      p.parse();

      score->setSaved(false);
      score->setCreated(true);
      score->connectTies();
      qDebug("Score::importBww() done\n");
      return true;      // OK
      }
