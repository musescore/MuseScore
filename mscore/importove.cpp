//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "importove.h"

#include "globals.h"
//#include "musescore.h"
#include "libmscore/sig.h"
#include "libmscore/tempo.h"
#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/box.h"
#include "libmscore/bracket.h"
#include "libmscore/breath.h"
#include "libmscore/clef.h"
#include "libmscore/drumset.h"
#include "libmscore/dynamic.h"
#include "libmscore/hairpin.h"
#include "libmscore/harmony.h"
#include "libmscore/glissando.h"
#include "libmscore/keysig.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/lyrics.h"
#include "libmscore/note.h"
#include "libmscore/accidental.h"
#include "libmscore/ottava.h"
#include "libmscore/pitchspelling.h"
#include "preferences.h"
#include "libmscore/repeat.h"
#include "libmscore/rest.h"
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
#include "libmscore/sym.h"
#include "libmscore/bracketItem.h"

using namespace Ms;

//---------------------------------------------------------
//   MeasureToTick
//---------------------------------------------------------

MeasureToTick::MeasureToTick()
      {
      _quarter = 480;
      _ove     = nullptr;
      }

//---------------------------------------------------------
//   measureTick
//---------------------------------------------------------

int measureTick(int quarter, int num, int den)
      {
      return quarter * 4 * num / den;
      }

//---------------------------------------------------------
//   MeasureToTick::build
//---------------------------------------------------------

void MeasureToTick::build(Ove::OveSong* ove, int quarter)
      {
      unsigned i;
      int currentTick = 0;
      unsigned measureCount = ove->measureCount();

      _quarter = quarter;
      _ove = ove;
      _tts.clear();

      for (i = 0; i < measureCount; ++i) {
            Ove::Measure* measure    = _ove->measure(i);
            Ove::TimeSignature* time = measure->timeSig();
            TimeTick tt;
            bool change = false;

            tt._tick        = currentTick;
            tt._numerator   = time->numerator();
            tt._denominator = time->denominator();
            tt._measure     = i;
            tt._isSymbol    = time->isSymbol();

            if (i == 0) {
                  change = true;
                  } 
            else {
                  Ove::TimeSignature* previousTime = _ove->measure(i - 1)->timeSig();

                  if (time->numerator() != previousTime->numerator() ||
                     time->denominator() != previousTime->denominator())
                        change = true;
                  else if (time->isSymbol() != previousTime->isSymbol())
                        change = true;
                  }

            if (change)
                  _tts.push_back(tt);

            currentTick += measureTick(_quarter, tt._numerator, tt._denominator);
            }
      }

//---------------------------------------------------------
//   MeasureToTick::tick
//---------------------------------------------------------

int MeasureToTick::tick(int measure, int tickPos)
      {
      TimeTick tt;

      for (int i = 0; i < _tts.size(); ++i) {
            if (measure >= _tts[i]._measure && (i == _tts.size() - 1 || measure < _tts[i + 1]._measure)) {
                  int measuresTick = (measure - _tts[i]._measure) * measureTick(_quarter, _tts[i]._numerator, _tts[i]._denominator);
                  return _tts[i]._tick + measuresTick + tickPos;
                  }
            }

      return 0;
      }

//---------------------------------------------------------
//   MeasureToTick::unitToTick
//---------------------------------------------------------

int MeasureToTick::unitToTick(int unit, int quarter)
      {
      // 0x100 correspond to quarter tick
      float ratio = float(unit) / float(256.0);
      int tick    = ratio * quarter;
      return tick;
      }

//---------------------------------------------------------
//   OveToMScore
//---------------------------------------------------------

OveToMScore::OveToMScore()
      {
      _ove   = nullptr;
      _mtt   = new MeasureToTick();
      _pedal = nullptr;
      }

OveToMScore::~OveToMScore()
      {
      delete _mtt;
      }

//---------------------------------------------------------
//   OveToMScore::convert
//---------------------------------------------------------

void OveToMScore::convert(Ove::OveSong* ove, Score* score)
      {
      _ove = ove;
      _score = score;
      _mtt->build(_ove, _ove->isQuarter());

      convertHeader();
      createStructure();
      convertGroups();
      convertSignature();
      // convertLineBreak();

      int staffCount = 0;
      for (int i = 0; i < _ove->partCount(); ++i) {
            int partStaffCount = _ove->staffCount(i);
            Part* part = _score->parts().at(i);

            for (int j = 0; j < partStaffCount; ++j) {
                  Ove::Track* track = _ove->track(i, j);

                  convertTrackHeader(track, part);
                  }

            staffCount += partStaffCount;
            }

      convertMeasures();

      // convert elements by ove track sequence
      staffCount = 0;
      for (int i = 0; i < _ove->partCount(); ++i) {
            int partStaffCount = _ove->staffCount(i);

            for (int j = 0; j < partStaffCount; ++j) {
                  int trackIndex = staffCount + j;

                  convertTrackElements(trackIndex);
                  }

            staffCount += partStaffCount;
            }

      clearUp();
      }

//---------------------------------------------------------
//   OveToMScore::createStructure
//---------------------------------------------------------

void OveToMScore::createStructure()
      {
      int i;
      for (i = 0; i < _ove->partCount(); ++i) {
            int partStaffCount = _ove->staffCount(i);
            Part* part = new Part(_score);

            for (int j = 0; j < partStaffCount; ++j) {
                  // Ove::Track* track = _ove->track(i, j);
                  Staff* staff = new Staff(_score);
                  staff->setPart(part);

                  part->staves()->push_back(staff);
                  _score->staves().push_back(staff);
                  }

            _score->appendPart(part);
            part->setStaves(partStaffCount);
            }

      for (i = 0; i < _ove->measureCount(); ++i) {
            Measure* measure  = new Measure(_score);
            int tick = _mtt->tick(i, 0);
            measure->setTick(Fraction::fromTicks(tick));
            measure->setNo(i);
            _score->measures()->add(measure);
            }
      }

//---------------------------------------------------------
//   OveToMScore::clearUp
//---------------------------------------------------------

void OveToMScore::clearUp()
      {
      if (_pedal) {
            delete _pedal;
            _pedal = nullptr;
            }
      }

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

static Ove::Staff* staff(const Ove::OveSong* ove, int track)
      {
      if (ove->lineCount() > 0) {
            Ove::Line* line = ove->line(0);
            if (line && line->staffCount() > 0) {
                  Ove::Staff* staff = line->staff(track);
                  return staff;
                  }
            }

      return nullptr;
      }

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

static void addText(VBox* & vbox, Score* s, QString strTxt, Tid stl)
      {
      if (!strTxt.isEmpty()) {
            Text* text = new Text(s, stl);
            text->setPlainText(strTxt);
            if (!vbox)
                  vbox = new VBox(s);
            vbox->add(text);
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertHeader
//---------------------------------------------------------

void OveToMScore::convertHeader()
      {
      VBox* vbox = nullptr;
      QList<QString> titles = _ove->titles();
      if (!titles.empty() && !titles[0].isEmpty()) {
            QString title = titles[0];
            _score->setMetaTag("movementTitle", title);
            addText(vbox, _score, title, Tid::TITLE);
            }

      QList<QString> copyrights = _ove->copyrights();
      if (!copyrights.empty() && !copyrights[0].isEmpty()) {
            QString copyright = copyrights[0];
            _score->setMetaTag("copyright", copyright);
            }

      QList<QString> annotates = _ove->annotates();
      if (!annotates.empty() && !annotates[0].isEmpty()) {
            QString annotate = annotates[0];
            addText(vbox, _score, annotate, Tid::POET);
            }

      QList<QString> writers = _ove->writers();
      if (!writers.empty()) {
            QString composer = writers[0];
            _score->setMetaTag("composer", composer);
            addText(vbox, _score, composer, Tid::COMPOSER);
            }

      if (writers.size() > 1) {
            QString lyricist = writers[1];
            addText(vbox, _score, lyricist, Tid::POET);
            }

      if (vbox) {
            vbox->setTick(Fraction(0,1));
            _score->measures()->add(vbox);
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertGroups
//---------------------------------------------------------

void OveToMScore::convertGroups()
      {
      int i;
      int staffCount = 0;
      const QList<Part*>& parts = _score->parts();
      for (i = 0; i < _ove->partCount(); ++i) {
            int partStaffCount = _ove->staffCount(i);
            Part* part = parts.at(i);
            if (!part)
                  continue;

            for (int j = 0; j < partStaffCount; ++j) {
                  int staffIndex = staffCount + j;
                  Staff* s = _score->staff(staffIndex);
                  if (!s)
                        continue;

                  // brace
                  if (j == 0 && partStaffCount == 2) {
                        s->setBracketType(0, BracketType::BRACE);
                        s->setBracketSpan(0, 2);
                        s->setBarLineSpan(2);
                        }

                  // bracket
                  Ove::Staff* staffPtr = staff(_ove, staffIndex);
                  if (staffPtr && staffPtr->groupType() == Ove::GroupType::Brackets) {
                        int span = staffPtr->groupStaffCount() + 1;
                        int endStaff = staffIndex + span;
                        if (span > 0 && endStaff >= staffIndex && endStaff <= _ove->trackCount()) {
                              s->addBracket(new BracketItem(s->score(), BracketType::NORMAL, span));
                              s->setBarLineSpan(span);
                              }
                        }
                  }

            staffCount += partStaffCount;
            }
      }

//---------------------------------------------------------
//   oveClefToClef
//---------------------------------------------------------

static ClefType oveClefToClef(Ove::ClefType type)
      {
      ClefType clef = ClefType::G;
      switch (type) {
            case Ove::ClefType::Treble:
                  clef = ClefType::G;
                  break;
            case Ove::ClefType::Bass:
                  clef = ClefType::F;
                  break;
            case Ove::ClefType::Alto:
                  clef = ClefType::C3;
                  break;
            case Ove::ClefType::UpAlto:
                  clef = ClefType::C4;
                  break;
            case Ove::ClefType::DownDownAlto:
                  clef = ClefType::C1;
                  break;
            case Ove::ClefType::DownAlto:
                  clef = ClefType::C2;
                  break;
            case Ove::ClefType::UpUpAlto:
                  clef = ClefType::C5;
                  break;
            case Ove::ClefType::Treble8va:
                  clef = ClefType::G8_VA;
                  break;
            case Ove::ClefType::Bass8va:
                  clef = ClefType::F_8VA;
                  break;
            case Ove::ClefType::Treble8vb:
                  clef = ClefType::G8_VB;
                  break;
            case Ove::ClefType::Bass8vb:
                  clef = ClefType::F8_VB;
                  break;
            case Ove::ClefType::Percussion1:
                  clef = ClefType::PERC;
                  break;
            case Ove::ClefType::Percussion2:
                  clef = ClefType::PERC2;
                  break;
            case Ove::ClefType::TAB:
                  clef = ClefType::TAB;
                  break;
            default:
                  break;
            }
      return clef;
      }

//---------------------------------------------------------
//   headGroup
//---------------------------------------------------------

static NoteHead::Group headGroup(Ove::NoteHeadType type)
      {
      NoteHead::Group headGroup = NoteHead::Group::HEAD_NORMAL;
      switch (type) {
            case Ove::NoteHeadType::Standard:
                  headGroup = NoteHead::Group::HEAD_NORMAL;
                  break;
            case Ove::NoteHeadType::Invisible: 
                  break;
            case Ove::NoteHeadType::Rhythmic_Slash:
                  headGroup = NoteHead::Group::HEAD_SLASH;
                  break;
            case Ove::NoteHeadType::Percussion:
                  headGroup = NoteHead::Group::HEAD_XCIRCLE;
                  break;
            case Ove::NoteHeadType::Closed_Rhythm:
            case Ove::NoteHeadType::Open_Rhythm:
                  headGroup = NoteHead::Group::HEAD_CROSS;
                  break;
            case Ove::NoteHeadType::Open_Slash:
                  headGroup = NoteHead::Group::HEAD_SLASH;
                  break;
            case Ove::NoteHeadType::Closed_Do:
            case Ove::NoteHeadType::Open_Do:
                  headGroup = NoteHead::Group::HEAD_DO;
                  break;
            case Ove::NoteHeadType::Closed_Re:
            case Ove::NoteHeadType::Open_Re:
                  headGroup = NoteHead::Group::HEAD_RE;
                  break;
            case Ove::NoteHeadType::Closed_Mi:
            case Ove::NoteHeadType::Open_Mi:
                  headGroup = NoteHead::Group::HEAD_MI;
                  break;
            case Ove::NoteHeadType::Closed_Fa:
            case Ove::NoteHeadType::Open_Fa:
                  headGroup = NoteHead::Group::HEAD_FA;
                  break;
            case Ove::NoteHeadType::Closed_Sol:
            case Ove::NoteHeadType::Open_Sol:
                  headGroup = NoteHead::Group::HEAD_SOL;
                  break;
            case Ove::NoteHeadType::Closed_La:
            case Ove::NoteHeadType::Open_La:
                  headGroup = NoteHead::Group::HEAD_LA;
                  break;
            case Ove::NoteHeadType::Closed_Ti:
            case Ove::NoteHeadType::Open_Ti:
                  headGroup = NoteHead::Group::HEAD_TI;
                  break;
            default:
                  break;
            }

      return headGroup;
      }

//---------------------------------------------------------
//   OvetToMScore::convertTrackHeader
//---------------------------------------------------------

void OveToMScore::convertTrackHeader(Ove::Track* track, Part* part)
      {
      if (!track || !part)
            return;

      QString longName = track->name();
      if (longName != QString() && track->showName())
            part->setPlainLongName(longName);

      QString shortName = track->briefName();
      if (shortName != QString() && track->showBriefName())
            part->setPlainShortName(shortName);

      part->setMidiProgram(track->patch());

      if (_ove->showTransposeTrack() && track->transpose() != 0) {
            Ms::Interval interval = part->instrument()->transpose();
            interval.diatonic = -track->transpose();
            part->instrument()->setTranspose(interval);
            }

      // DrumSet
      if (track->startClef() == Ove::ClefType::Percussion1
         || track->startClef() == Ove::ClefType::Percussion2) {
            // use overture drumset
            Drumset* drumset = new Drumset();
            for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
                  drumset->drum(i).name     = smDrumset->drum(i).name;
                  drumset->drum(i).notehead = smDrumset->drum(i).notehead;
                  drumset->drum(i).line     = smDrumset->drum(i).line;
                  drumset->drum(i).stemDirection = smDrumset->drum(i).stemDirection;
                  drumset->drum(i).voice    = smDrumset->drum(i).voice;
                  drumset->drum(i).shortcut = 0;
                  }
            QList<Ove::Track::DrumNode> nodes = track->drumKit();
            for (int i = 0; i < nodes.size(); ++i) {
                  int pitch = nodes[i]._pitch;
                  Ove::Track::DrumNode node = nodes[i];
                  if (pitch < DRUM_INSTRUMENTS) {
                        drumset->drum(pitch).line = node._line;
                        drumset->drum(pitch).notehead = headGroup(Ove::NoteHeadType(node._headType));
                        drumset->drum(pitch).voice = node._voice;
                        }
                  }

            part->instrument()->channel(0)->setBank(128);
            part->setMidiProgram(0);
            part->instrument()->setDrumset(smDrumset);
            part->instrument()->setDrumset(drumset);
            }
      }

//---------------------------------------------------------
//   octaveShiftTypeToInt
//---------------------------------------------------------

static OttavaType octaveShiftTypeToInt(Ove::OctaveShiftType type)
      {
      OttavaType subtype = OttavaType::OTTAVA_8VA;
      switch (type) {
            case Ove::OctaveShiftType::OS_8:
                  subtype = OttavaType::OTTAVA_8VA;
                  break;
            case Ove::OctaveShiftType::OS_15:
                  subtype = OttavaType::OTTAVA_15MA;
                  break;
            case Ove::OctaveShiftType::OS_Minus_8:
                  subtype = OttavaType::OTTAVA_8VB;
                  break;
            case Ove::OctaveShiftType::OS_Minus_15:
                  subtype = OttavaType::OTTAVA_15MB;
                  break;
            default:
                  break;
            }

      return subtype;
      }

//---------------------------------------------------------
//   OveToMScore::convertTrackElements
//---------------------------------------------------------

void OveToMScore::convertTrackElements(int track)
      {
      Ottava* ottava = nullptr;

      for (int i = 0; i < _ove->trackBarCount(); ++i) {
            Ove::MeasureData* measureData = _ove->measureData(track, i);
            if (!measureData)
                  continue;

            // octave shift
            QList<Ove::MusicData*> octaves = measureData->musicDatas(Ove::MusicDataType::OctaveShift_EndPoint);
            for (int j = 0; j < octaves.size(); ++j) {
                  Ove::OctaveShiftEndPoint* octave = static_cast<Ove::OctaveShiftEndPoint*>(octaves[j]);
                  int absTick = _mtt->tick(i, octave->tick());

                  if (octave->octaveShiftPosition() == Ove::OctaveShiftPosition::Start) {
                        if (!ottava) {
                              ottava = new Ottava(_score);
                              ottava->setTrack(track * VOICES);
                              ottava->setOttavaType(octaveShiftTypeToInt(octave->octaveShiftType()));

                              int y_off = 0;
                              switch (octave->octaveShiftType()) {
                                    case Ove::OctaveShiftType::OS_8:
                                    case Ove::OctaveShiftType::OS_15:
                                          y_off = -3;
                                          break;
                                    case Ove::OctaveShiftType::OS_Minus_8:
                                    case Ove::OctaveShiftType::OS_Minus_15:
                                          y_off = 8;
                                          break;
                                    default:
                                          break;
                                    }

                              if (y_off != 0)
                                    ottava->setOffset(QPointF(0, y_off * _score->spatium()));

                              ottava->setTick(Fraction::fromTicks(absTick));
                              }
                        else {
                              qDebug("overlapping octave-shift not supported");
                              delete ottava;
                              ottava = nullptr;
                              }
                        }
                  else if (octave->octaveShiftPosition() == Ove::OctaveShiftPosition::Stop) {
                        if (ottava) {
                              ottava->setTick2(Fraction::fromTicks(absTick));
                              _score->addSpanner(ottava);
                              ottava->staff()->updateOttava();
                              ottava = nullptr;
                              }
                        else {
                              qDebug("octave-shift stop without start");
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertLineBreak
//---------------------------------------------------------

void OveToMScore::convertLineBreak()
      {
      for (MeasureBase* mb = _score->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != ElementType::MEASURE)
                  continue;
            Measure* measure = toMeasure(mb);

            for (int i = 0; i < _ove->lineCount(); ++i) {
                  Ove::Line* line = _ove->line(i);
                  if (measure->no() > 0) {
                        if (int(line->beginBar()) + int(line->barCount()) - 1 == measure->no()) {
                              LayoutBreak* lb = new LayoutBreak(_score);
                              lb->setTrack(0);
                              lb->setLayoutBreakType(LayoutBreak::Type::LINE);
                              measure->add(lb);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertSignature
//---------------------------------------------------------

void OveToMScore::convertSignature()
      {
      int i;
      int j;
      int k;

      // Time
      const QList<MeasureToTick::TimeTick> tts = _mtt->timeTicks();
      for (i = 0; i < tts.size(); ++i) {
            MeasureToTick::TimeTick tt = tts[i];
            Fraction f(tt._numerator, tt._denominator);

            TimeSigMap* sigmap = _score->sigmap();
            sigmap->add(tt._tick, f);

            Measure* measure  = _score->tick2measure(Fraction::fromTicks(tt._tick));
            if (measure) {
                  for (int staffIdx = 0; staffIdx < _score->nstaves(); ++staffIdx) {
                        TimeSigType subtype = TimeSigType::NORMAL;
                        if (tt._numerator == 4 && tt._denominator == 4 && tt._isSymbol)
                              subtype = TimeSigType::FOUR_FOUR;
                        else if (tt._numerator == 2 && tt._denominator == 2 && tt._isSymbol)
                              subtype = TimeSigType::ALLA_BREVE;

                        TimeSig* ts = new TimeSig(_score);
                        ts->setTrack(staffIdx * VOICES);
                        ts->setSig(Fraction(tt._numerator, tt._denominator), subtype);

                        Segment* seg = measure->getSegment(SegmentType::TimeSig, Fraction::fromTicks(tt._tick));
                        seg->add(ts);
                        }
                  }
            }

      // Key
      int staffCount = 0;
      bool createKey = false;
      for (i = 0; i < _ove->partCount(); ++i) {
            int partStaffCount = _ove->staffCount(i);

            for (j = 0; j < partStaffCount; ++j) {
                  for (k = 0; k < _ove->measureCount(); ++k) {
                        Ove::MeasureData* measureData = _ove->measureData(i, j, k);

                        if (measureData) {
                              Ove::Key* keyPtr = measureData->key();

                              if (k == 0 || keyPtr->key() != keyPtr->previousKey())	{
                                    int tick = _mtt->tick(k, 0);
                                    int keyValue = keyPtr->key();
                                    Measure* measure = _score->tick2measure(Fraction::fromTicks(tick));
                                    if (measure) {
                                          KeySigEvent ke;
                                          ke.setKey(Key(keyValue));
                                          _score->staff(staffCount+j)->setKey(Fraction::fromTicks(tick), ke);

                                          KeySig* keysig = new KeySig(_score);
                                          keysig->setTrack((staffCount+j) * VOICES);
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
            for (i = 0; i < _ove->partCount(); ++i) {
                  int partStaffCount = _ove->staffCount(i);

                  for (j = 0; j < partStaffCount; ++j) {
                        Measure* measure = _score->tick2measure(Fraction::fromTicks(_mtt->tick(0, 0)));
                        if (measure) {
                              KeySig* keysig = new KeySig(_score);
                              keysig->setTrack((staffCount+j) * VOICES);
                              keysig->setKeySigEvent(KeySigEvent());

                              Segment* s = measure->getSegment(SegmentType::KeySig, Fraction(0,1));
                              s->add(keysig);
                              }
                        }
                  staffCount += partStaffCount;
                  }
            }

      // Clef
      staffCount = 0;
      for (i = 0; i < _ove->partCount(); ++i) {
            int partStaffCount = _ove->staffCount(i);
            for (j = 0; j < partStaffCount; ++j) {
                  // start clef
                  Staff* staff = _score->staff(staffCount+j);
                  if (staff) {
                        Ove::Track* track = _ove->track(i, j);
                        ClefType clefType = oveClefToClef(track->startClef());
                        Measure* measure = _score->tick2measure(Fraction(0,1));
                        // staff->setClef(0, clefType);

                        // note: also generate symbol for tick 0
                        // was not necessary before 0.9.6
                        Clef* clef = new Clef(_score);
                        clef->setClefType(clefType);
                        clef->setTrack((staffCount + j) * VOICES);

                        Segment* s = measure->getSegment(SegmentType::HeaderClef, Fraction(0,1));
                        s->add(clef);
                        }

                  // clef in measure
                  for (k = 0; k < _ove->measureCount(); ++k) {
                        Ove::MeasureData* measureData = _ove->measureData(i, j, k);
                        QList<Ove::MusicData*> clefs = measureData->musicDatas(Ove::MusicDataType::Clef);
                        Measure* measure = _score->tick2measure(Fraction::fromTicks(_mtt->tick(k, 0)));

                        for (int l = 0; l < clefs.size(); ++l) {
                              if (measure) {
                                    Ove::Clef* clefPtr = static_cast<Ove::Clef*>(clefs[l]);
                                    int absTick = _mtt->tick(k, clefPtr->tick());
                                    ClefType clefType = oveClefToClef(clefPtr->clefType());

                                    Clef* clef = new Clef(_score);
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
      std::map<int, qreal> tempos;
      for (i = 0; i < _ove->partCount(); ++i) {
            int partStaffCount = _ove->staffCount(i);

            for (j = 0; j < partStaffCount; ++j) {
                  for (k = 0; k < _ove->measureCount(); ++k) {
                        Ove::Measure* measure = _ove->measure(k);
                        Ove::MeasureData* measureData = _ove->measureData(i, j, k);
                        QList<Ove::MusicData*> tempoPtrs = measureData->musicDatas(Ove::MusicDataType::Tempo);

                        if (k == 0
                           || (k > 0 && qAbs(measure->typeTempo() - _ove->measure(k - 1)->typeTempo()) > 0.01)) {
                              int tick = _mtt->tick(k, 0);
                              tempos[tick] = measure->typeTempo();
                              }

                        for (int l = 0; l < tempoPtrs.size(); ++l) {
                              Ove::Tempo* ptr = static_cast<Ove::Tempo*>(tempoPtrs[l]);
                              int tick = _mtt->tick(measure->barNumber()->index(), ptr->tick());
                              qreal tempo = ptr->quarterTempo() > 0 ? ptr->quarterTempo() : 1.0;

                              tempos[tick] = tempo;
                              }
                        }
                  }
            }

      std::map<int, qreal>::iterator it;
      int lastTempo = 0;
      for (it = tempos.begin(); it != tempos.end(); ++it) {
            if (it == tempos.begin() || (*it).second != lastTempo) {
                  qreal tpo = ((*it).second) / 60.0;
                  _score->setTempo(Fraction::fromTicks((*it).first), tpo);
                  }

            lastTempo = (*it).second;
            }
      }

#if 0
//---------------------------------------------------------
//   containerToTick
//---------------------------------------------------------

static int containerToTick(Ove::NoteContainer* container, int quarter)
      {
      int tick = Ove::noteTypeToTick(container->noteType(), quarter);

      int dotLength = tick;
      for (int i = 0; i < container->dot(); ++i)
            dotLength /= 2;
      dotLength = tick - dotLength;

      if (container->tuplet() > 0)
            tick = tick * container->space() / container->tuplet();

      tick += dotLength;

      return tick;
      }
#endif

//---------------------------------------------------------
//   getTuplet
//---------------------------------------------------------

static const Ove::Tuplet* getTuplet(const QList<Ove::MusicData*>& tuplets, int unit)
      {
      for (int i = 0; i < tuplets.size(); ++i) {
            const Ove::MusicData* data = tuplets[i];
            if (unit >= data->start()->offset() && unit <= data->stop()->offset()) {
                  const Ove::Tuplet* tuplet = static_cast<Ove::Tuplet*>(tuplets[i]);
                  return tuplet;
                  }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   oveNoteTypeToDuration
//---------------------------------------------------------

static TDuration oveNoteTypeToDuration(Ove::NoteType noteType)
      {
      TDuration d;
      switch (noteType) {
            case Ove::NoteType::Note_DoubleWhole:
                  d.setType(TDuration::DurationType::V_BREVE);
                  break;
            case Ove::NoteType::Note_Whole:
                  d.setType(TDuration::DurationType::V_WHOLE);
                  break;
            case Ove::NoteType::Note_Half:
                  d.setType(TDuration::DurationType::V_HALF);
                  break;
            case Ove::NoteType::Note_Quarter:
                  d.setType(TDuration::DurationType::V_QUARTER);
                  break;
            case Ove::NoteType::Note_Eight:
                  d.setType(TDuration::DurationType::V_EIGHTH);
                  break;
            case Ove::NoteType::Note_Sixteen:
                  d.setType(TDuration::DurationType::V_16TH);
                  break;
            case Ove::NoteType::Note_32:
                  d.setType(TDuration::DurationType::V_32ND);
                  break;
            case Ove::NoteType::Note_64:
                  d.setType(TDuration::DurationType::V_64TH);
                  break;
            case Ove::NoteType::Note_128:
                  d.setType(TDuration::DurationType::V_128TH);
                  break;
            case Ove::NoteType::Note_256:
                  d.setType(TDuration::DurationType::V_256TH);
                  break;
            default:
                  d.setType(TDuration::DurationType::V_QUARTER);
                  break;
            }

      return d;
      }

//---------------------------------------------------------
//   accidentalToAlter
//---------------------------------------------------------

static int accidentalToAlter(Ove::AccidentalType type)
      {
      int alter = 0;

      switch (type) {
            case Ove::AccidentalType::Normal:
            case Ove::AccidentalType::Natural:
            case Ove::AccidentalType::Natural_Caution:
                  alter = 0;
                  break;
            case Ove::AccidentalType::Sharp:
            case Ove::AccidentalType::Sharp_Caution:
                  alter = 1;
                  break;
            case Ove::AccidentalType::Flat:
            case Ove::AccidentalType::Flat_Caution:
                  alter = -1;
                  break;
            case Ove::AccidentalType::DoubleSharp:
            case Ove::AccidentalType::DoubleSharp_Caution:
                  alter = 2;
                  break;
            case Ove::AccidentalType::DoubleFlat:
            case Ove::AccidentalType::DoubleFlat_Caution:
                  alter = -2;
                  break;
            default:
                  break;
            }

      return alter;
      }

//---------------------------------------------------------
//   getMiddleToneOctave
//---------------------------------------------------------

static void getMiddleToneOctave(Ove::ClefType clef, Ove::ToneType& tone, int& octave)
      {
      tone = Ove::ToneType::B;
      octave = 4;

      switch (clef) {
            case Ove::ClefType::Treble:
                  tone = Ove::ToneType::B;
                  octave = 4;
                  break;
            case Ove::ClefType::Treble8va:
                  tone = Ove::ToneType::B;
                  octave = 5;
                  break;
            case Ove::ClefType::Treble8vb:
                  tone = Ove::ToneType::B;
                  octave = 3;
                  break;
            case Ove::ClefType::Bass:
                  tone = Ove::ToneType::D;
                  octave = 3;
                  break;
            case Ove::ClefType::Bass8va:
                  tone = Ove::ToneType::D;
                  octave = 4;
                  break;
            case Ove::ClefType::Bass8vb:
                  tone = Ove::ToneType::D;
                  octave = 2;
                  break;
            case Ove::ClefType::Alto:
                  tone = Ove::ToneType::C;
                  octave = 4;
                  break;
            case Ove::ClefType::UpAlto:
                  tone = Ove::ToneType::A;
                  octave = 3;
                  break;
            case Ove::ClefType::DownDownAlto:
                  tone = Ove::ToneType::G;
                  octave = 4;
                  break;
            case Ove::ClefType::DownAlto:
                  tone = Ove::ToneType::E;
                  octave = 4;
                  break;
            case Ove::ClefType::UpUpAlto:
                  tone = Ove::ToneType::F;
                  octave = 3;
                  break;
            case Ove::ClefType::Percussion1:
                  tone = Ove::ToneType::A;
                  octave = 3;
                  break;
            case Ove::ClefType::Percussion2:
                  tone = Ove::ToneType::A;
                  octave = 3;
                  break;
            case Ove::ClefType::TAB:
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

static Ove::ClefType clefType(Ove::MeasureData* measure, int tick)
      {
      Ove::ClefType type = measure->clef()->clefType();
      QList<Ove::MusicData*> clefs = measure->musicDatas(Ove::MusicDataType::Clef);

      for (int i = 0; i < clefs.size(); ++i) {
            if (tick < clefs[i]->tick()) {
                  break;
                  }
            else /* if (tick >= clefs[i]->tick()) */ {
                  Ove::Clef* clef = static_cast<Ove::Clef*>(clefs[i]);
                  type = clef->clefType();
                  }
            }

      return type;
      }

//---------------------------------------------------------
//   OveToMScore::convertMeasures
//---------------------------------------------------------

void OveToMScore::convertMeasures()
      {
      for (MeasureBase* mb = _score->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != ElementType::MEASURE)
                  continue;
            Measure* measure = toMeasure(mb);
            int tick = measure->tick().ticks();
            measure->setTicks(_score->sigmap()->timesig(tick).timesig());
            measure->setTimesig(_score->sigmap()->timesig(tick).timesig()); //?
            convertMeasure(measure);
            }

      // convert based on notes
      for (MeasureBase* mb = _score->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != ElementType::MEASURE)
                  continue;
            Measure* measure = toMeasure(mb);

            convertLine(measure);
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertMeasure
//---------------------------------------------------------

void OveToMScore::convertMeasure(Measure* measure)
      {
      int staffCount = 0;
      int measureCount = _ove->measureCount();

      for (int i = 0; i < _ove->partCount(); ++i) {
            int partStaffCount = _ove->staffCount(i);

            for (int j = 0; j < partStaffCount; ++j) {
                  int measureID = measure->no();

                  if (measureID >= 0 && measureID < measureCount) {
                        int trackIndex = (staffCount + j) * VOICES;

                        convertMeasureMisc(measure, i, j, trackIndex);
                        convertNote(measure, i, j, trackIndex);
                        convertLyric(measure, i, j, trackIndex);
                        convertHarmony(measure, i, j, trackIndex);
                        convertRepeat(measure, i, j, trackIndex);
                        convertDynamic(measure, i, j, trackIndex);
                        convertExpression(measure, i, j, trackIndex);
                        }
                  }

            staffCount += partStaffCount;
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertLine
//---------------------------------------------------------

void OveToMScore::convertLine(Measure* measure)
      {
      int staffCount = 0;
      int measureCount = _ove->measureCount();

      for (int i = 0; i < _ove->partCount(); ++i) {
            int partStaffCount = _ove->staffCount(i);

            for (int j = 0; j < partStaffCount; ++j) {
                  int measureID = measure->no();

                  if (measureID >= 0 && measureID < measureCount) {
                        int trackIndex = (staffCount + j) * VOICES;

                        convertSlur(measure, i, j, trackIndex);
                        convertGlissando(measure, i, j, trackIndex);
                        convertWedge(measure, i, j, trackIndex);
                        }
                  }

            staffCount += partStaffCount;
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertMeasureMisc
//---------------------------------------------------------

void OveToMScore::convertMeasureMisc(Measure* measure, int part, int staff, int track)
      {
      Ove::Measure* measurePtr = _ove->measure(measure->no());
      Ove::MeasureData* measureData = _ove->measureData(part, staff, measure->no());
      if (!measurePtr || !measureData)
            return;

      // pickup
      if (measurePtr->isPickup())
            measure->setIrregular(true);

      // multiple measure rest
      if (measurePtr->isMultiMeasureRest())
            measure->setBreakMultiMeasureRest(true);

      // barline
      BarLineType barlineType = BarLineType::NORMAL;

      switch (measurePtr->rightBarlineType()) {
            case Ove::BarLineType::Default:
                  barlineType = BarLineType::NORMAL;
                  break;
            case Ove::BarLineType::Double:
                  barlineType = BarLineType::DOUBLE;
                  break;
            case Ove::BarLineType::Final:
                  barlineType = BarLineType::END;
                  break;
            case Ove::BarLineType::Null:
                  barlineType = BarLineType::NORMAL;
                  break;
            case Ove::BarLineType::Repeat_Left:
                  barlineType = BarLineType::START_REPEAT;
                  measure->setRepeatStart(true);
                  break;
            case Ove::BarLineType::Repeat_Right:
                  barlineType = BarLineType::END_REPEAT;
                  measure->setRepeatEnd(true);
                  break;
            case Ove::BarLineType::Dashed:
                  barlineType = BarLineType::BROKEN;
                  break;
            default:
                  break;
            }

      if (barlineType != BarLineType::NORMAL && barlineType != BarLineType::END_REPEAT
         && barlineType != BarLineType::START_REPEAT && barlineType != BarLineType::END_START_REPEAT
         && barlineType != BarLineType::END)
            measure->setEndBarLineType(barlineType, 0);

      else if (barlineType == BarLineType::END_REPEAT)
            measure->setRepeatEnd(true);

      else if (barlineType == BarLineType::START_REPEAT)
            measure->setRepeatStart(true);

      // rehearsal
      int i;
      QList<Ove::MusicData*> texts = measureData->musicDatas(Ove::MusicDataType::Text);
      for (i = 0; i < texts.size(); ++i) {
            Ove::Text* textPtr = static_cast<Ove::Text*>(texts[i]);
            if (textPtr->textType() == Ove::Text::TextType::Rehearsal) {
                  RehearsalMark* rm = new RehearsalMark(_score);
                  rm->setPlainText(textPtr->text());
// TODO:ws        text->setAbove(true);
                  rm->setTrack(track);

                  Segment* s = measure->getSegment(SegmentType::ChordRest,
                     Fraction::fromTicks(_mtt->tick(measure->no(), 0)));
                  s->add(rm);
                  }
            }

      // tempo
      QList<Ove::MusicData*> tempos = measureData->musicDatas(Ove::MusicDataType::Tempo);
      for (i = 0; i < tempos.size(); ++i) {
            Ove::Tempo* tempoPtr = static_cast<Ove::Tempo*>(tempos[i]);
            TempoText* t = new TempoText(_score);
            int absTick = _mtt->tick(measure->no(), tempoPtr->tick());
            qreal tpo = (tempoPtr->quarterTempo())/60.0;

            _score->setTempo(Fraction::fromTicks(absTick), tpo);

            t->setTempo(tpo);
            QString durationTempoL;
            QString durationTempoR;
            if (static_cast<int>(tempoPtr->leftNoteType()))
                  durationTempoL = TempoText::duration2tempoTextString(oveNoteTypeToDuration(tempoPtr->leftNoteType()));
            if (static_cast<int>(tempoPtr->rightNoteType()))
                  durationTempoR = TempoText::duration2tempoTextString(oveNoteTypeToDuration(tempoPtr->rightNoteType()));
            QString textTempo;
            if (tempoPtr->showBeforeText())
                  textTempo += (tempoPtr->leftText()).toHtmlEscaped();
            if (tempoPtr->showMark()) {
                  if (!textTempo.isEmpty())
                        textTempo += " ";
                  if (tempoPtr->showParentheses())
                        textTempo += "(";
                  textTempo += durationTempoL;
                  if (tempoPtr->leftNoteDot())
                        textTempo += "<sym>space</sym><sym>metAugmentationDot</sym>";
                  textTempo += " = ";
                  switch (tempoPtr->rightSideType()) {
                        case 1:
                              textTempo += durationTempoR;
                              if (tempoPtr->rightNoteDot())
                                    textTempo += "<sym>space</sym><sym>metAugmentationDot</sym>";
                              break;
                        case 2:
                              textTempo += (tempoPtr->rightText()).toHtmlEscaped();
                              break;
                        case 3:
                              textTempo += QString::number(qFloor(tempoPtr->typeTempo()));
                              break;
                        case 0:
                        default:
                              textTempo += QString::number(tempoPtr->typeTempo());
                              break;
                        }
                  if (tempoPtr->showParentheses())
                        textTempo += ")";
                  }
            if (textTempo.isEmpty()) {
                  textTempo = durationTempoL;
                  if (tempoPtr->leftNoteDot())
                        textTempo += "<sym>space</sym><sym>metAugmentationDot</sym>";
                  textTempo += " = " + QString::number(tempoPtr->typeTempo());
                  t->setVisible(false);
                  }
            t->setXmlText(textTempo);
// TODO:ws  t->setAbove(true);
            t->setTrack(track);

            Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
            s->add(t);
            }
      }

#if 0
//---------------------------------------------------------
//   graceLevel
//    Beam level in grace notes
//---------------------------------------------------------

static int graceLevel(const QList<Ove::NoteContainer*>& containers, int tick, int unit)
      {
      int graceCount = 0;
      int level = 0; // normal chord rest

      for (int i = 0; i < containers.size(); ++i) {
            Ove::NoteContainer* container = containers[i];
            if (container->tick() > tick)
                  break;

            if (container->isGrace() && container->tick() == tick) {
                  ++graceCount;

                  if (unit <= container->start()->offset()) {
                        ++level;
                        }
                  }
            }

      return level;
      }
#endif

//---------------------------------------------------------
//   isRestDefaultLine
//---------------------------------------------------------

static bool isRestDefaultLine(Ove::Note* rest, Ove::NoteType noteType)
      {
      bool isDefault = true;
      switch (noteType) {
            case Ove::NoteType::Note_DoubleWhole:
            case Ove::NoteType::Note_Whole:
            case Ove::NoteType::Note_Half:
            case Ove::NoteType::Note_Quarter:
                  if (rest->line() != 0)
                        isDefault = false;
                  break;
            case Ove::NoteType::Note_Eight:
                  if (rest->line() != 1)
                        isDefault = false;
                  break;
            case Ove::NoteType::Note_Sixteen:
            case Ove::NoteType::Note_32:
                  if (rest->line() != -1)
                        isDefault = false;
                  break;
            case Ove::NoteType::Note_64:
                  if (rest->line() != -3)
                        isDefault = false;
                  break;
            case Ove::NoteType::Note_128:
                  if (rest->line() != -4)
                        isDefault = false;
                  break;
            default:
                  break;
            }

      return isDefault;
      }

//---------------------------------------------------------
//   drumset
//---------------------------------------------------------

static Drumset* drumset(Score* score, int part)
      {
      Part* p = score->parts().at(part);
      return const_cast<Drumset*>(p->instrument()->drumset());   // TODO: remove cast
      }

//---------------------------------------------------------
//   OveToMScore::convertNote
//---------------------------------------------------------

void OveToMScore::convertNote(Measure* measure, int part, int staff, int track)
      {
      int j;
      Ove::MeasureData* measureData = _ove->measureData(part, staff, measure->no());
      QList<Ove::NoteContainer*> containers = measureData->noteContainers();
      QList<Ove::MusicData*> tuplets = measureData->crossMeasureElements(Ove::MusicDataType::Tuplet, Ove::MeasureData::PairType::Start);
      QList<Ove::MusicData*> beams = measureData->crossMeasureElements(Ove::MusicDataType::Beam, Ove::MeasureData::PairType::Start);
      Tuplet* t = nullptr;
      ChordRest* cr = nullptr;
      int partStaffCount = _ove->staffCount(part);

      if (containers.empty()) {
            int absTick = _mtt->tick(measure->no(), 0);

            cr = new Rest(_score);
            cr->setTicks(measure->ticks());
            cr->setDurationType(TDuration::DurationType::V_MEASURE);
            cr->setTrack(track);
            Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
            s->add(cr);
            }
      QList<Ms::Chord*> graceNotes;
      for (int i = 0; i < containers.size(); ++i) {
            Ove::NoteContainer* container = containers[i];
            int tick = _mtt->tick(measure->no(), container->tick());
            int noteTrack = track + container->voice();

            if (container->isRest()) {
                  TDuration duration = oveNoteTypeToDuration(container->noteType());
                  duration.setDots(container->dot());

                  cr = new Rest(_score);
                  cr->setTicks(duration.fraction());
                  cr->setDurationType(duration);
                  cr->setTrack(noteTrack);
                  cr->setVisible(container->show());
                  Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(tick));
                  s->add(cr);

                  QList<Ove::Note*> notes = container->notesRests();
                  for (j = 0; j < notes.size(); ++j) {
                        Ove::Note* notePtr = notes[j];
                        if (!isRestDefaultLine(notePtr, container->noteType()) && notePtr->line() != 0) {
                              qreal yOffset = -qreal(notePtr->line());
                              int stepOffset = cr->staff()->staffType(cr->tick())->stepOffset();
                              int lineOffset = toRest(cr)->computeLineOffset(5);
                              yOffset -= qreal(lineOffset + stepOffset);
                              yOffset *= _score->spatium() / 2.0;
                              cr->ryoffset() = yOffset;
                              cr->setAutoplace(false);
                              }
                        }
                  }
            else {
                  QList<Ove::Note*> notes = container->notesRests();

                  cr = measure->findChord(Fraction::fromTicks(tick), noteTrack);
                  if (!cr) {
                        cr = new Ms::Chord(_score);
                        cr->setTrack(noteTrack);

                        // grace
                        if (container->isGrace()) {
                              TDuration duration = oveNoteTypeToDuration(container->graceNoteType());
                              duration.setDots(container->dot());
                              toChord(cr)->setNoteType(NoteType::APPOGGIATURA);

                              if (duration.type() == TDuration::DurationType::V_QUARTER) {
                                    toChord(cr)->setNoteType(NoteType::GRACE4);
                                    cr->setDurationType(TDuration::DurationType::V_QUARTER);
                                    }
                              else if (duration.type() == TDuration::DurationType::V_16TH) {
                                    toChord(cr)->setNoteType(NoteType::GRACE16);
                                    cr->setDurationType(TDuration::DurationType::V_16TH);
                                    }
                              else if (duration.type() == TDuration::DurationType::V_32ND) {
                                    toChord(cr)->setNoteType(NoteType::GRACE32);
                                    cr->setDurationType(TDuration::DurationType::V_32ND);
                                    }
                              else {
                                    cr->setDurationType(TDuration::DurationType::V_EIGHTH);
                                    }

                              // st = SegmentType::Grace;
                              }
                        else {
                              TDuration duration = oveNoteTypeToDuration(container->noteType());
                              duration.setDots(container->dot());

                              if (duration.type() == TDuration::DurationType::V_INVALID)
                                    duration.setType(TDuration::DurationType::V_QUARTER);
                              cr->setDurationType(duration);
                              // append grace notes before
                              int ii = -1;
                              for (ii = graceNotes.size() - 1; ii >= 0; --ii) {
                                    Ms::Chord* gc = graceNotes[ii];
                                    if (gc->voice() == cr->voice())
                                          cr->add(gc);
                                    }
                              graceNotes.clear();
                              }
                        cr->setTicks(cr->durationType().fraction());

                        if (!container->isGrace()) {
                              Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(tick));
                              s->add(cr);
                              }
                        else {
                              graceNotes.append(toChord(cr));
                              }
                        }

                  cr->setVisible(container->show());
                  cr->setSmall(container->isCue());
                  for (j = 0; j < notes.size(); ++j) {
                        Ove::Note* oveNote = notes[j];
                        Note* note = new Note(_score);
                        int pitch = oveNote->note();

                        // note->setTrack(noteTrack);
                        note->setVeloType(Note::ValueType::USER_VAL);
                        note->setVeloOffset(oveNote->onVelocity());
                        note->setPitch(pitch);

                        // tpc
                        bool setDirection = false;
                        Ove::ClefType type = clefType(measureData, container->tick());
                        if (type == Ove::ClefType::Percussion1 || type == Ove::ClefType::Percussion2) {
                              Drumset* dSet = drumset(_score, part);
                              if (dSet) {
                                    if (!dSet->isValid(pitch) || pitch == -1) {
                                          qDebug("unmapped drum note 0x%02x %d", note->pitch(), note->pitch());
                                          }
                                    else {
                                          note->setHeadGroup(dSet->noteHead(pitch));
                                          int line = dSet->line(pitch);
                                          note->setLine(line);
                                          note->setTpcFromPitch();
                                          toChord(cr)->setStemDirection(dSet->stemDirection(pitch));
                                          setDirection = true;
                                          }
                                    }
                              else {
                              	    // no drumset, we don't allow mid staff percussion
                                    note->setTpc(14);
                                    }
                              }
                        else {
                              const int OCTAVE = 7;
                              Ove::ToneType clefMiddleTone;
                              int clefMiddleOctave;
                              getMiddleToneOctave(type, clefMiddleTone, clefMiddleOctave);
                              int absLine = static_cast<int>(clefMiddleTone) + clefMiddleOctave * OCTAVE + oveNote->line();
                              if ((partStaffCount == 2) && oveNote->offsetStaff())
                                    absLine += 2 * (oveNote->offsetStaff());
                              int tone = absLine % OCTAVE;
                              int alter = accidentalToAlter(oveNote->accidental());
                              NoteVal nv(pitch);
                              note->setTrack(cr->track());
                              note->setNval(nv, Fraction::fromTicks(tick));
                              // note->setTpcFromPitch();
                              note->setTpc(step2tpc(tone, AccidentalVal(alter)));
                              if (oveNote->showAccidental()) {
                                    Ms::Accidental* a = new Accidental(_score);
                                    bool bracket = static_cast<int>(oveNote->accidental()) & 0x8;
                                    AccidentalType at = Ms::AccidentalType::NONE;
                                    switch (alter) {
                                          case 0:  at = Ms::AccidentalType::NATURAL; break;
                                          case 1:  at = Ms::AccidentalType::SHARP;   break;
                                          case -1: at = Ms::AccidentalType::FLAT;    break;
                                          case 2:  at = Ms::AccidentalType::SHARP2;  break;
                                          case -2: at = Ms::AccidentalType::FLAT2;   break;
                                          }
                                    a->setAccidentalType(at);
                                    a->setBracket(AccidentalBracket(bracket));
                                    a->setRole(Ms::AccidentalRole::USER);
                                    note->add(a);
                                    }
                              note->setHeadGroup(headGroup(oveNote->headType()));
                              }
                        if ((oveNote->headType() == Ove::NoteHeadType::Invisible) || !(oveNote->show()))
                              note->setVisible(false);
                        // tie
                        if ((int(oveNote->tiePos()) & int(Ove::TiePos::LeftEnd)) == int(Ove::TiePos::LeftEnd)) {
                              Tie* tie = new Tie(_score);
                              note->setTieFor(tie);
                              tie->setStartNote(note);
                              tie->setTrack(noteTrack);
                              }

                        // pitch must be set before adding note to chord as note
                        // is inserted into pitch sorted list (ws)
                        cr->add(note);

                        // cr->setVisible(oveNote->show());
                        toChord(cr)->setNoStem(int(container->noteType()) <= int(Ove::NoteType::Note_Whole));
                        if (!setDirection)
                              toChord(cr)->setStemDirection(container->up() ? Direction::UP : Direction::DOWN);

                        // cross staff
                        int staffMove = 0;
                        if (partStaffCount == 2) // treble-bass
                              staffMove = oveNote->offsetStaff();
                        cr->setStaffMove(staffMove);
                        }
                  }

            // beam
            // Beam::Mode bm = container->isRest() ? Beam::Mode::NONE : Beam::Mode::AUTO;
            Beam::Mode bm = Beam::Mode::NONE;
            if (container->inBeam()) {
                  Ove::MeasurePos pos = container->start()->shiftMeasure(0);
                  Ove::MusicData* data = crossMeasureElementByPos(part, staff, pos, container->voice(), Ove::MusicDataType::Beam);

                  if (data) {
                        Ove::Beam* beam = static_cast<Ove::Beam*>(data);
                        Ove::MeasurePos startPos = beam->start()->shiftMeasure(0);
                        Ove::MeasurePos stopPos = beam->stop()->shiftMeasure(beam->start()->measure());

                        if (startPos == pos)
                              bm = Beam::Mode::BEGIN;
                        else if (stopPos == pos)
                              bm = Beam::Mode::END;
                        else
                              bm = Beam::Mode::MID;
                        }
                  }
            cr->setBeamMode(bm);

            // tuplet
            if (container->tuplet() > 0) {
                  if (!t) {
                        bool create = true;

                        // check tuplet start
                        if (container->noteType() < Ove::NoteType::Note_Eight) {
                              const Ove::Tuplet* oveTuplet = getTuplet(tuplets, container->start()->offset());
                              if (!oveTuplet)
                                    create = false;
                              }

                        if (create) {
                              t = new Tuplet(_score);
                              t->setTrack(noteTrack);
                              t->setRatio(Fraction(container->tuplet(), container->space()));
                              TDuration duration = oveNoteTypeToDuration(container->noteType());
                              t->setBaseLen(duration);
                              // tuplet->setTick(tick);
                              t->setParent(measure);
                              // measure->add(tuplet);
                              }
                        }

                  if (t) {
                        cr->setTuplet(t);
                        t->add(cr);

                        // check tuplet end
                        const Ove::Tuplet* oveTuplet = getTuplet(tuplets, container->start()->offset());
                        if (oveTuplet) {
                              //set direction
                              t->setDirection(oveTuplet->leftShoulder()->yOffset() < 0 ? Direction::UP : Direction::DOWN);

                              if (container->start()->offset() == oveTuplet->stop()->offset())
                                    t = nullptr;
                              }
                        }
                  }

            // articulation
            QList<Ove::Articulation*> articulations = container->articulations();
            for (j = 0; j < articulations.size(); ++j)
                  convertArticulation(measure, cr, noteTrack, tick, articulations[j]);
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertArticulation
//---------------------------------------------------------

void OveToMScore::convertArticulation(Measure* measure, ChordRest* cr,
   int track, int absTick, Ove::Articulation* art)
      {
      switch (art->articulationType()) {
            case Ove::ArticulationType::Major_Trill:
            case Ove::ArticulationType::Minor_Trill: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::ornamentTrill);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Trill_Section: {
                  break;
                  }
            case Ove::ArticulationType::Inverted_Short_Mordent:
            case Ove::ArticulationType::Inverted_Long_Mordent: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::ornamentMordent);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Short_Mordent: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::ornamentMordentInverted);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Turn: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::ornamentTurn);
                  cr->add(a);
                  break;
                  }
            // case Ove::ArticulationType::Flat_Accidental_For_Trill:
            // case Ove::ArticulationType::Sharp_Accidental_For_Trill:
            // case Ove::ArticulationType::Natural_Accidental_For_Trill:
            case Ove::ArticulationType::Tremolo_Eighth: {
                  Tremolo* t = new Tremolo(_score);
                  t->setTremoloType(TremoloType::R8);
                  cr->add(t);
                  break;
                  }
            case Ove::ArticulationType::Tremolo_Sixteenth: {
                  Tremolo* t = new Tremolo(_score);
                  t->setTremoloType(TremoloType::R16);
                  cr->add(t);
                  break;
                  }
            case Ove::ArticulationType::Tremolo_Thirty_Second: {
                  Tremolo* t = new Tremolo(_score);
                  t->setTremoloType(TremoloType::R32);
                  cr->add(t);
                  break;
                  }
            case Ove::ArticulationType::Tremolo_Sixty_Fourth: {
                  Tremolo* t = new Tremolo(_score);
                  t->setTremoloType(TremoloType::R64);
                  cr->add(t);
                  break;
                  }
            case Ove::ArticulationType::Marcato: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::articAccentAbove);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Marcato_Dot: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::articAccentAbove);
                  cr->add(a);

                  a = new Articulation(_score);
                  a->setSymId(SymId::articStaccatoAbove);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Heavy_Attack: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::articAccentAbove);
                  cr->add(a);

                  a = new Articulation(_score);
                  a->setSymId(SymId::articTenutoAbove);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::SForzando: {
                  Articulation* a = new Articulation(_score);
                  a->setUp(true);
                  a->setSymId(SymId::articMarcatoAbove);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::SForzando_Inverted: {
                  Articulation* a = new Articulation(_score);
                  a->setUp(false);
                  a->setSymId(SymId::articMarcatoAbove);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::SForzando_Dot: {
                  Articulation* a = new Articulation(_score);
                  a->setUp(true);
                  a->setSymId(SymId::articMarcatoAbove);
                  cr->add(a);

                  a = new Articulation(_score);
                  a->setSymId(SymId::articStaccatoAbove);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::SForzando_Dot_Inverted: {
                  Articulation* a = new Articulation(_score);
                  a->setUp(false);
                  a->setSymId(SymId::articMarcatoAbove);
                  cr->add(a);

                  a = new Articulation(_score);
                  a->setSymId(SymId::articStaccatoAbove);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Heavier_Attack: {
                  Articulation* a = new Articulation(_score);
                  a->setUp(true);
                  a->setSymId(SymId::articMarcatoAbove);
                  cr->add(a);

                  a = new Articulation(_score);
                  a->setSymId(SymId::articTenutoAbove);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Staccatissimo: {
                  Articulation* a = new Articulation(_score);
                  a->setUp(true);
                  a->setSymId(SymId::articStaccatissimoAbove);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Staccato: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::articStaccatoAbove);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Tenuto: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::articTenutoAbove);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Pause: {
                  Breath* b = new Breath(_score);
                  b->setTrack(track);
                  Segment* seg = measure->getSegment(SegmentType::Breath,
                     Fraction::fromTicks(absTick + (cr ? cr->actualTicks().ticks() : 0)));
                  seg->add(b);
                  break;
                  }
            case Ove::ArticulationType::Grand_Pause: {
                  // TODO?
                  break;
                  }
            case Ove::ArticulationType::Up_Bow: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::stringsUpBow);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Down_Bow: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::stringsDownBow);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Up_Bow_Inverted: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::stringsUpBow);
                  a->ryoffset() = 5.3;
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Down_Bow_Inverted: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::stringsDownBow);
                  a->ryoffset() = 5.3;
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Natural_Harmonic: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::stringsHarmonic);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Artificial_Harmonic: {
                  break;
                  }
            case Ove::ArticulationType::Finger_1:
            case Ove::ArticulationType::Finger_2:
            case Ove::ArticulationType::Finger_3:
            case Ove::ArticulationType::Finger_4:
            case Ove::ArticulationType::Finger_5: {
                  break;
                  }
            case Ove::ArticulationType::Plus_Sign: {
                  Articulation* a = new Articulation(_score);
                  a->setSymId(SymId::brassMuteClosed);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Arpeggio: {
                  // there can be only one
                  if (!(toChord(cr))->arpeggio()) {
                        Arpeggio* a = new Arpeggio(_score);
                        a->setArpeggioType(ArpeggioType::NORMAL);
                        /*
                        if (art->placementAbove())
                              a->setSubtype(ArpeggioType::UP);
                        else
                              a->setSubtype(ARpeggioType::DOWN);
                        */
                        cr->add(a);
                        }

                  break;
                  }
            case Ove::ArticulationType::Fermata: {
                  Articulation* a = new Articulation(_score);
                  a->setUp(true);
                  a->setSymId(SymId::fermataAbove);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Fermata_Inverted: {
                  Articulation* a = new Articulation(_score);
                  a->setDirection(Direction::DOWN);
                  a->setSymId(SymId::fermataBelow);
                  cr->add(a);
                  break;
                  }
            case Ove::ArticulationType::Pedal_Down: {
                  if (_pedal) {
                        delete _pedal;
                        _pedal = nullptr;
                        }
                  else {
                        _pedal = new Pedal(_score);
                        _pedal->setTrack(track);
                        Segment* seg = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
                        _pedal->setTick(seg->tick());
                        }
                  break;
                  }
            case Ove::ArticulationType::Pedal_Up: {
                  if (_pedal) {
                        Segment* seg = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
                        _pedal->setTick2(seg->tick());
                        _score->addSpanner(_pedal);
                        _pedal = nullptr;
                        }
                  break;
                  }
            // case Ove::ArticulationType::Toe_Pedal:
            // case Ove::ArticulationType::Heel_Pedal:
            // case Ove::ArticulationType::Toe_To_Heel_Pedal:
            // case Ove::ArticulationType::Heel_To_Toe_Pedal:
            // case Ove::ArticulationType::Open_String:
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertLyric
//---------------------------------------------------------

void OveToMScore::convertLyric(Measure* measure, int part, int staff, int track)
      {
      Ove::MeasureData* measureData = _ove->measureData(part, staff, measure->no());
      if (!measureData)
            return;

      QList<Ove::MusicData*> lyrics = measureData->musicDatas(Ove::MusicDataType::Lyric);

      for (int i = 0; i < lyrics.size(); ++i) {
            Ove::Lyric* oveLyric = static_cast<Ove::Lyric*>(lyrics[i]);
            int tick = _mtt->tick(measure->no(), oveLyric->tick());

            Lyrics* lyric = new Lyrics(_score);
            lyric->setNo(oveLyric->verse());
            lyric->setPlainText(oveLyric->lyric());
            lyric->setTrack(track);
            Segment* segment = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(tick));
            if (segment->element(track))
                  toChordRest(segment->element(track))->add(lyric);
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertHarmony
//---------------------------------------------------------

void OveToMScore::convertHarmony(Measure* measure, int part, int staff, int track)
      {
      Ove::MeasureData* measureData = _ove->measureData(part, staff, measure->no());
      if (!measureData)
            return;

      QList<Ove::MusicData*> harmonys = measureData->musicDatas(Ove::MusicDataType::Harmony);

      for (int i = 0; i < harmonys.size(); ++i) {
            Ove::Harmony* harmonyPtr = static_cast<Ove::Harmony*>(harmonys[i]);
            int absTick = _mtt->tick(measure->no(), harmonyPtr->tick());

            Harmony* harmony = new Harmony(_score);

            // TODO - does this need to be key-aware?
            harmony->setTrack(track);
            harmony->setRootTpc(step2tpc(harmonyPtr->root(), AccidentalVal(harmonyPtr->alterRoot())));
            if (harmonyPtr->bass() != Ove::INVALID_NOTE
               && (harmonyPtr->bass() != harmonyPtr->root()
                  || (harmonyPtr->bass() == harmonyPtr->root() && harmonyPtr->alterBass() != harmonyPtr->alterRoot())))
                  harmony->setBaseTpc(step2tpc(harmonyPtr->bass(), AccidentalVal(harmonyPtr->alterBass())));
            const ChordDescription* d = harmony->fromXml(harmonyPtr->harmonyType());
            if (d) {
                  harmony->setId(d->id);
                  harmony->setTextName(d->names.front());
                  }
            else {
                  harmony->setId(-1);
                  harmony->setTextName(harmonyPtr->harmonyType());
                  }
            harmony->render();

            Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
            s->add(harmony);
            }
      }

#if 0
//---------------------------------------------------------
//   OveToMScore::musicDataByUnit
//---------------------------------------------------------

Ove::MusicData* OveToMScore::musicDataByUnit(int part, int staff, int measure, int unit, Ove::MusicDataType type)
      {
      Ove::MeasureData* measureData = _ove->measureData(part, staff, measure);
      if (measureData) {
            const QList<Ove::MusicData*>& datas = measureData->musicDatas(type);
            for (int i = 0; i < datas.size(); ++i) {
                  if (datas[i]->tick() == unit) // different measurement
                        return datas[i];
                  }
            }
      return nullptr;
      }
#endif

//---------------------------------------------------------
//   OveToMScore::crossMeasureElementByPos
//---------------------------------------------------------

Ove::MusicData* OveToMScore::crossMeasureElementByPos(int part, int staff, const Ove::MeasurePos& pos,
   int voice, Ove::MusicDataType type)
      {
      Ove::MeasureData* measureData = _ove->measureData(part, staff, pos.measure());
      if (measureData) {
            const QList<Ove::MusicData*>& datas = measureData->crossMeasureElements(type, Ove::MeasureData::PairType::All);
            for (int i = 0; i < datas.size(); ++i) {
                  Ove::MeasurePos dataStart = datas[i]->start()->shiftMeasure(0);
                  Ove::MeasurePos dataStop = datas[i]->stop()->shiftMeasure(datas[i]->start()->measure());

                  if (dataStart <= pos && dataStop >= pos && int(datas[i]->voice()) == voice)
                        return datas[i];
                  }
            }

      return nullptr;
      }

//---------------------------------------------------------
//   OveToMScore::containerByPos
//---------------------------------------------------------

Ove::NoteContainer* OveToMScore::containerByPos(int part, int staff, const Ove::MeasurePos& pos)
      {
      Ove::MeasureData* measureData = _ove->measureData(part, staff, pos.measure());
      if (measureData) {
            const QList<Ove::NoteContainer*>& containers = measureData->noteContainers();
            for (int i = 0; i < containers.size(); ++i) {
                  if (pos == containers[i]->start()->shiftMeasure(0))
                        return containers[i];
                  }
            }

      return nullptr;
      }

//---------------------------------------------------------
//   OveToMScore::convertRepeat
//---------------------------------------------------------

void OveToMScore::convertRepeat(Measure* measure, int part, int staff, int track)
      {
      Ove::MeasureData* measureData = _ove->measureData(part, staff, measure->no());
      if (!measureData)
            return;

      int i;
      QList<Ove::MusicData*> repeats = measureData->musicDatas(Ove::MusicDataType::Repeat);

      for (i = 0; i < repeats.size(); ++i) {
            Ove::RepeatSymbol* repeatPtr = static_cast<Ove::RepeatSymbol*>(repeats[i]);
            Ove::RepeatType type = repeatPtr->repeatType();
            Element* e = nullptr;

            switch (type) {
                  case Ove::RepeatType::Segno: {
                        Marker* marker = new Marker(_score);
                        marker->setMarkerType(Marker::Type::SEGNO);
                        e = marker;
                        break;
                        }
                  case Ove::RepeatType::Coda: {
                        Marker* marker = new Marker(_score);
                        marker->setMarkerType(Marker::Type::CODA);
                        e = marker;
                        break;
                        }
                  case Ove::RepeatType::DSAlCoda: {
                        Jump* jp = new Jump(_score);
                        jp->setJumpType(Jump::Type::DS_AL_CODA);
                        e = jp;
                        break;
                        }
                  case Ove::RepeatType::DSAlFine: {
                        Jump* jp = new Jump(_score);
                        jp->setJumpType(Jump::Type::DS_AL_FINE);
                        e = jp;
                        break;
                        }
                  case Ove::RepeatType::DCAlCoda: {
                        Jump* jp = new Jump(_score);
                        jp->setJumpType(Jump::Type::DC_AL_CODA);
                        e = jp;
                        break;
                        }
                  case Ove::RepeatType::DCAlFine: {
                        Jump* jp = new Jump(_score);
                        jp->setJumpType(Jump::Type::DC_AL_FINE);
                        e = jp;
                        break;
                        }
                  case Ove::RepeatType::ToCoda: {
                        Marker* m = new Marker(_score);
                        m->setMarkerType(Marker::Type::TOCODA);
                        e = m;
                        break;
                        }
                  case Ove::RepeatType::Fine: {
                        Marker* m = new Marker(_score);
                        m->setMarkerType(Marker::Type::FINE);
                        e = m;
                        break;
                        }
                  default:
                        break;
                  }

            if (e) {
                  e->setTrack(track);
                  measure->add(e);
                  }
            }

      QList<Ove::MusicData*> endings = measureData->crossMeasureElements(Ove::MusicDataType::Numeric_Ending, Ove::MeasureData::PairType::Start);

      for (i = 0; i < endings.size(); ++i) {
            Ove::NumericEnding* ending = static_cast<Ove::NumericEnding*>(endings[i]);
            int absTick1 = _mtt->tick(measure->no(), 0);
            int absTick2 = _mtt->tick(measure->no() + ending->stop()->measure(), 0);

            if (absTick1 < absTick2) {
                  Volta* volta = new Volta(_score);
                  volta->setTrack(track);
                  volta->setTick(Fraction::fromTicks(absTick1));
                  volta->setTick2(Fraction::fromTicks(absTick2));
                  _score->addElement(volta);
                  volta->setVoltaType(Volta::Type::CLOSED);
                  volta->setText(ending->text());

                  volta->endings().clear();
                  QList<int> numbers = ending->numbers();
                  for (int j = 0; j < numbers.size(); ++j) {
                        volta->endings().append(numbers[j]);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertSlur
//---------------------------------------------------------

void OveToMScore::convertSlur(Measure* measure, int part, int staff, int track)
      {
      Ove::MeasureData* measureData = _ove->measureData(part, staff, measure->no());
      if (!measureData)
            return;

      QList<Ove::MusicData*> slurs = measureData->crossMeasureElements(Ove::MusicDataType::Slur, Ove::MeasureData::PairType::Start);

      for (int i = 0; i < slurs.size(); ++i) {
            Ove::Slur* slurPtr = static_cast<Ove::Slur*>(slurs[i]);

            Ove::NoteContainer* startContainer = containerByPos(part, staff, slurPtr->start()->shiftMeasure(0));
            Ove::NoteContainer* endContainer = containerByPos(part, staff, slurPtr->stop()->shiftMeasure(slurPtr->start()->measure()));

            if (startContainer && endContainer) {
                  int absStartTick = _mtt->tick(slurPtr->start()->measure(), startContainer->tick());
                  int absEndTick = _mtt->tick(slurPtr->start()->measure()+slurPtr->stop()->measure(), endContainer->tick());

                  Slur* slur = new Slur(_score);
                  slur->setSlurDirection(slurPtr->showOnTop()? Direction::UP : Direction::DOWN);
                  slur->setTick(Fraction::fromTicks(absStartTick));
                  slur->setTick2(Fraction::fromTicks(absEndTick));
                  slur->setTrack(track);
                  slur->setTrack2(track+endContainer->offsetStaff());

                  _score->addSpanner(slur);
                  }
            }
      }

//---------------------------------------------------------
//   oveDynamicToDynamic
//---------------------------------------------------------

static QString oveDynamicToDynamic(Ove::DynamicType type)
      {
      QString dynamic = "other-dynamic";

      switch (type) {
            case Ove::DynamicType::PPPP:
                  dynamic = "pppp";
                  break;
            case Ove::DynamicType::PPP:
                  dynamic = "ppp";
                  break;
            case Ove::DynamicType::PP:
                  dynamic = "pp";
                  break;
            case Ove::DynamicType::P:
                  dynamic = "p";
                  break;
            case Ove::DynamicType::MP:
                  dynamic = "mp";
                  break;
            case Ove::DynamicType::MF:
                  dynamic = "mf";
                  break;
            case Ove::DynamicType::F:
                  dynamic = "f";
                  break;
            case Ove::DynamicType::FF:
                  dynamic = "ff";
                  break;
            case Ove::DynamicType::FFF:
                  dynamic = "fff";
                  break;
            case Ove::DynamicType::FFFF:
                  dynamic = "ffff";
                  break;
            case Ove::DynamicType::SF:
                  dynamic = "sf";
                  break;
            case Ove::DynamicType::FZ:
                  dynamic = "fz";
                  break;
            case Ove::DynamicType::SFZ:
                  dynamic = "sfz";
                  break;
            case Ove::DynamicType::SFFZ:
                  dynamic = "sffz";
                  break;
            case Ove::DynamicType::FP:
                  dynamic = "fp";
                  break;
            case Ove::DynamicType::SFP:
                  dynamic = "sfp";
                  break;
            default:
                  break;
            }

      return dynamic;
      }

//---------------------------------------------------------
//   OveToMScore::convertDynamic
//---------------------------------------------------------

void OveToMScore::convertDynamic(Measure* measure, int part, int staff, int track)
      {
      Ove::MeasureData* measureData = _ove->measureData(part, staff, measure->no());
      if (!measureData)
            return;

      QList<Ove::MusicData*> dynamic = measureData->musicDatas(Ove::MusicDataType::Dynamic);

      for (int i = 0; i < dynamic.size(); ++i) {
            Ove::Dynamic* dynamicPtr = static_cast<Ove::Dynamic*>(dynamic[i]);
            int absTick = _mtt->tick(measure->no(), dynamicPtr->tick());
            Dynamic* d = new Dynamic(_score);

            d->setDynamicType(oveDynamicToDynamic(dynamicPtr->dynamicType()));
            d->setTrack(track);

            Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
            s->add(d);
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertExpression
//---------------------------------------------------------

void OveToMScore::convertExpression(Measure* measure, int part, int staff, int track)
      {
      Ove::MeasureData* measureData = _ove->measureData(part, staff, measure->no());
      if (!measureData)
            return;

      QList<Ove::MusicData*> expressions = measureData->musicDatas(Ove::MusicDataType::Expression);

      for (int i = 0; i < expressions.size(); ++i) {
            Ove::Expression* expressionPtr = static_cast<Ove::Expression*>(expressions[i]);
            int absTick = _mtt->tick(measure->no(), expressionPtr->tick());
            Text* t = new Text(_score, Tid::EXPRESSION);

            t->setPlainText(expressionPtr->text());
            t->setTrack(track);

            Segment* s = measure->getSegment(SegmentType::ChordRest, Fraction::fromTicks(absTick));
            s->add(t);
            }
      }

//---------------------------------------------------------
//   OveToMScore::convertGlissando
//---------------------------------------------------------

void OveToMScore::convertGlissando(Measure* measure, int part, int staff, int track)
      {
      Ove::MeasureData* measureData = _ove->measureData(part, staff, measure->no());
      if (!measureData)
            return;

      QList<Ove::MusicData*> glissandos = measureData->crossMeasureElements(Ove::MusicDataType::Glissando, Ove::MeasureData::PairType::All);

      for (int i = 0; i < glissandos.size(); ++i) {
            Ove::Glissando* glissandoPtr = static_cast<Ove::Glissando*>(glissandos[i]);
            Ove::NoteContainer* startContainer = containerByPos(part, staff, glissandoPtr->start()->shiftMeasure(0));
            Ove::NoteContainer* endContainer = containerByPos(part, staff, glissandoPtr->stop()->shiftMeasure(glissandoPtr->start()->measure()));

            if (startContainer && endContainer) {
                  int absTick = _mtt->tick(measure->no(), glissandoPtr->tick());
                  ChordRest* cr = measure->findChordRest(Fraction::fromTicks(absTick), track);
                  if (cr) {
                        Glissando* g = new Glissando(_score);
                        g->setGlissandoType(GlissandoType::WAVY);
                        cr->add(g);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   oveWedgeTypeToType
//---------------------------------------------------------

static HairpinType oveWedgeTypeToType(Ove::WedgeType type)
      {
      HairpinType subtype = HairpinType::CRESC_HAIRPIN;
      switch (type) {
            case Ove::WedgeType::Crescendo:
            case Ove::WedgeType::Crescendo_Line:
            case Ove::WedgeType::Double_Line:
                  subtype = HairpinType::CRESC_HAIRPIN;
                  break;
            case Ove::WedgeType::Decrescendo:
            case Ove::WedgeType::Decrescendo_Line:
                  subtype = HairpinType::DECRESC_HAIRPIN;
                  break;
            default:
                  break;
            }

      return subtype;
      }

//---------------------------------------------------------
//   OveToMScore::convertWedge
//---------------------------------------------------------

void OveToMScore::convertWedge(Measure* measure, int part, int staff, int track)
      {
      Ove::MeasureData* measureData = _ove->measureData(part, staff, measure->no());
      if (!measureData)
            return;

      QList<Ove::MusicData*> wedges = measureData->crossMeasureElements(Ove::MusicDataType::Wedge, Ove::MeasureData::PairType::All);

      for (int i = 0; i < wedges.size(); ++i) {
            Ove::Wedge* wedgePtr = static_cast<Ove::Wedge*>(wedges[i]);
            int absTick = _mtt->tick(measure->no(), MeasureToTick::unitToTick(wedgePtr->start()->offset(), _ove->isQuarter()));
            int absTick2 = _mtt->tick(measure->no()+wedgePtr->stop()->measure(), MeasureToTick::unitToTick(wedgePtr->stop()->offset(), _ove->isQuarter()));

            if (absTick2 > absTick) {
                  Hairpin* hp = new Hairpin(_score);

                  hp->setHairpinType(oveWedgeTypeToType(wedgePtr->wedgeType()));
                  //hp->setYoff(wedgePtr->yOffset());
                  hp->setTrack(track);

                  hp->setTick(Fraction::fromTicks(absTick));
                  hp->setTick2(Fraction::fromTicks(absTick2));
                  hp->setAnchor(Spanner::Anchor::SEGMENT);
                  _score->addSpanner(hp);
                  _score->updateHairpin(hp);
                  }
            }
      }

//---------------------------------------------------------
//   importOve
//---------------------------------------------------------

Score::FileError importOve(MasterScore* score, const QString& name)
      {
      Ove::IOVEStreamLoader* oveLoader = Ove::createOveStreamLoader();
      Ove::OveSong oveSong;

      QFile oveFile(name);
      if (!oveFile.exists())
            return Score::FileError::FILE_NOT_FOUND;
      if (!oveFile.open(QFile::ReadOnly)) {
            //messageOutString(QString("can't read file!"));
            return Score::FileError::FILE_OPEN_ERROR;
            }

      QByteArray buffer = oveFile.readAll();

      oveFile.close();

      oveSong.setTextCodecName(preferences.getString(PREF_IMPORT_OVERTURE_CHARSET));
      oveLoader->setOve(&oveSong);
      oveLoader->setFileStream((unsigned char*) buffer.data(), buffer.size());
      bool result = oveLoader->load();
      oveLoader->release();

      if (result) {
            OveToMScore otm;
            otm.convert(&oveSong, score);

            // score->connectSlurs();
            }

      return result ? Score::FileError::FILE_NO_ERROR : Score::FileError::FILE_ERROR;
      }
