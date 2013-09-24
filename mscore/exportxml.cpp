//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: exportxml.cpp 5598 2012-04-30 07:20:39Z lvinken $
//
//  Copyright (C) 2002-2013 Werner Schweer and others
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

/**
 MusicXML export.
 */

// TODO: trill lines need to be handled the same way as slurs
// in MuseScore they are measure level elements, while in MusicXML
// they are attached to notes (as ornaments)

//=========================================================
//  LVI FIXME
//
//  Evaluate paramenter handling between the various classes, could be simplified
//=========================================================

// TODO LVI 2011-10-30: determine how to report export errors.
// Currently all output (both debug and error reports) are done using qDebug.

#include <math.h>
#include "config.h"
//#include "musescore.h"
#include "file.h"
#include "libmscore/score.h"
#include "libmscore/rest.h"
#include "libmscore/chord.h"
#include "libmscore/sig.h"
#include "libmscore/key.h"
#include "libmscore/clef.h"
#include "libmscore/note.h"
#include "libmscore/segment.h"
#include "libmscore/xml.h"
#include "libmscore/beam.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/measure.h"
#include "libmscore/style.h"
#include "musicxml.h"
#include "libmscore/slur.h"
#include "libmscore/hairpin.h"
#include "libmscore/dynamic.h"
#include "libmscore/barline.h"
#include "libmscore/timesig.h"
#include "libmscore/ottava.h"
#include "libmscore/pedal.h"
#include "libmscore/text.h"
#include "libmscore/tuplet.h"
#include "libmscore/lyrics.h"
#include "libmscore/volta.h"
#include "libmscore/keysig.h"
#include "libmscore/bracket.h"
#include "libmscore/arpeggio.h"
#include "libmscore/jump.h"
#include "libmscore/marker.h"
#include "libmscore/tremolo.h"
#include "libmscore/trill.h"
#include "libmscore/harmony.h"
#include "libmscore/tempotext.h"
#include "libmscore/sym.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/utils.h"
#include "libmscore/articulation.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/element.h"
#include "libmscore/glissando.h"
#include "libmscore/navigate.h"
#include "libmscore/spanner.h"
#include "libmscore/drumset.h"
#include "preferences.h"
#include "libmscore/mscore.h"
#include "libmscore/accidental.h"
#include "libmscore/breath.h"
#include "libmscore/chordline.h"
#include "libmscore/figuredbass.h"
#include "libmscore/tablature.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/qzipwriter_p.h"
#include "libmscore/fret.h"

namespace Ms {

//---------------------------------------------------------
//   local defines for debug output
//---------------------------------------------------------

// #define DEBUG_CLEF true
// #define DEBUG_REPEATS true
// #define DEBUG_TICK true

#ifdef DEBUG_CLEF
#define clefDebug(...) qDebug(__VA_ARGS__)
#else
#define clefDebug(...) ;
#endif

//---------------------------------------------------------
//   typedefs
//---------------------------------------------------------

typedef QMap<int, const FiguredBass*> FigBassMap;

//---------------------------------------------------------
//   attributes -- prints <attributes> tag when necessary
//---------------------------------------------------------

class Attributes {
      bool inAttributes;

public:
      Attributes() { start(); }
      void doAttr(Xml& xml, bool attr);
      void start();
      void stop(Xml& xml);
      };

//---------------------------------------------------------
//   doAttr - when necessary change state and print <attributes> tag
//---------------------------------------------------------

void Attributes::doAttr(Xml& xml, bool attr)
      {
      if (!inAttributes && attr) {
            xml.stag("attributes");
            inAttributes = true;
            }
      else if (inAttributes && !attr) {
            xml.etag();
            inAttributes = false;
            }
      }

//---------------------------------------------------------
//   start -- initialize
//---------------------------------------------------------

void Attributes::start()
      {
      inAttributes = false;
      }

//---------------------------------------------------------
//   stop -- print </attributes> tag when necessary
//---------------------------------------------------------

void Attributes::stop(Xml& xml)
      {
      if (inAttributes) {
            xml.etag();
            inAttributes = false;
            }
      }

//---------------------------------------------------------
//   notations -- prints <notations> tag when necessary
//---------------------------------------------------------

class Notations {
      bool notationsPrinted;

public:
      Notations() { notationsPrinted = false; }
      void tag(Xml& xml);
      void etag(Xml& xml);
      };

//---------------------------------------------------------
//   articulations -- prints <articulations> tag when necessary
//---------------------------------------------------------

class Articulations {
      bool articulationsPrinted;

public:
      Articulations() { articulationsPrinted = false; }
      void tag(Xml& xml);
      void etag(Xml& xml);
      };

//---------------------------------------------------------
//   ornaments -- prints <ornaments> tag when necessary
//---------------------------------------------------------

class Ornaments {
      bool ornamentsPrinted;

public:
      Ornaments() { ornamentsPrinted = false; }
      void tag(Xml& xml);
      void etag(Xml& xml);
      };

//---------------------------------------------------------
//   technical -- prints <technical> tag when necessary
//---------------------------------------------------------

class Technical {
      bool technicalPrinted;

public:
      Technical() { technicalPrinted = false; }
      void tag(Xml& xml);
      void etag(Xml& xml);
      };

//---------------------------------------------------------
//   slur handler -- prints <slur> tags
//---------------------------------------------------------

class SlurHandler {
      const Slur* slur[MAX_NUMBER_LEVEL];
      bool started[MAX_NUMBER_LEVEL];
      int findSlur(const Slur* s) const;

public:
      SlurHandler();
      void doSlurStart(Chord* chord, Notations& notations, Xml& xml);
      void doSlurStop(Chord* chord, Notations& notations, Xml& xml);
      };

//---------------------------------------------------------
//   glissando handler -- prints <glissando> tags
//---------------------------------------------------------

class GlissandoHandler {
      const Chord* glissChrd[MAX_NUMBER_LEVEL];
      const Chord* slideChrd[MAX_NUMBER_LEVEL];
      int findChord(const Chord* c, int st) const;

public:
      GlissandoHandler();
      void doGlissandoStart(Chord* chord, Notations& notations, Xml& xml);
      void doGlissandoStop(Chord* chord, Notations& notations, Xml& xml);
      };

//---------------------------------------------------------
//   ExportMusicXml
//---------------------------------------------------------

typedef QHash<const Chord*, const Trill*> TrillHash;

class ExportMusicXml {
      Score* _score;
      Xml xml;
      SlurHandler sh;
      GlissandoHandler gh;
      int tick;
      Attributes attr;
      TextLine const* bracket[MAX_BRACKETS];
      int div;
      double millimeters;
      int tenths;
      TrillHash trillStart;
      TrillHash trillStop;

      int findBracket(const TextLine* tl) const;
      void chord(Chord* chord, int staff, const QList<Lyrics*>* ll, bool useDrumset);
      void rest(Rest* chord, int staff);
      void clef(int staff, ClefType clef);
      void timesig(TimeSig* tsig);
      void keysig(int key, int staff = 0, bool visible = true);
      void barlineLeft(Measure* m);
      void barlineRight(Measure* m);
      void pitch2xml(Note* note, char& c, int& alter, int& octave);
      void unpitch2xml(Note* note, char& c, int& octave);
      void lyrics(const QList<Lyrics*>* ll, const int trk);
      void work(const MeasureBase* measure);
      void calcDivMoveToTick(int t);
      void calcDivisions();
      double getTenthsFromInches(double);
      double getTenthsFromDots(double);
      void keysigTimesig(Measure* m, int strack, int etrack);

public:
      ExportMusicXml(Score* s)
            {
            _score = s; tick = 0; div = 1; tenths = 40;
            millimeters = _score->spatium() * tenths / (10 * MScore::DPMM);
            }
      void write(QIODevice* dev);
      void credits(Xml& xml);
      void moveToTick(int t);
      void words(Text const* const text, int staff);
      void rehearsal(RehearsalMark const* const rmk, int staff);
      void hairpin(Hairpin const* const hp, int staff, int tick);
      void ottava(Ottava const* const ot, int staff, int tick);
      void pedal(Pedal const* const pd, int staff, int tick);
      void textLine(TextLine const* const tl, int staff, int tick);
      void dynamic(Dynamic const* const dyn, int staff);
      void symbol(Symbol const* const sym, int staff);
      void tempoText(TempoText const* const text, int staff);
      void harmony(Harmony const* const, FretDiagram const* const fd, int offset = 0);
      Score* score() { return _score; }
      };

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Notations::tag(Xml& xml)
      {
      if (!notationsPrinted)
            xml.stag("notations");
      notationsPrinted = true;
      }

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Notations::etag(Xml& xml)
      {
      if (notationsPrinted)
            xml.etag();
      notationsPrinted = false;
      }

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Articulations::tag(Xml& xml)
      {
      if (!articulationsPrinted)
            xml.stag("articulations");
      articulationsPrinted = true;
      }

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Articulations::etag(Xml& xml)
      {
      if (articulationsPrinted)
            xml.etag();
      articulationsPrinted = false;
      }

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Ornaments::tag(Xml& xml)
      {
      if (!ornamentsPrinted)
            xml.stag("ornaments");
      ornamentsPrinted = true;
      }

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Ornaments::etag(Xml& xml)
      {
      if (ornamentsPrinted)
            xml.etag();
      ornamentsPrinted = false;
      }

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Technical::tag(Xml& xml)
      {
      if (!technicalPrinted)
            xml.stag("technical");
      technicalPrinted = true;
      }

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Technical::etag(Xml& xml)
      {
      if (technicalPrinted)
            xml.etag();
      technicalPrinted = false;
      }

//---------------------------------------------------------
//   slurHandler
//---------------------------------------------------------

SlurHandler::SlurHandler()
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            slur[i] = 0;
            started[i] = false;
            }
      }

//---------------------------------------------------------
//   findSlur -- get index of slur in slur table
//   return -1 if not found
//---------------------------------------------------------

int SlurHandler::findSlur(const Slur* s) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (slur[i] == s) return i;
      return -1;
      }

//---------------------------------------------------------
//   doSlurStart
//---------------------------------------------------------

void SlurHandler::doSlurStart(Chord* chord, Notations& notations, Xml& xml)
      {
      // search for slur(s) starting at this chord
      int tick = chord->tick();
      auto sl = chord->score()->spanner();
      for (auto it = sl.lower_bound(tick); it != sl.upper_bound(tick); ++it) {
            Spanner* sp = it->second;
            if (sp->type() != Element::SLUR || sp->track() != chord->track())
                  continue;
            const Slur* s = static_cast<const Slur*>(sp);
            // check if on slur list (i.e. stop already seen)
            int i = findSlur(s);
            //define line type
            QString rest;
            QString lineType;
            switch (s->lineType()) {
                  case 1:
                        lineType = "dotted";
                        break;
                  case 2:
                        lineType = "dashed";
                        break;
                  default:
                        lineType = "";
                  }
            if (!lineType.isEmpty())
                  rest += QString(" line-type=\"%1\"").arg(lineType);
            if (i >= 0) {
                  // remove from list and print start
                  slur[i] = 0;
                  started[i] = false;
                  notations.tag(xml);
                  xml.tagE(QString("slur%1 type=\"start\"%2 number=\"%3\"").arg(rest).arg(s->slurDirection() == MScore::UP ? " placement=\"above\"" : "").arg(i + 1));
                  }
            else {
                  // find free slot to store it
                  i = findSlur(0);
                  if (i >= 0) {
                        slur[i] = s;
                        started[i] = true;
                        notations.tag(xml);
                        xml.tagE(QString("slur%1 type=\"start\" number=\"%2\"").arg(rest).arg(i + 1));
                        }
                  else
                        qDebug("no free slur slot");
                  }
            }
      }

//---------------------------------------------------------
//   doSlurStop
//---------------------------------------------------------

// Note: a slur may start in a higher voice in the same measure.
// In that case it is not yet started (i.e. on the active slur list)
// when doSlurStop() is executed. Handle this slur as follows:
// - generate stop anyway and put it on the slur list
// - doSlurStart() starts slur but doesn't store it

void SlurHandler::doSlurStop(Chord* chord, Notations& notations, Xml& xml)
      {
      // search for slur(s) stopping at this chord but not on slur list yet
      for (auto it : chord->score()->spanner()) {
            Spanner* sp = it.second;
            if (sp->type() != Element::SLUR || sp->tick2() != chord->tick() || sp->track() != chord->track())
                  continue;
            const Slur* s = static_cast<const Slur*>(sp);
            // check if on slur list
            int i = findSlur(s);
            if (i < 0) {
                  // if not, find free slot to store it
                  i = findSlur(0);
                  if (i >= 0) {
                        slur[i] = s;
                        started[i] = false;
                        notations.tag(xml);
                        xml.tagE("slur type=\"stop\" number=\"%d\"", i + 1);
                        }
                  else
                        qDebug("no free slur slot");
                  }
            }

      // search slur list for already started slur(s) stopping at this chord
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (slur[i] && slur[i]->tick2() == chord->tick() && slur[i]->track() == chord->track()) {
                  if (started[i]) {
                        slur[i] = 0;
                        started[i] = false;
                        notations.tag(xml);
                        xml.tagE("slur type=\"stop\" number=\"%d\"", i + 1);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   glissando
//---------------------------------------------------------

// <notations>
//   <slide line-type="solid" number="1" type="start"/>
//   </notations>

// <notations>
//   <glissando line-type="wavy" number="1" type="start"/>
//   </notations>

static void glissando(Glissando* gli, int number, bool start, Notations& notations, Xml& xml)
      {
      GlissandoType st = gli->glissandoType();
      switch (st) {
            case GlissandoType::STRAIGHT:
                  notations.tag(xml);
                  xml.tagE("slide line-type=\"solid\" number=\"%d\" type=\"%s\"",
                           number, start ? "start" : "stop");
                  break;
            case GlissandoType::WAVY:
                  notations.tag(xml);
                  xml.tagE("glissando line-type=\"wavy\" number=\"%d\" type=\"%s\"",
                           number, start ? "start" : "stop");
                  break;
            default:
                  qDebug("unknown glissando subtype %d", st);
                  break;
            }
      }

//---------------------------------------------------------
//   GlissandoHandler
//---------------------------------------------------------

GlissandoHandler::GlissandoHandler()
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            glissChrd[i] = 0;
            slideChrd[i] = 0;
            }
      }

//---------------------------------------------------------
//   findChord -- get index of chord in chord table for subtype st
//   return -1 if not found
//---------------------------------------------------------

int GlissandoHandler::findChord(const Chord* c, int st) const
      {
      if (st != 0 && st != 1) {
            qDebug("GlissandoHandler::findChord: unknown glissando subtype %d", st);
            return -1;
            }
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (st == 0 && slideChrd[i] == c) return i;
            if (st == 1 && glissChrd[i] == c) return i;
            }
      return -1;
      }

//---------------------------------------------------------
//   doGlissandoStart
//---------------------------------------------------------

void GlissandoHandler::doGlissandoStart(Chord* chord, Notations& notations, Xml& xml)
      {
      GlissandoType st = chord->glissando()->glissandoType();
      if (st != GlissandoType::STRAIGHT && st != GlissandoType::WAVY) {
            qDebug("doGlissandoStart: unknown glissando subtype %d", st);
            return;
            }
      // check if on chord list
      int i = findChord(chord, int(st));
      if (i >= 0) {
            // print error and remove from list
            qDebug("doGlissandoStart: chord %p already on list", chord);
            if (st == GlissandoType::STRAIGHT) slideChrd[i] = 0;
            if (st == GlissandoType::WAVY) glissChrd[i] = 0;
            }
      // find free slot to store it
      i = findChord(0, int(st));
      if (i >= 0) {
            if (st == GlissandoType::STRAIGHT) slideChrd[i] = chord;
            if (st == GlissandoType::WAVY) glissChrd[i] = chord;
            glissando(chord->glissando(), i + 1, true, notations, xml);
            }
      else
            qDebug("doGlissandoStart: no free slot");
      }

//---------------------------------------------------------
//   doGlissandoStop
//---------------------------------------------------------

void GlissandoHandler::doGlissandoStop(Chord* chord, Notations& notations, Xml& xml)
      {
      GlissandoType st = chord->glissando()->glissandoType();
      if (st != GlissandoType::STRAIGHT && st != GlissandoType::WAVY) {
            qDebug("doGlissandoStart: unknown glissando subtype %d", st);
            return;
            }
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (st == GlissandoType::STRAIGHT && slideChrd[i] == chord) {
                  slideChrd[i] = 0;
                  glissando(chord->glissando(), i + 1, false, notations, xml);
                  return;
                  }
            if (st == GlissandoType::WAVY && glissChrd[i] == chord) {
                  glissChrd[i] = 0;
                  glissando(chord->glissando(), i + 1, false, notations, xml);
                  return;
                  }
            }
      qDebug("doGlissandoStop: glissando chord %p not found", chord);
      }

//---------------------------------------------------------
//   directions anchor -- anchor directions at another element or a specific tick
//---------------------------------------------------------

class DirectionsAnchor {
      Element* direct;        // the element containing the direction
      Element* anchor;        // the element it is attached to
      bool start;             // whether it is attached to start or end
      int tick;               // the timestamp

public:
      DirectionsAnchor(Element* a, bool s, int t) { direct = 0; anchor = a; start = s; tick = t; }
      DirectionsAnchor(int t) { direct = 0; anchor = 0; start = true; tick = t; }
      Element* getDirect() { return direct; }
      Element* getAnchor() { return anchor; }
      bool getStart() { return start; }
      int getTick() { return tick; }
      void setDirect(Element* d) { direct = d; }
      };

//---------------------------------------------------------
// trill hadling
//---------------------------------------------------------

// Find chords to attach trills to. This is necessary because in MuseScore
// trills are spanners (thus attached to segments), while in MusicXML trills
// are attached to notes.
// TBD: must trill end be in the same staff as trill started ?
// if so, no need to pass in strack and etrack (trill has a track)

static void findTrillAnchors(const Trill* trill, Chord*& startChord, Chord*& stopChord)
      {
      const Segment* seg = trill->startSegment();
      const int endTick  = trill->tick2();
      const int strack   = trill->track();
      // try to find chords in the same track:
      // find a track with suitable chords both for start and stop
      for (int i = 0; i < VOICES; ++i) {
            Element* el = seg->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != Element::CHORD)
                  continue;
            startChord = static_cast<Chord*>(el);
            Segment* s = trill->score()->tick2segmentEnd(strack + i, endTick);
            if (!s)
                  continue;
            el = s->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != Element::CHORD)
                  continue;
            stopChord = static_cast<Chord*>(el);
            return;
            }
      // try to find start/stop chords in different tracks
      for (int i = 0; i < VOICES; ++i) {
            Element* el = seg->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != Element::CHORD)
                  continue;
            startChord = static_cast<Chord*>(el);
            break;      // first chord found is OK
            }
      for (int i = 0; i < VOICES; ++i) {
            Segment* s = trill->score()->tick2segmentEnd(strack + i, endTick);
            if (!s)
                  continue;
            Element* el = s->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != Element::CHORD)
                  continue;
            stopChord = static_cast<Chord*>(el);
            break;      // first chord found is OK
            }
      }

// find all trills in this measure and this part

static void findTrills(Measure* measure, int strack, int etrack, TrillHash& trillStart, TrillHash& trillStop)
      {
      // loop over all segments in this measure
      for (auto it : measure->score()->spanner()) {
            Spanner* e = it.second;
            if (e->type() == Element::TRILL && strack <= e->track() && e->track() < etrack
               && e->tick() >= measure->tick() && e->tick() < measure->tick())
                  {
                  // a trill is found starting in this segment, trill end time is known
                  // determine notes to write trill start and stop
                  const Trill* tr = static_cast<const Trill*>(e);
                  Chord* startChord = 0;  // chord where trill starts
                  Chord* stopChord = 0;   // chord where trill stops

                  findTrillAnchors(tr, startChord, stopChord);

                  if (startChord && stopChord) {
                        trillStart.insert(startChord, tr);
                        trillStop.insert(stopChord, tr);
                        }
                  }
            }
      }

//---------------------------------------------------------
// helpers for ::calcDivisions
//---------------------------------------------------------

typedef QList<int> IntVector;
static IntVector integers;
static IntVector primes;

// check if all integers can be divided by d

static bool canDivideBy(int d)
      {
      bool res = true;
      for (int i = 0; i < integers.count(); i++) {
            if ((integers[i] <= 1) || ((integers[i] % d) != 0)) {
                  res = false;
                  }
            }
      return res;
      }

// divide all integers by d

static void divideBy(int d)
      {
      for (int i = 0; i < integers.count(); i++) {
            integers[i] /= d;
            }
      }

static void addInteger(int len)
      {
      if (!integers.contains(len)) {
            integers.append(len);
            }
      }

//---------------------------------------------------------
//   calcDivMoveToTick
//---------------------------------------------------------

void ExportMusicXml::calcDivMoveToTick(int t)
      {
      if (t < tick) {
#ifdef DEBUG_TICK
            qDebug("backup %d\n", tick - t);
#endif
            addInteger(tick - t);
            }
      else if (t > tick) {
#ifdef DEBUG_TICK
            qDebug("forward %d\n", t - tick);
#endif
            addInteger(t - tick);
            }
      tick = t;
      }

//---------------------------------------------------------
// isTwoNoteTremolo - determine is chord is part of two note tremolo
//---------------------------------------------------------

static bool isTwoNoteTremolo(Chord* chord)
      {
      return (chord->tremolo() && chord->tremolo()->twoNotes());
      }

//---------------------------------------------------------
//  calcDivisions
//---------------------------------------------------------

// Loop over all voices in all staffs and determine a suitable value for divisions.

// Length of time in MusicXML is expressed in "units", which should allow expressing all time values
// as an integral number of units. Divisions contains the number of units in a quarter note.
// MuseScore uses division (480) midi ticks to represent a quarter note, which expresses all note values
// plus triplets and quintuplets as integer values. Solution is to collect all time values required,
// and divide them by the highest common denominator, which is implemented as a series of
// divisions by prime factors. Initialize the list with division to make sure a quarter note can always
// be written as an integral number of units.

/**
 */

void ExportMusicXml::calcDivisions()
      {
      // init
      integers.clear();
      primes.clear();
      integers.append(MScore::division);
      primes.append(2);
      primes.append(3);
      primes.append(5);

      const QList<Part*>& il = _score->parts();

      for (int idx = 0; idx < il.size(); ++idx) {

            Part* part = il.at(idx);
            tick = 0;

            int staves = part->nstaves();
            int strack = _score->staffIdx(part) * VOICES;
            int etrack = strack + staves * VOICES;

            for (MeasureBase* mb = _score->measures()->first(); mb; mb = mb->next()) {

                  if (mb->type() != Element::MEASURE)
                        continue;
                  Measure* m = (Measure*)mb;

                  for (int st = strack; st < etrack; ++st) {
                        // sstaff - xml staff number, counting from 1 for this
                        // instrument
                        // special number 0 -> dont show staff number in
                        // xml output (because there is only one staff)

                        int sstaff = (staves > 1) ? st - strack + VOICES : 0;
                        sstaff /= VOICES;

                        for (Segment* seg = m->first(); seg; seg = seg->next()) {

                              Element* el = seg->element(st);
                              if (!el)
                                    continue;

                              // must ignore start repeat to prevent spurious backup/forward
                              if (el->type() == Element::BAR_LINE && static_cast<BarLine*>(el)->barLineType() == START_REPEAT)
                                    continue;

                              if (tick != seg->tick())
                                    calcDivMoveToTick(seg->tick());

                              if (el->isChordRest()) {
                                    int l = static_cast<ChordRest*>(el)->actualTicks();
                                    if (el->type() == Element::CHORD) {
                                          if (isTwoNoteTremolo(static_cast<Chord*>(el)))
                                                l /= 2;
                                          }
#ifdef DEBUG_TICK
                                    qDebug("chordrest %d", l);
#endif
                                    addInteger(l);
                                    tick += l;
                                    }
                              }
                        }
                  // move to end of measure (in case of incomplete last voice)
                  calcDivMoveToTick(m->tick() + m->ticks());
                  }
            }

      // do it: divide by all primes as often as possible
      for (int u = 0; u < primes.count(); u++)
            while (canDivideBy(primes[u]))
                  divideBy(primes[u]);

      div = MScore::division / integers[0];
#ifdef DEBUG_TICK
      qDebug("divisions=%d div=%d", integers[0], div);
#endif
      }

//---------------------------------------------------------
//   writePageFormat
//---------------------------------------------------------

static void writePageFormat(const PageFormat* pf, Xml& xml, double conversion)
      {
      xml.stag("page-layout");

      //qreal t = 2 * PPI * 10 / 9;

      xml.tag("page-height", pf->size().height() * conversion);
      xml.tag("page-width", pf->size().width() * conversion);
      QString type("both");
      if (pf->twosided()) {
            type = "even";
            xml.stag(QString("page-margins type=\"%1\"").arg(type));
            xml.tag("left-margin",   pf->evenLeftMargin() * conversion);
            xml.tag("right-margin",  pf->evenRightMargin() * conversion);
            xml.tag("top-margin",    pf->evenTopMargin() * conversion);
            xml.tag("bottom-margin", pf->evenBottomMargin() * conversion);
            xml.etag();
            type = "odd";
            }
      xml.stag(QString("page-margins type=\"%1\"").arg(type));
      xml.tag("left-margin",   pf->oddLeftMargin() * conversion);
      xml.tag("right-margin",  pf->oddRightMargin() * conversion);
      xml.tag("top-margin",    pf->oddTopMargin() * conversion);
      xml.tag("bottom-margin", pf->oddBottomMargin() * conversion);
      xml.etag();

      xml.etag();
      }

//---------------------------------------------------------
//   defaults
//---------------------------------------------------------

// _spatium = MScore::DPMM * (millimeter * 10.0 / tenths);

static void defaults(Xml& xml, Score* s, double& millimeters, const int& tenths)
      {
      xml.stag("defaults");
      xml.stag("scaling");
      xml.tag("millimeters", millimeters);
      xml.tag("tenths", tenths);
      xml.etag();
      const PageFormat* pf = s->pageFormat();
      if (pf)
            writePageFormat(pf, xml, INCH / millimeters * tenths);
      // TODO: also write default system layout here
      // when exporting only manual or no breaks, system-distance is not written at all
      xml.etag();
      }


//---------------------------------------------------------
//   creditWords
//---------------------------------------------------------

static void creditWords(Xml& xml, double x, double y, int fs, QString just, QString val, QString words)
      {
      xml.stag("credit page=\"1\"");
      QString tagname = QString("credit-words");
      tagname += QString(" default-x=\"%1\"").arg(x);
      tagname += QString(" default-y=\"%1\"").arg(y);
      tagname += QString(" font-size=\"%1\"").arg(fs);
      tagname += " justify=\"" + just + "\"";
      tagname += " valign=\"" + val + "\"";
      xml.tag(tagname, words);
      xml.etag();
      }


//---------------------------------------------------------
//   credits
//---------------------------------------------------------

void ExportMusicXml::credits(Xml& xml)
      {
      // debug
      qDebug("credits:");
      const MeasureBase* measure = _score->measures()->first();
      if (measure) {
            foreach(const Element* element, *measure->el()) {
                  if (element->type() == Element::TEXT) {
                        const Text* text = (const Text*)element;
                        bool mustPrint = true;
                        if (mustPrint) qDebug("text styled %d style %d(%s) '%s' at %f,%f",
                                              text->styled(),
                                              text->textStyleType(),
                                              qPrintable(text->textStyle().name()),
                                              qPrintable(text->text()),
                                              text->pagePos().x(),
                                              text->pagePos().y()
                                              );
                        }
                  }
            }
      QString rights = _score->metaTag("copyright");
      if (!rights.isEmpty())
            qDebug("copyright '%s'", qPrintable(rights));
      qDebug("end credits");
      // determine formatting
      const PageFormat* pf = _score->pageFormat();
      if (!pf) return;
      //const double t  = 2 * PPI * 10 / 9;
      //const double t  = INCH / millimeters * tenths;
      const double h  = getTenthsFromInches(pf->size().height());
      const double w  = getTenthsFromInches(pf->size().width());
      const double lm = getTenthsFromInches(pf->oddLeftMargin());
      const double rm = getTenthsFromInches(pf->oddRightMargin());
      const double tm = getTenthsFromInches(pf->oddTopMargin());
      const double bm = getTenthsFromInches(pf->oddBottomMargin());
      qDebug(" h=%g w=%g lm=%g rm=%g tm=%g bm=%g", h, w, lm, rm, tm, bm);

      // write the credits
      // TODO add real font size
      if (measure) {
            foreach(const Element* element, *measure->el()) {
                  if (element->type() == Element::TEXT) {
                        const Text* text = (const Text*)element;
                        qDebug("x=%g, y=%g fs=%d",
                               text->pagePos().x(),
                               h - text->pagePos().y(),
                               text->font().pointSize()
                               );
                        const double ty = h - getTenthsFromDots(text->pagePos().y());
                        const int fs = text->font().pointSize();
                        // MusicXML credit-words are untyped and simple list position and font info.
                        // TODO: these parameters should be extracted from text layout and style
                        //       instead of relying on the style name
                        if (text->styled()) {
                              QString styleName = text->textStyle().name();
                              if (styleName == "Title")
                                    creditWords(xml, w / 2, ty, fs, "center", "top", text->text());
                              else if (styleName == "Subtitle")
                                    creditWords(xml, w / 2, ty, fs, "center", "top", text->text());
                              else if (styleName == "Composer")
                                    creditWords(xml, w - rm, ty, fs, "right", "top", text->text());
                              else if (styleName == "Lyricist")
                                    creditWords(xml, lm, ty, fs, "left", "top", text->text());
                              else
                                    qDebug("credits: text style %s not supported", qPrintable(styleName));
                              }
                        }
                  }
            }
      if (!rights.isEmpty()) {
            const int fs = 8; // score->copyright()->font().pointSize();
            creditWords(xml, w / 2, bm, fs, "center", "bottom", rights);
            }
      }

//---------------------------------------------------------
//   midipitch2xml
//---------------------------------------------------------

static int alterTab[12] = { 0,   1,   0,   1,   0,  0,   1,   0,   1,   0,   1,   0 };
static char noteTab[12] = { 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B' };

static void midipitch2xml(int pitch, char& c, int& alter, int& octave)
      {
      // 60 = C 4
      c      = noteTab[pitch % 12];
      alter  = alterTab[pitch % 12];
      octave = pitch / 12 - 1;
      qDebug("midipitch2xml(pitch %d) step %c, alter %d, octave %d", pitch, c, alter, octave);
      }

//---------------------------------------------------------
//   tabpitch2xml
//---------------------------------------------------------

static char TpcNames[] = "FCGDAEB";

static void tabpitch2xml(int pitch, int tpc, char& c, int& alter, int& octave)
      {
      c      = TpcNames[(tpc + 1) % 7];
      alter  = (tpc + 1) / 7 - 2;
      octave = pitch / 12 - 1;
      if (alter < -2 || 2 < alter)
            qDebug("tabpitch2xml(pitch %d, tpc %d) problem:  step %c, alter %d, octave %d",
                   pitch, tpc, c, alter, octave);
      else
            qDebug("tabpitch2xml(pitch %d, tpc %d) step %c, alter %d, octave %d",
                   pitch, tpc, c, alter, octave);
      }

//---------------------------------------------------------
//   pitch2xml
//---------------------------------------------------------

// TODO cleanup / validation
// TBD: use tpc/pitch instead (like tabpitch2xml)

void ExportMusicXml::pitch2xml(Note* note, char& c, int& alter, int& octave)
      {
      static char table1[]  = "FEDCBAG";

      Chord* chord = note->chord();

      int tick   = chord->tick();

      int staffIdx = chord->staffIdx() + chord->staffMove();
      Staff* i   = note->score()->staff(staffIdx);

      ClefType clef   = i->clef(tick);
      int offset = ClefInfo::pitchOffset(clef) - 45;  // HACK

      int step   = (note->line() - offset + 700) % 7;
      // step = 6 - ((absoluteStaffLine(note->line(), clef) + 3)%7));

      // c          = "CDEFGA"[absoluteStaffLine(note->line(), clef) % 7];
      c          = table1[step];

      // printf("====<%c>  <%c>\n", c, "CDEFGAB"[absoluteStaffLine(note->line(), clef) % 7]);

      int pitch  = note->pitch() - 12;
      octave     = pitch / 12;

      static int table2[7] = { 5, 4, 2, 0, 11, 9, 7 };
      int npitch = table2[step] + (octave + 1) * 12;

      //
      // HACK:
      // On percussion clefs there is no relationship between
      // note->pitch() and note->line()
      // note->line() is determined by drumMap
      //
      if (clef == ClefType::PERC || clef == ClefType::PERC2) {
            alter = 0;
            pitch = line2pitch(note->line(), clef, 0);
            octave = (pitch / 12) - 1;
            }
      else
            alter = note->pitch() - npitch;

      // correct for ottava lines
      int ottava = 0;
      switch (note->ppitch() - note->pitch()) {
            case  24: ottava =  2; break;
            case  12: ottava =  1; break;
            case   0: ottava =  0; break;
            case -12: ottava = -1; break;
            case -24: ottava = -2; break;
            default:  qDebug("pitch2xml() tick=%d pitch()=%d ppitch()=%dd",
                             tick, note->pitch(), note->ppitch());
            }
      octave += ottava;

      //deal with Cb and B#
      if (alter > 2) {
            qDebug("pitch2xml problem: alter %d step %d(line %d) octave %d clef %d(offset %d)\n",
                   alter, step, note->line(), octave, clef, offset);
            //HACK:
            alter  -= 12;
            octave += 1;
            }
      if (alter < -2) {
            qDebug("pitch2xml problem: alter %d step %d(line %d) octave %d clef %d(offset %d)\n",
                   alter, step, note->line(), octave, clef, offset);
            //HACK:
            alter  += 12;
            octave -= 1;
            }
      }

// unpitch2xml -- calculate display-step and display-octave for an unpitched note
// note:
// even though this produces the correct step/octave according to Recordare's tutorial
// Finale Notepad 2012 does not import a three line staff with percussion clef correctly
// Same goes for Sibelius 6 in case of three or five line staff with percussion clef

void ExportMusicXml::unpitch2xml(Note* note, char& step, int& octave)
      {
      static char table1[]  = "FEDCBAG";

      int tick        = note->chord()->tick();
      Staff* st       = note->staff();
      ClefType ct     = st->clef(tick);
      // offset in lines between staff with current clef and with G clef
      int clefOffset  = ClefInfo::pitchOffset(ct) - ClefInfo::pitchOffset(ClefType::G);
      // line note would be on on a five line staff with G clef
      // note top line is line 0, bottom line is line 8
      int line5g      = note->line() - clefOffset;
      // in MusicXML with percussion clef, step and octave are determined as if G clef is used
      // when stafflines is not equal to five, in MusicXML the bottom line is still E4.
      // in MuseScore assumes line 0 is F5
      // MS line numbers (top to bottom) plus correction to get lowest line at E4 (line 8)
      // 1 line staff: 0             -> correction 8
      // 3 line staff: 2, 4, 6       -> correction 2
      // 5 line staff: 0, 2, 4, 6, 8 -> correction 0
      // TODO handle other # staff lines ?
      if (st->lines() == 1) line5g += 8;
      if (st->lines() == 3) line5g += 2;
      // index in table1 to get step
      int stepIdx     = (line5g + 700) % 7;
      // get step
      step            = table1[stepIdx];
      // calculate octave, offset "3" correcting for the fact that an octave starts
      // with C instead of F
      octave =(3 - line5g + 700) / 7 + 5 - 100;
      // qDebug("ExportMusicXml::unpitch2xml(%p) clef %d clef.po %d clefOffset %d staff.lines %d note.line %d line5g %d step %c oct %d",
      //        note, ct, clefTable[ct].pitchOffset, clefOffset, st->lines(), note->line(), line5g, step, octave);
      }

//---------------------------------------------------------
//   tick2xml
//    set type + dots depending on tick len
//---------------------------------------------------------

static QString tick2xml(const int ticks, int* dots)
      {
      TDuration t;
      t.setVal(ticks);
      *dots = t.dots();
      return t.name();
      }

//---------------------------------------------------------
//   findVolta -- find volta starting in measure m
//---------------------------------------------------------

static Volta* findVolta(Measure* m, bool left)
      {
      int stick = m->tick();
      int etick = m->tick() + m->ticks();
      for (auto i : m->score()->spannerMap().findOverlapping(stick, etick)) {
            Spanner* el = i.value;
            if (el->type() != Element::VOLTA)
                  continue;
            if(left && el->tick() == stick)
                  return (Volta*) el;
            if(!left && el->tick2() == etick)
                  return (Volta*) el;
            }
      return 0;
      }

//---------------------------------------------------------
//   ending
//---------------------------------------------------------

static void ending(Xml& xml, Volta* v, bool left)
      {
      QString number = "";
      QString type = "";
      foreach(int i, v->endings()) {
            if (!number.isEmpty())
                  number += ", ";
            number += QString("%1").arg(i);
            }
      if (left) {
            type = "start";
            }
      else {
            VoltaType st = v->voltaType();
            switch (st) {
                  case VoltaType::OPEN:
                        type = "discontinue";
                        break;
                  case VoltaType::CLOSED:
                        type = "stop";
                        break;
                  default:
                        qDebug("unknown volta subtype %d\n", st);
                        type = "unknown";
                        break;
                  }
            }
      xml.tagE("ending number=\"%s\" type=\"%s\"",
               number.toLatin1().data(),
               type.toLatin1().data());
      }

//---------------------------------------------------------
//   barlineLeft -- search for and handle barline left
//---------------------------------------------------------

void ExportMusicXml::barlineLeft(Measure* m)
      {
      bool rs = m->repeatFlags() & RepeatStart;
      Volta* volta = findVolta(m, true);
      if (!rs && !volta) return;
      attr.doAttr(xml, false);
      xml.stag(QString("barline location=\"left\""));
      if (rs) {
            xml.tag("bar-style", QString("heavy-light"));
            xml.tagE("repeat direction=\"forward\"");
            }
      if (volta) {
            ending(xml, volta, true);
            }
      xml.etag();
      }

//---------------------------------------------------------
//   barlineRight -- search for and handle barline right
//---------------------------------------------------------

void ExportMusicXml::barlineRight(Measure* m)
      {
      int bst = m->endBarLineType();
      bool visible = m->endBarLineVisible();
      bool needBarStyle = (bst != NORMAL_BAR && bst != START_REPEAT) || !visible;
      Volta* volta = findVolta(m, false);
      if (!needBarStyle && !volta)
            return;
      xml.stag(QString("barline location=\"right\""));
      if (needBarStyle) {
            if (!visible) {
                  xml.tag("bar-style", QString("none"));
                  } else {
                  switch (bst) {
                        case DOUBLE_BAR:
                              xml.tag("bar-style", QString("light-light"));
                              break;
                        case END_REPEAT:
                              xml.tag("bar-style", QString("light-heavy"));
                              break;
                        case BROKEN_BAR:
                              xml.tag("bar-style", QString("dashed"));
                              break;
                        case DOTTED_BAR:
                              xml.tag("bar-style", QString("dotted"));
                              break;
                        case END_BAR:
                        case END_START_REPEAT:
                              xml.tag("bar-style", QString("light-heavy"));
                              break;
                        default:
                              qDebug("ExportMusicXml::bar(): bar subtype %d not supported\n", bst);
                              break;
                        }
                  }
            }
      if (volta)
            ending(xml, volta, false);
      if (bst == END_REPEAT || bst == END_START_REPEAT)
            xml.tagE("repeat direction=\"backward\"");
      xml.etag();
      }

//---------------------------------------------------------
//   moveToTick
//---------------------------------------------------------

void ExportMusicXml::moveToTick(int t)
      {
      // qDebug("ExportMusicXml::moveToTick(t=%d) tick=%d\n", t, tick);
      if (t < tick) {
#ifdef DEBUG_TICK
            qDebug(" -> backup");
#endif
            attr.doAttr(xml, false);
            xml.stag("backup");
            xml.tag("duration", (tick - t) / div);
            xml.etag();
            }
      else if (t > tick) {
#ifdef DEBUG_TICK
            qDebug(" -> forward");
#endif
            attr.doAttr(xml, false);
            xml.stag("forward");
            xml.tag("duration", (t - tick) / div);
            xml.etag();
            }
      tick = t;
      }

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

void ExportMusicXml::timesig(TimeSig* tsig)
      {
      int st = tsig->timeSigType();
      Fraction ts = tsig->sig();
      int z = ts.numerator();
      int n = ts.denominator();
      QString ns = tsig->numeratorString();

      attr.doAttr(xml, true);
      if (st == TSIG_FOUR_FOUR)
            xml.stag("time symbol=\"common\"");
      else if (st == TSIG_ALLA_BREVE)
            xml.stag("time symbol=\"cut\"");
      else
            xml.stag("time");

      QRegExp rx("^\\d+(\\+\\d+)+$"); // matches a compound numerator
      if (rx.exactMatch(ns))
            // if compound numerator, exported as is
            xml.tag("beats", ns);
      else
            // else fall back and use the numerator as integer
            xml.tag("beats", z);
      xml.tag("beat-type", n);
      xml.etag();
      }

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

void ExportMusicXml::keysig(int key, int staff, bool visible)
      {
      QString tg = "key";
      if (staff)
            tg += QString(" number=\"%1\"").arg(staff);
      if (!visible)
            tg += " print-object=\"no\"";
      attr.doAttr(xml, true);
      xml.stag(tg);
      xml.tag("fifths", key);
      xml.tag("mode", QString("major"));
      xml.etag();
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

void ExportMusicXml::clef(int staff, ClefType clef)
      {
      clefDebug("ExportMusicXml::clef(staff %d, clef %d)", staff, clef);

      attr.doAttr(xml, true);
      if (staff)
            xml.stag(QString("clef number=\"%1\"").arg(staff));
      else
            xml.stag("clef");
      QString sign = ClefInfo::sign(clef);
      int line   = ClefInfo::line(clef);
      xml.tag("sign", sign);
      xml.tag("line", line);
      if (ClefInfo::octChng(clef))
            xml.tag("clef-octave-change", ClefInfo::octChng(clef));
      xml.etag();
      }

//---------------------------------------------------------
//   tupletStartStop
//---------------------------------------------------------

// LVIFIX: add placement to tuplet support
// <notations>
//   <tuplet type="start" placement="above" bracket="no"/>
// </notations>

static void tupletStartStop(ChordRest* cr, Notations& notations, Xml& xml)
      {
      Tuplet* t = cr->tuplet();
      if (!t) return;
      if (cr == t->elements().front()) {
            notations.tag(xml);
            QString tupletTag = "tuplet type=\"start\"";
            tupletTag += " bracket=";
            tupletTag += t->hasBracket() ? "\"yes\"" : "\"no\"";
            if (t->numberType() == Tuplet::SHOW_RELATION)
                  tupletTag += " show-number=\"both\"";
            if (t->numberType() == Tuplet::NO_TEXT)
                  tupletTag += " show-number=\"none\"";
            xml.tagE(tupletTag);
            }
      if (cr == t->elements().back()) {
            notations.tag(xml);
            xml.tagE("tuplet type=\"stop\"");
            }
      }

//---------------------------------------------------------
//   wavyLineStartStop
//---------------------------------------------------------

static void wavyLineStartStop(Chord* chord, Notations& notations, Ornaments& ornaments, Xml& xml,
                              TrillHash& trillStart, TrillHash& trillStop)
      {
      // TODO LVI this does not support overlapping trills
      if (trillStop.contains(chord)) {
            notations.tag(xml);
            ornaments.tag(xml);
            xml.tagE("wavy-line type=\"stop\"");
            trillStop.remove(chord);
            }
      if (trillStart.contains(chord)) {
            notations.tag(xml);
            ornaments.tag(xml);
            // mscore only supports wavy-line with trill-mark
            xml.tagE("trill-mark");
            xml.tagE("wavy-line type=\"start\"");
            trillStart.remove(chord);
            }
      }

//---------------------------------------------------------
//   hasBreathMark - determine if chord has breath-mark
//---------------------------------------------------------

static Breath* hasBreathMark(Chord* ch)
      {
      Segment* s = ch->segment();
      s = s->next1();
      Breath* b = 0;
      if (s->segmentType() == Segment::SegBreath)
            b = static_cast<Breath*>(s->element(ch->track()));
      return b;
      }

//---------------------------------------------------------
//   tremoloSingleStartStop
//---------------------------------------------------------

static void tremoloSingleStartStop(Chord* chord, Notations& notations, Ornaments& ornaments, Xml& xml)
      {
      if (chord->tremolo()) {
            Tremolo* tr = chord->tremolo();
            int count = 0;
            int st = tr->tremoloType();
            QString type = "";

            if (chord->tremoloChordType() == TremoloSingle) {
                  type = "single";
                  switch (st) {
                        case TREMOLO_R8:  count = 1; break;
                        case TREMOLO_R16: count = 2; break;
                        case TREMOLO_R32: count = 3; break;
                        case TREMOLO_R64: count = 4; break;
                        default: qDebug("unknown tremolo single %d\n", st); break;
                        }
                  }
            else if (chord->tremoloChordType() == TremoloFirstNote) {
                  type = "start";
                  switch (st) {
                        case TREMOLO_C8:  count = 1; break;
                        case TREMOLO_C16: count = 2; break;
                        case TREMOLO_C32: count = 3; break;
                        case TREMOLO_C64: count = 4; break;
                        default: qDebug("unknown tremolo double %d\n", st); break;
                        }
                  }
            else if (chord->tremoloChordType() == TremoloSecondNote) {
                  type = "stop";
                  switch (st) {
                        case TREMOLO_C8:  count = 1; break;
                        case TREMOLO_C16: count = 2; break;
                        case TREMOLO_C32: count = 3; break;
                        case TREMOLO_C64: count = 4; break;
                        default: qDebug("unknown tremolo double %d\n", st); break;
                        }
                  }
            else qDebug("unknown tremolo subtype %d\n", st);


            if (type != "" && count > 0) {
                  notations.tag(xml);
                  ornaments.tag(xml);
                  xml.tag(QString("tremolo type=\"%1\"").arg(type), count);
                  }
            }
      }


//---------------------------------------------------------
//   chordAttributes
//---------------------------------------------------------

static void chordAttributes(Chord* chord, Notations& notations, Technical& technical, Xml& xml,
                            TrillHash& trillStart, TrillHash& trillStop)
      {
      const QList<Articulation*>& na = chord->articulations();
      // first output the fermatas
      foreach (const Articulation* a, na) {
            ArticulationType at = a->articulationType();
            if (at == Articulation_Fermata
                || at == Articulation_Shortfermata
                || at == Articulation_Longfermata
                || at == Articulation_Verylongfermata) {
                  notations.tag(xml);
                  QString type = a->up() ? "upright" : "inverted";
                  if (at == Articulation_Fermata)
                        xml.tagE(QString("fermata type=\"%1\"").arg(type));
                  else if (at == Articulation_Shortfermata)
                        xml.tag(QString("fermata type=\"%1\"").arg(type), "angled");
                  // MusicXML does not support the very long fermata,
                  // export as long fermata (better than not exporting at all)
                  else if (at == Articulation_Longfermata
                           || at == Articulation_Verylongfermata)
                        xml.tag(QString("fermata type=\"%1\"").arg(type), "square");
                  }
            }

      // then the attributes whose elements are children of <articulations>
      Articulations articulations;
      foreach (const Articulation* a, na) {
            switch (a->articulationType()) {
                  case Articulation_Fermata:
                  case Articulation_Shortfermata:
                  case Articulation_Longfermata:
                  case Articulation_Verylongfermata:
                        // ignore, already handled
                        break;
                  case Articulation_Sforzatoaccent:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("accent");
                        }
                        break;
                  case Articulation_Staccato:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("staccato");
                        }
                        break;
                  case Articulation_Staccatissimo:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("staccatissimo");
                        }
                        break;
                  case Articulation_Tenuto:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("tenuto");
                        }
                        break;
                  case Articulation_Marcato:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        if (a->up())
                              xml.tagE("strong-accent type=\"up\"");
                        else
                              xml.tagE("strong-accent type=\"down\"");
                        }
                        break;
                  case Articulation_Portato:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("detached-legato");
                        }
                        break;
                  case Articulation_Reverseturn:
                  case Articulation_Turn:
                  case Articulation_Trill:
                  case Articulation_Prall:
                  case Articulation_Mordent:
                  case Articulation_PrallPrall:
                  case Articulation_PrallMordent:
                  case Articulation_UpPrall:
                  case Articulation_DownPrall:
                  case Articulation_UpMordent:
                  case Articulation_DownMordent:
                  case Articulation_PrallDown:
                  case Articulation_PrallUp:
                  case Articulation_LinePrall:
                        // ignore, handled with ornaments
                        break;
                  case Articulation_Plusstop:
                  case Articulation_Upbow:
                  case Articulation_Downbow:
                  case Articulation_Snappizzicato:
                  case Articulation_Thumb:
                        // ignore, handled with technical
                        break;
                  default:
                        qDebug("unknown chord attribute %s", qPrintable(a->subtypeUserName()));
                        break;
                  }
            }

      if (Breath* b = hasBreathMark(chord)) {
            notations.tag(xml);
            articulations.tag(xml);
            int st = b->breathType();
            if (st == 0 || st == 1)
                  xml.tagE("breath-mark");
            else
                  xml.tagE("caesura");
            }

      foreach(Element* e, chord->el()) {
            qDebug("chordAttributes: el %p type %d (%s)", e, e->type(), e->name());
            if (e->type() == Element::CHORDLINE) {
                  ChordLine const* const cl = static_cast<ChordLine const* const>(e);
                  QString subtype;
                  switch (cl->chordLineType()) {
                        case CHORDLINE_FALL:
                              subtype = "falloff";
                              break;
                        case CHORDLINE_DOIT:
                              subtype = "doit";
                              break;
                        case CHORDLINE_PLOP:
                              subtype = "plop";
                              break;
                        case CHORDLINE_SCOOP:
                              subtype = "scoop";
                              break;
                        default:
                              qDebug("unknown ChordLine subtype %d", cl->chordLineType());
                        }
                  if (subtype != "") {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE(subtype);
                        }
                  }
            }

      articulations.etag(xml);

      // then the attributes whose elements are children of <ornaments>
      Ornaments ornaments;
      foreach (const Articulation* a, na) {
            switch (a->articulationType()) {
                  case Articulation_Fermata:
                  case Articulation_Shortfermata:
                  case Articulation_Longfermata:
                  case Articulation_Verylongfermata:
                  case Articulation_Sforzatoaccent:
                  case Articulation_Staccato:
                  case Articulation_Staccatissimo:
                  case Articulation_Tenuto:
                  case Articulation_Marcato:
                  case Articulation_Portato:
                        // ignore, already handled
                        break;
                  case Articulation_Reverseturn:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-turn");
                        }
                        break;
                  case Articulation_Turn:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("turn");
                        }
                        break;
                  case Articulation_Trill:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("trill-mark");
                        }
                        break;
                  case Articulation_Prall:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent");
                        }
                        break;
                  case Articulation_Mordent:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent");
                        }
                        break;
                  case Articulation_PrallPrall:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\"");
                        }
                        break;
                  case Articulation_PrallMordent:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent long=\"yes\"");
                        }
                        break;
                  case Articulation_UpPrall:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" approach=\"below\"");
                        }
                        break;
                  case Articulation_DownPrall:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" approach=\"above\"");
                        }
                        break;
                  case Articulation_UpMordent:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent long=\"yes\" approach=\"below\"");
                        }
                        break;
                  case Articulation_DownMordent:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent long=\"yes\" approach=\"above\"");
                        }
                        break;
                  case Articulation_PrallDown:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" departure=\"below\"");
                        }
                        break;
                  case Articulation_PrallUp:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" departure=\"above\"");
                        }
                        break;
                  case Articulation_LinePrall:
                        {
                        // MusicXML 3.0 does not distinguish between downprall and lineprall
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" approach=\"above\"");
                        }
                        break;
                  case Articulation_Schleifer:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("schleifer");
                        }
                        break;
                  case Articulation_Plusstop:
                  case Articulation_Upbow:
                  case Articulation_Downbow:
                  case Articulation_Snappizzicato:
                  case Articulation_Thumb:
                        // ignore, handled with technical
                        break;
                  default:
                        qDebug("unknown chord attribute %s\n", qPrintable(a->subtypeUserName()));
                        break;
                  }
            }
      tremoloSingleStartStop(chord, notations, ornaments, xml);
      wavyLineStartStop(chord, notations, ornaments, xml, trillStart, trillStop);
      ornaments.etag(xml);

      // and finally the attributes whose elements are children of <technical>
      foreach (const Articulation* a, na) {
            switch (a->articulationType()) {
                  case Articulation_Plusstop:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("stopped");
                        }
                        break;
                  case Articulation_Upbow:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("up-bow");
                        }
                        break;
                  case Articulation_Downbow:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("down-bow");
                        }
                        break;
                  case Articulation_Snappizzicato:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("snap-pizzicato");
                        }
                        break;
                  case Articulation_Ouvert:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("open-string");
                        }
                        break;
                  case Articulation_Thumb:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("thumb-position");
                        }
                        break;
                  default:
                        // others silently ignored
                        // qDebug("unknown chord attribute %s", qPrintable(a->subtypeUserName()));
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   arpeggiate
//---------------------------------------------------------

// <notations>
//   <arpeggiate direction="up"/>
//   </notations>

static void arpeggiate(Arpeggio* arp, bool front, bool back, Xml& xml, Notations& notations)
      {
      switch (arp->arpeggioType()) {
            case ArpeggioType::NORMAL:
                  notations.tag(xml);
                  xml.tagE("arpeggiate");
                  break;
            case ArpeggioType::UP:
                  notations.tag(xml);
                  xml.tagE("arpeggiate direction=\"up\"");
                  break;
            case ArpeggioType::DOWN:
                  notations.tag(xml);
                  xml.tagE("arpeggiate direction=\"down\"");
                  break;
            case ArpeggioType::BRACKET:
                  if (front) {
                        notations.tag(xml);
                        xml.tagE("non-arpeggiate type=\"bottom\"");
                        }
                  if (back) {
                        notations.tag(xml);
                        xml.tagE("non-arpeggiate type=\"top\"");
                        }
                  break;
            default:
                  qDebug("unknown arpeggio subtype %d\n", int(arp->arpeggioType()));
                  break;
            }
      }

// find the next chord in the same track

static Chord* nextChord(Chord* ch)
      {
      Segment* s = ch->segment();
      s = s->next1();
      while (s) {
            if (s->segmentType() == Segment::SegChordRest && s->element(ch->track()))
                  break;
            s = s->next1();
            }
      if (s == 0) {
            // qDebug("no segment for second note of glissando found\n");
            return 0;
            }
      Chord* c = static_cast<Chord*>(s->element(ch->track()));
      if (c == 0 || c->type() != Element::CHORD) {
            // qDebug("no second note for glissando found, track %d\n", track());
            return 0;
            }
      return c;
      }

//---------------------------------------------------------
//   determineTupletNormalTicks
//---------------------------------------------------------

/**
 Determine the ticks in the normal type for the tuplet \a chord.
 This is non-zero only if chord if part of a tuplet containing
 different length duration elements.
 TODO determine how to handle baselen with dots and verify correct behaviour.
 TODO verify if baseLen should always be correctly set
      (it seems after MusicXMLimport this is not the case)
 */

static int determineTupletNormalTicks(ChordRest const* const chord)
      {
      Tuplet const* const t = chord->tuplet();
      if (!t)
            return 0;
      qDebug("determineTupletNormalTicks t %p baselen %d", t, t->baseLen().ticks());
      for (int i = 0; i < t->elements().size(); ++i)
            qDebug("determineTupletNormalTicks t %p i %d ticks %d", t, i, t->elements().at(i)->duration().ticks());
      for (int i = 1; i < t->elements().size(); ++i)
            if (t->elements().at(0)->duration().ticks() != t->elements().at(i)->duration().ticks())
                  return t->baseLen().ticks();
      return 0;
      }

//---------------------------------------------------------
//   writeBeam
//---------------------------------------------------------

static void writeBeam(Xml& xml, ChordRest* cr, Beam* b)
      {
      const QList<ChordRest*>& elements = b->elements();
      int idx = elements.indexOf(cr);
      if (idx == -1) {
            qDebug("Beam::writeMusicXml(): cannot find ChordRest\n");
            return;
            }
      int blp = -1; // beam level previous chord
      int blc = -1; // beam level current chord
      int bln = -1; // beam level next chord
      // find beam level previous chord
      for (int i = idx - 1; blp == -1 && i >= 0; --i) {
            ChordRest* crst = elements[i];
            if (crst->type() == Element::CHORD)
                  blp = (static_cast<Chord*>(crst))->beams();
            }
      // find beam level current chord
      if (cr->type() == Element::CHORD)
            blc = (static_cast<Chord*>(cr))->beams();
      // find beam level next chord
      for (int i = idx + 1; bln == -1 && i < elements.size(); ++i) {
            ChordRest* crst = elements[i];
            if (crst->type() == Element::CHORD)
                  bln = (static_cast<Chord*>(crst))->beams();
            }
      for (int i = 1; i <= blc; ++i) {
            QString s;
            if (blp < i && bln >= i) s = "begin";
            else if (blp < i && bln < i) {
                  if (bln > 0) s = "forward hook";
                  else if (blp > 0) s = "backward hook";
                  }
            else if (blp >= i && bln < i)
                  s = "end";
            else if (blp >= i && bln >= i)
                  s = "continue";
            if (s != "")
                  xml.tag(QString("beam number=\"%1\"").arg(i), s);
            }
      }

//---------------------------------------------------------
//   chord
//---------------------------------------------------------

/**
 Write \a chord on \a staff with lyriclist \a ll.

 For a single-staff part, \a staff equals zero, suppressing the <staff> element.
 */

void ExportMusicXml::chord(Chord* chord, int staff, const QList<Lyrics*>* ll, bool useDrumset)
      {
      QList<Note*> nl = chord->notes();
      NoteType gracen = nl.front()->noteType();
      bool grace = (gracen == NOTE_ACCIACCATURA
                    || gracen == NOTE_APPOGGIATURA
                    || gracen == NOTE_GRACE4
                    || gracen == NOTE_GRACE16
                    || gracen == NOTE_GRACE32);
      int tremCorr = 1; // duration correction for two note tremolo
      if (isTwoNoteTremolo(chord)) tremCorr = 2;
      if (!grace) tick += chord->actualTicks();
#ifdef DEBUG_TICK
      qDebug("ExportMusicXml::chord() oldtick=%d", tick);
      qDebug("notetype=%d grace=%d", gracen, grace);
      qDebug(" newtick=%d", tick);
#endif

      const PageFormat* pf = _score->pageFormat();
      const double pageHeight  = getTenthsFromInches(pf->size().height());
      // const double pageWidth  = getTenthsFromInches(pf->size().width());

      foreach(Note* note, nl) {
            QString val;

            attr.doAttr(xml, false);
            QString noteTag = QString("note");

            if (preferences.musicxmlExportLayout && pf) {
                  double measureX = getTenthsFromDots(chord->measure()->pagePos().x());
                  double measureY = pageHeight - getTenthsFromDots(chord->measure()->pagePos().y());
                  double noteX = getTenthsFromDots(note->pagePos().x());
                  double noteY = pageHeight - getTenthsFromDots(note->pagePos().y());

                  noteTag += QString(" default-x=\"%1\"").arg(QString::number(noteX - measureX,'f',2));
                  noteTag += QString(" default-y=\"%1\"").arg(QString::number(noteY - measureY,'f',2));
                  }

            if (!note->visible()) {
                  noteTag += QString(" print-object=\"no\"");
                  }
            //TODO support for OFFSET_VAL
            if (note->veloType() == MScore::USER_VAL) {
                  int velo = note->veloOffset();
                  noteTag += QString(" dynamics=\"%1\"").arg(QString::number(velo * 100.0 / 90.0,'f',2));
                  }
            xml.stag(noteTag);

            if (grace) {
                  if (note->noteType() == NOTE_ACCIACCATURA)
                        xml.tagE("grace slash=\"yes\"");
                  else
                        xml.tagE("grace");
                  }
            if (note != nl.front())
                  xml.tagE("chord");

            char c;
            int alter;
            int octave;
            char buffer[2];

            // TODO: following code requires further cleanup and validation
            if (chord->staff() && chord->staff()->isTabStaff()) {
                  tabpitch2xml(note->pitch(), note->tpc(), c, alter, octave);
                  buffer[0] = c;
                  buffer[1] = 0;
                  // pitch
                  xml.stag("pitch");
                  xml.tag("step", QString(buffer));
                  if (alter)
                        xml.tag("alter", alter);
                  xml.tag("octave", octave);
                  xml.etag();
                  }

            else {
                  if (!useDrumset) {
                        pitch2xml(note, c, alter, octave);
                        buffer[0] = c;
                        buffer[1] = 0;
                        // pitch
                        xml.stag("pitch");
                        xml.tag("step", QString(buffer));
                        if (alter)
                              xml.tag("alter", alter);
                        xml.tag("octave", octave);
                        xml.etag();
                        }
                  else {
                        // unpitched
                        unpitch2xml(note, c, octave);
                        buffer[0] = c;
                        buffer[1] = 0;
                        xml.stag("unpitched");
                        xml.tag("display-step", QString(buffer));
                        xml.tag("display-octave", octave);
                        xml.etag();
                        }
                  }

            // duration
            if (!grace)
                  xml.tag("duration", note->chord()->actualTicks() / div);

            if (note->tieBack())
                  xml.tagE("tie type=\"stop\"");
            if (note->tieFor())
                  xml.tagE("tie type=\"start\"");

            //instrument for unpitched
            if (useDrumset)
                  xml.tagE(QString("instrument id=\"P%1-I%2\"").arg(_score->parts().indexOf(note->staff()->part()) + 1).arg(note->pitch() + 1));

            // voice
            // for a single-staff part, staff is 0, which needs to be corrected
            // to calculate the correct voice number
            int voice = (staff-1) * VOICES + note->chord()->voice() + 1;
            if (staff == 0)
                  voice += VOICES;

            xml.tag("voice", voice);

            // type
            int dots = 0;
            Tuplet* t = note->chord()->tuplet();
            int actNotes = 1;
            int nrmNotes = 1;
            int nrmTicks = 0;
            if (t) {
                  actNotes = t->ratio().numerator();
                  nrmNotes = t->ratio().denominator();
                  nrmTicks = determineTupletNormalTicks(chord);
                  }

            QString s = tick2xml(note->chord()->actualTicks() * actNotes * tremCorr / nrmNotes, &dots);
            if (s.isEmpty()) {
                  qDebug("no note type found for ticks %d\n",
                         note->chord()->actualTicks());
                  }
            xml.tag("type", s);
            for (int ni = dots; ni > 0; ni--)
                  xml.tagE("dot");

            // accidental
            Accidental* acc = note->accidental();
            if (acc) {
                  /*
                        MusicXML accidental names include:
                        sharp,natural, flat, double-sharp, sharp-sharp, flat-flat,
                        natural-sharp, natural-flat, quarter-flat, quarter-sharp,
                        three-quarters-flat, and three-quarters-sharp
                    */
                  QString s;
                  switch (acc->accidentalType()) {
                        case Accidental::ACC_SHARP:              s = "sharp";                break;
                        case Accidental::ACC_FLAT:               s = "flat";                 break;
                        case Accidental::ACC_SHARP2:             s = "double-sharp";         break;
                        case Accidental::ACC_FLAT2:              s = "flat-flat";            break;
                        case Accidental::ACC_NATURAL:            s = "natural";              break;
                        case Accidental::ACC_FLAT_SLASH:         s = "quarter-flat";         break; // (alternative)
                        case Accidental::ACC_MIRRORED_FLAT:      s = "quarter-flat";         break; // (recommended by Michael)
                        case Accidental::ACC_FLAT_ARROW_UP:      s = "quarter-flat";         break; // (alternative)
                        case Accidental::ACC_NATURAL_ARROW_DOWN: s = "quarter-flat";         break; // (alternative)
                        case Accidental::ACC_SHARP_SLASH:        s = "quarter-sharp";        break; // (recommended by Michael)
                        case Accidental::ACC_SHARP_ARROW_DOWN:   s = "quarter-sharp";        break; // (alternative)
                        case Accidental::ACC_NATURAL_ARROW_UP:   s = "quarter-sharp";        break; // (alternative)
                        case Accidental::ACC_MIRRORED_FLAT2:     s = "three-quarters-flat";  break; // (recommended by Michael)
                        case Accidental::ACC_FLAT_FLAT_SLASH:    s = "three-quarters-flat";  break; // (alternative)
                        case Accidental::ACC_FLAT_ARROW_DOWN:    s = "three-quarters-flat";  break; // (alternative)
                        case Accidental::ACC_SHARP_SLASH4:       s = "three-quarters-sharp"; break; // (recommended by Michael)
                        case Accidental::ACC_SHARP_ARROW_UP:     s = "three-quarters-sharp"; break; // (alternate)
                        case Accidental::ACC_SORI:               s = "sori";                 break; //sori
                        case Accidental::ACC_KORON:              s = "koron";                break; //koron
                        default:
                              qDebug("unknown accidental %d\n", int(acc->accidentalType()));
                        }
                  if (s != "") {
                        if (note->accidental()->hasBracket())
                              xml.tag("accidental parentheses=\"yes\"", s);
                        else
                              xml.tag("accidental", s);
                        }
                  }

            // time modification for two note tremolo
            // TODO: support tremolo in tuplet ?
            if (tremCorr == 2) {
                  xml.stag("time-modification");
                  xml.tag("actual-notes", 2);
                  xml.tag("normal-notes", 1);
                  xml.etag();
                  }

            // time modification for tuplet
            if (t) {
                  // TODO: remove following duplicated code (present for both notes and rests)
                  xml.stag("time-modification");
                  xml.tag("actual-notes", actNotes);
                  xml.tag("normal-notes", nrmNotes);
                  qDebug("nrmTicks %d", nrmTicks);
                  if (nrmTicks > 0) {
                        int nrmDots = 0;
                        QString nrmType = tick2xml(nrmTicks, &nrmDots);
                        if (nrmType.isEmpty())
                              qDebug("no note type found for ticks %d", nrmTicks);
                        else {
                              xml.tag("normal-type", nrmType);
                              for (int ni = nrmDots; ni > 0; ni--)
                                    xml.tagE("normal-dot");
                              }
                        }
                  xml.etag();
                  }

            // no stem for whole notes and beyond
            if (chord->noStem() || chord->measure()->slashStyle(chord->staffIdx())) {
                  xml.tag("stem", QString("none"));
                  }
            else if ((note->chord()->actualTicks() * actNotes * tremCorr / nrmNotes) < (4 * MScore::division)) {
                  xml.tag("stem", QString(note->chord()->up() ? "up" : "down"));
                  }

            QString noteheadTagname = QString("notehead");
            QColor noteheadColor = note->color();
            if (noteheadColor != MScore::defaultColor)
                  noteheadTagname += " color=\"" + noteheadColor.name().toUpper() + "\"";
            if (note->headGroup() == 5)
                  xml.tag(noteheadTagname, "slash");
            else if (note->headGroup() == 3)
                  xml.tag(noteheadTagname, "triangle");
            else if (note->headGroup() == 2)
                  xml.tag(noteheadTagname, "diamond");
            else if (note->headGroup() == 1)
                  xml.tag(noteheadTagname, "x");
            else if (note->headGroup() == 6)
                  xml.tag(noteheadTagname, "circle-x");
            else if (note->headGroup() == 7)
                  xml.tag(noteheadTagname, "do");
            else if (note->headGroup() == 8)
                  xml.tag(noteheadTagname, "re");
            else if (note->headGroup() == 4)
                  xml.tag(noteheadTagname, "mi");
            else if (note->headGroup() == 9)
                  xml.tag(noteheadTagname, "fa");
            else if (note->headGroup() == 10)
                  xml.tag(noteheadTagname, "la");
            else if (note->headGroup() == 11)
                  xml.tag(noteheadTagname, "ti");
            else if (note->headGroup() == 12)
                  xml.tag(noteheadTagname, "so");
            else if (noteheadColor != MScore::defaultColor)
                  xml.tag(noteheadTagname, "normal");

            // LVIFIX: check move() handling
            if (staff)
                  xml.tag("staff", staff + note->chord()->staffMove());

            //  beaming
            //    <beam number="1">start</beam>
            //    <beam number="1">end</beam>
            //    <beam number="1">continue</beam>
            //    <beam number="1">backward hook</beam>
            //    <beam number="1">forward hook</beam>

            if (note == nl.front() && chord->beam())
                  writeBeam(xml, chord, chord->beam());

            Notations notations;
            Technical technical;
            if (note->tieBack()) {
                  notations.tag(xml);
                  xml.tagE("tied type=\"stop\"");
                  }
            if (note->tieFor()) {
                  notations.tag(xml);
                  xml.tagE("tied type=\"start\"");
                  }

            if (note == nl.front()) {
                  tupletStartStop(chord, notations, xml);
                  sh.doSlurStop(chord, notations, xml);
                  sh.doSlurStart(chord, notations, xml);
                  chordAttributes(chord, notations, technical, xml, trillStart, trillStop);
                  }
            foreach (const Element* e, note->el()) {
                  if (e->type() == Element::FINGERING) {
                        Text* f = (Text*)e;
                        notations.tag(xml);
                        technical.tag(xml);
                        QString t = f->text();
                        if (f->textStyleType() == TEXT_STYLE_FINGERING) {
                              // p, i, m, a, c represent the plucking finger
                              if (t == "p" || t == "i" || t == "m" || t == "a" || t == "c")
                                    xml.tag("pluck", t);
                              else
                                    xml.tag("fingering", t);
                              }
                        else if (f->textStyleType() == TEXT_STYLE_STRING_NUMBER) {
                              xml.tag("string", t);
                              }
                        else
                              qDebug("unknown fingering style");
                        }
                  else {
                        // TODO
                        }
                  }

            // write tablature string / fret
            if (chord->staff() && chord->staff()->isTabStaff())
                  if (note->fret() >= 0 && note->string() >= 0) {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tag("string", note->string() + 1);
                        xml.tag("fret", note->fret());
                        }

            technical.etag(xml);
            if (chord->arpeggio()) {
                  arpeggiate(chord->arpeggio(), note == nl.front(), note == nl.back(), xml, notations);
                  }
            // write glissando (only for last note)
            Chord* ch = nextChord(chord);
            if ((note == nl.back()) && ch && ch->glissando()) {
                  gh.doGlissandoStart(ch, notations, xml);
                  }
            if (chord->glissando()) {
                  gh.doGlissandoStop(chord, notations, xml);
                  }
            notations.etag(xml);
            // write lyrics (only for first note)
            if ((note == nl.front()) && ll)
                  lyrics(ll, chord->track());
            xml.etag();
            }
      }

//---------------------------------------------------------
//   rest
//---------------------------------------------------------

/**
 Write \a rest on \a staff.

 For a single-staff part, \a staff equals zero, suppressing the <staff> element.
 */

void ExportMusicXml::rest(Rest* rest, int staff)
      {
      static char table2[]  = "CDEFGAB";
#ifdef DEBUG_TICK
      qDebug("ExportMusicXml::rest() oldtick=%d", tick);
#endif
      attr.doAttr(xml, false);

      QString noteTag = QString("note");
      if (!rest->visible() ) {
            noteTag += QString(" print-object=\"no\"");
            }
      xml.stag(noteTag);

      int yOffsSt   = 0;
      int oct       = 0;
      int stp       = 0;
      ClefType clef = rest->staff()->clef(rest->tick());
      int po        = ClefInfo::pitchOffset(clef);

      // Determine y position, but leave at zero in case of tablature staff
      // as no display-step or display-octave should be written for a tablature staff,

      if (clef != ClefType::TAB && clef != ClefType::TAB2) {
            double yOffsSp = rest->userOff().y() / rest->spatium();              // y offset in spatium (negative = up)
            yOffsSt = -2 * int(yOffsSp > 0.0 ? yOffsSp + 0.5 : yOffsSp - 0.5); // same rounded to int (positive = up)

            po -= 4;    // pitch middle staff line (two lines times two steps lower than top line)
            po += yOffsSt; // rest "pitch"
            oct = po / 7; // octave
            stp = po % 7; // step
            }

      // Either <rest/>
      // or <rest><display-step>F</display-step><display-octave>5</display-octave></rest>
      if (yOffsSt == 0) {
            xml.tagE("rest");
            }
      else {
            xml.stag("rest");
            xml.tag("display-step", QString(QChar(table2[stp])));
            xml.tag("display-octave", oct - 1);
            xml.etag();
            }

      TDuration d = rest->durationType();
      int tickLen = rest->actualTicks();
      if (d.type() == TDuration::V_MEASURE) {
            // to avoid forward since rest->ticklen=0 in this case.
            tickLen = rest->measure()->ticks();
            }
      tick += tickLen;
#ifdef DEBUG_TICK
      qDebug(" tickLen=%d newtick=%d", tickLen, tick);
#endif

      xml.tag("duration", tickLen / div);

      // for a single-staff part, staff is 0, which needs to be corrected
      // to calculate the correct voice number
      int voice = (staff-1) * VOICES + rest->voice() + 1;
      if (staff == 0)
            voice += VOICES;
      xml.tag("voice", voice);

      // do not output a "type" element for whole measure rest
      if (d.type() != TDuration::V_MEASURE) {
            QString s = d.name();
            int dots  = rest->dots();
            xml.tag("type", s);
            for (int i = dots; i > 0; i--)
                  xml.tagE("dot");
            }

      if (rest->tuplet()) {
            Tuplet* t = rest->tuplet();
            xml.stag("time-modification");
            xml.tag("actual-notes", t->ratio().numerator());
            xml.tag("normal-notes", t->ratio().denominator());
            int nrmTicks = determineTupletNormalTicks(rest);
            if (nrmTicks > 0) {
                  int nrmDots = 0;
                  QString nrmType = tick2xml(nrmTicks, &nrmDots);
                  if (nrmType.isEmpty())
                        qDebug("no note type found for ticks %d", nrmTicks);
                  else {
                        xml.tag("normal-type", nrmType);
                        for (int ni = nrmDots; ni > 0; ni--)
                              xml.tagE("normal-dot");
                        }
                  }
            xml.etag();
            }

      if (staff)
            xml.tag("staff", staff);

      Notations notations;
      tupletStartStop(rest, notations, xml);
      notations.etag(xml);

      xml.etag();
      }

//---------------------------------------------------------
//   directionTag
//---------------------------------------------------------

static void directionTag(Xml& xml, Attributes& attr, Element const* const el = 0)
      {
      attr.doAttr(xml, false);
      QString tagname = QString("direction");
      if (el) {
            /*
            qDebug("directionTag() spatium=%g nelem tp=%d (%s,%s)\ndirectionTag()  x=%g y=%g xsp,ysp=%g,%g w=%g h=%g userOff.y=%g",
                   el->spatium(),
                   el->type(),
                   el->name(), el->subtypeName().toUtf8().data(),
                   el->x(), el->y(),
                   el->x()/el->spatium(), el->y()/el->spatium(),
                   el->width(), el->height(),
                   el->userOff().y()
                  );
            if (el->type() == Element::HAIRPIN || el->type() == Element::OTTAVA || el->type() == Element::TEXTLINE) {
                  SLine* sl = static_cast<const SLine*>(el);
                  qDebug("directionTag()  slin segsz=%d", sl->spannerSegments().size());
                  if (sl->spannerSegments().size() > 0) {
                        LineSegment* seg = (LineSegment*)sl->spannerSegments().at(0);
                        qDebug(" x=%g y=%g w=%g h=%g cpx=%g cpy=%g userOff.y=%g",
                               seg->x(), seg->y(),
                               seg->width(), seg->height(),
                               seg->pagePos().x(), seg->pagePos().y(),
                               seg->userOff().y());
                         }
                  } // if (el->type() == ...
            */
            Element* pel = el->parent();
            Element* ppel = 0;
            if (pel) ppel = pel->parent();
            /*
            if (pel) {
                  qDebug("directionTag()  prnt tp=%d (%s,%s) x=%g y=%g w=%g h=%g userOff.y=%g",
                         pel->type(),
                         pel->name(), pel->subtypeName().toUtf8().data(),
                         pel->x(), pel->y(),
                         pel->width(), pel->height(),
                         pel->userOff().y());
                  }
            if (ppel) {
                  qDebug("directionTag()  pprnt tp=%d (%s,%s) x=%g y=%g w=%g h=%g userOff.y=%g",
                         ppel->type(),
                         ppel->name(), ppel->subtypeName().toUtf8().data(),
                         ppel->x(), ppel->y(),
                         ppel->width(), ppel->height(),
                         ppel->userOff().y());
                  }
            */
            if (ppel && ppel->type() == Element::MEASURE) {
                  // Measure* m = static_cast<Measure*>(ppel);
                  // System* sys = m->system();
                  // QRectF bb = sys->staff(el->staffIdx())->bbox();
                  /*
                  qDebug("directionTag()  syst x=%g y=%g cpx=%g cpy=%g",
                         sys->pos().x(),  sys->pos().y(),
                         sys->pagePos().x(),
                         sys->pagePos().y()
                        );
                  qDebug("directionTag()  staf x=%g y=%g w=%g h=%g",
                         bb.x(), bb.y(),
                         bb.width(), bb.height());
                  // element is above the staff if center of bbox is above center of staff
                  qDebug("directionTag()  center diff=%g", el->y() + el->height() / 2 - bb.y() - bb.height() / 2);
                  */
                  if (el->placement() == Element::ABOVE)
                        tagname += " placement=\"above\"";
                  else if (el->placement() == Element::BELOW)
                        tagname += " placement=\"below\"";

#if 0
                  if (el->type() == Element::HAIRPIN || el->type() == Element::OTTAVA || el->type() == Element::PEDAL || el->type() == Element::TEXTLINE) {
                        SLine const* const sl = static_cast<SLine const* const>(el);
                        if (sl->spannerSegments().size() > 0) {
                              LineSegment* seg = (LineSegment*)sl->spannerSegments().at(0);
                              // for the line type elements the reference point is vertically centered
                              // actual position info is in the segments
                              // compare the segment's canvas ypos with the staff's center height
                              if (seg->pagePos().y() < sys->pagePos().y() + bb.y() + bb.height() / 2)
                                    tagname += " placement=\"above\"";
                              else
                                    tagname += " placement=\"below\"";
                              }
                        }
                  else {
                        if (el->y() + el->height() / 2 < bb.y() + bb.height() / 2)
                              tagname += " placement=\"above\"";
                        else
                              tagname += " placement=\"below\"";
                        }
#endif
                  } // if (ppel && ...
            }
      xml.stag(tagname);
      }

//---------------------------------------------------------
//   directionETag
//---------------------------------------------------------

static void directionETag(Xml& xml, int staff, int offs = 0)
      {
      if (offs)
            xml.tag("offset", offs);
      if (staff)
            xml.tag("staff", staff);
      xml.etag();
      }

//---------------------------------------------------------
//   partGroupStart
//---------------------------------------------------------

static void partGroupStart(Xml& xml, int number, int bracket)
      {
      xml.stag(QString("part-group type=\"start\" number=\"%1\"").arg(number));
      QString br = "";
      switch (bracket) {
            case NO_BRACKET:
                  br = "none";
                  break;
            case BRACKET_NORMAL:
                  br = "bracket";
                  break;
            case BRACKET_BRACE:
                  br = "brace";
                  break;
            case BRACKET_LINE:
                  br = "line";
                  break;
            case BRACKET_SQUARE:
                  br = "square";
                  break;
            default:
                  qDebug("bracket subtype %d not understood\n", bracket);
            }
      if (br != "")
            xml.tag("group-symbol", br);
      xml.etag();
      }

//---------------------------------------------------------
//   words
//---------------------------------------------------------

// The note and dot symbols are in Unicode plane, they are stored
// in a QString as a surrogate pair.
// Example: quarter note is 0x1d15f, stored as 0xd834 0xdd5f.

// a line containing only a note and zero or more dots
QRegExp metro("^\\xd834([\\xdd5c-\\xdd5f]|[\\xdd60-\\xdd63])(\\xd834\\xdd6d)?$");
// a note, zero or more dots, zero or more spaces, an equals sign, zero or more spaces
QRegExp metroPlusEquals("\\xd834([\\xdd5c-\\xdd5f]|[\\xdd60-\\xdd63])(\\xd834\\xdd6d)? ?= ?");
// a parenthesis open, zero or more spaces at end of line
QRegExp leftParen("\\( ?$");
// zero or more spaces, an equals sign, zero or more spaces at end of line
QRegExp equals(" ?= ?$");

// note: findUnitAndDots does not check the first char of the surrogate pair
//       this has already been done by findMetronome using the regexps above

static bool findUnitAndDots(QString words, QString& unit, int& dots)
      {
      unit = "";
      dots = 0;
      // qDebug("findUnitAndDots('%s') slen=%d", qPrintable(words), words.length());
      if (!metro.exactMatch(words))
            return false;
      switch (words.at(1).unicode()) {
            case 0xdd5c: unit = "breve"; break;
            case 0xdd5d: unit = "whole"; break;
            case 0xdd5e: unit = "half"; break;
            case 0xdd5f: unit = "quarter"; break;
            case 0xdd60: unit = "eighth"; break;
            case 0xdd61: unit = "16th"; break;
            case 0xdd62: unit = "32nd"; break;
            case 0xdd63: unit = "64th"; break;
            default: qDebug("findUnitAndDots: unknown char '%s'(0x%0xd)",
                            qPrintable(words.mid(0, 1)), words.at(0).unicode());
            }
      for (int i = 3; i < words.length(); i += 2)
            switch (words.at(i).unicode()) {
                  case '.':    // fall through
                  case 0xdd6d: ++dots; break;
                  // TODO case 0xe10b: ++dots; ++dots; break;
                  default: qDebug("findUnitAndDots: unknown char '%s'(0x%0xd)",
                                  qPrintable(words.mid(i, 1)), words.at(i).unicode());
                  }
      // qDebug(" unit='%s' dots=%d", qPrintable(unit), dots);
      return true;
      }

static bool findMetronome(QString words,
                          QString& wordsLeft,  // words left of metronome
                          bool& hasParen,      // parenthesis
                          QString& metroLeft,  // left part of metronome
                          QString& metroRight, // right part of metronome
                          QString& wordsRight  // words right of metronome
                          )
      {
      QString hexWords;
      for (int i = 0; i < words.length(); ++i) {
            QString n;
            n.setNum(words.at(i).unicode(),16);
            if (i != 0) hexWords += " ";
            hexWords += "0x";
            hexWords += n;
            }
      // qDebug("findMetronome('%s') (%s) slen=%d", qPrintable(words), qPrintable(hexWords), words.length());
      wordsLeft  = "";
      hasParen   = false;
      metroLeft  = "";
      metroRight = "";
      wordsRight = "";
      int pos = metroPlusEquals.indexIn(words);
      if (pos != -1) {
            int len = metroPlusEquals.matchedLength();
            /*
            qDebug(" mpos=%d mlen=%d",
                   pos, len
                   );
            */
            if (words.length() > pos + len) {
                  QString s1 = words.mid(0, pos);    // string to the left of metronome
                  QString s2 = words.mid(pos, len);  // first note and equals sign
                  QString s3 = words.mid(pos + len); // string to the right of equals sign
                  /*
                  qDebug("found metronome: '%s'%s'%s'",
                         qPrintable(s1),
                         qPrintable(s2),
                         qPrintable(s3)
                         );
                  */
                  // determine if metronome has parentheses
                  // left part of string must end with parenthesis plus optional spaces
                  // right part of string must have parenthesis (but not in first pos)
                  int lparen = leftParen.indexIn(s1);
                  int rparen = s3.indexOf(")");
                  hasParen = (lparen != -1 && rparen > 0);
                  // qDebug(" lparen=%d rparen=%d hasP=%d", lparen, rparen, hasParen);
                  if (hasParen) wordsLeft = s1.mid(0, lparen);
                  else wordsLeft = s1;
                  int equalsPos = equals.indexIn(s2);
                  if (equalsPos != -1) metroLeft = s2.mid(0, equalsPos);
                  // else qDebug("\ncan't find equals in s2\n");
                  if (hasParen) {
                        metroRight = s3.mid(0, rparen);
                        wordsRight = s3.mid(rparen + 1, s3.length() - rparen - 1);
                        }
                  else {
                        metroRight = s3;
                        }
                  /*
                  qDebug(" '%s'%s'%s'%s'",
                         qPrintable(wordsLeft),
                         qPrintable(metroLeft),
                         qPrintable(metroRight),
                         qPrintable(wordsRight)
                         );
                  */
                  QString unit;
                  int dots;
                  return findUnitAndDots(metroLeft, unit, dots);
                  }
            }
      return false;
      }

static void wordsMetrome(Xml& xml, Text const* const text)
      {
      QString wordsLeft;  // words left of metronome
      bool hasParen;      // parenthesis
      QString metroLeft;  // left part of metronome
      QString metroRight; // right part of metronome
      QString wordsRight; // words right of metronome
      if (findMetronome(text->text(), wordsLeft, hasParen, metroLeft, metroRight, wordsRight)) {
            if (wordsLeft != "") {
                  xml.stag("direction-type");
                  xml.tag("words", wordsLeft);
                  xml.etag();
                  }
            xml.stag("direction-type");
            xml.stag(QString("metronome parentheses=\"%1\"").arg(hasParen ? "yes" : "no"));
            QString unit;
            int dots;
            findUnitAndDots(metroLeft, unit, dots);
            xml.tag("beat-unit", unit);
            while (dots > 0) {
                  xml.tagE("beat-unit-dot");
                  --dots;
                  }
            if (findUnitAndDots(metroRight, unit, dots)) {
                  xml.tag("beat-unit", unit);
                  while (dots > 0) {
                        xml.tagE("beat-unit-dot");
                        --dots;
                        }
                  }
            else
                  xml.tag("per-minute", metroRight);
            xml.etag();
            xml.etag();
            if (wordsRight != "") {
                  xml.stag("direction-type");
                  xml.tag("words", wordsRight);
                  xml.etag();
                  }
            }
      else {
            xml.stag("direction-type");
            xml.tag("words", text->text());
            xml.etag();
            }
      }

void ExportMusicXml::tempoText(TempoText const* const text, int staff)
      {
      /*
      qDebug("ExportMusicXml::tempoText(TempoText='%s')", qPrintable(text->text()));
      */
      attr.doAttr(xml, false);
      xml.stag(QString("direction placement=\"%1\"").arg((text->parent()->y()-text->y() < 0.0) ? "below" : "above"));
      wordsMetrome(xml, text);
      /*
      int offs = text->mxmlOff();
      if (offs)
            xml.tag("offset", offs);
      */
      if (staff)
            xml.tag("staff", staff);
      xml.tagE(QString("sound tempo=\"%1\"").arg(QString::number(text->tempo()*60.0)));
      xml.etag();
      }

//---------------------------------------------------------
//   words
//---------------------------------------------------------

void ExportMusicXml::words(Text const* const text, int staff)
      {
      /*
      qDebug("ExportMusicXml::words userOff.x=%f userOff.y=%f xoff=%g yoff=%g text='%s'",
             text->userOff().x(), text->userOff().y(), text->xoff(), text->yoff(),
             text->text().toUtf8().data());
      */
      directionTag(xml, attr, text);
      if (text->type() == Element::REHEARSAL_MARK) {
            // TODO: check if dead code (see rehearsal below)
            xml.stag("direction-type");
            xml.tag("rehearsal", text->text());
            xml.etag();
            }
      else
            wordsMetrome(xml, text);
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   rehearsal
//---------------------------------------------------------

void ExportMusicXml::rehearsal(RehearsalMark const* const rmk, int staff)
      {
      directionTag(xml, attr, rmk);
      xml.stag("direction-type");
      xml.tag("rehearsal", rmk->text());
      xml.etag();
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   hairpin
//---------------------------------------------------------

void ExportMusicXml::hairpin(Hairpin const* const hp, int staff, int tick)
      {
      directionTag(xml, attr, hp);
      xml.stag("direction-type");
      if (hp->tick() == tick)
            xml.tagE("wedge type=\"%s\"", hp->hairpinType() ? "diminuendo" : "crescendo");
      else
            xml.tagE("wedge type=\"stop\"");
      xml.etag();
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   ottava
// <octave-shift type="down" size="8" relative-y="14"/>
// <octave-shift type="stop" size="8"/>
//---------------------------------------------------------

void ExportMusicXml::ottava(Ottava const* const ot, int staff, int tick)
      {
      int st = int(ot->ottavaType());
      directionTag(xml, attr, ot);
      xml.stag("direction-type");
      if (ot->tick() == tick) {
            const char* sz = 0;
            const char* tp = 0;
            switch (st) {
                  case 0:
                        sz = "8";
                        tp = "down";
                        break;
                  case 1:
                        sz = "15";
                        tp = "down";
                        break;
                  case 2:
                        sz = "8";
                        tp = "up";
                        break;
                  case 3:
                        sz = "15";
                        tp = "up";
                        break;
                  default:
                        qDebug("ottava subtype %d not understood\n", st);
                  }
            if (sz && tp)
                  xml.tagE("octave-shift type=\"%s\" size=\"%s\"", tp, sz);
            }
      else {
            if (st == 0 || st == 2)
                  xml.tagE("octave-shift type=\"stop\" size=\"8\"");
            else if (st == 1 || st == 3)
                  xml.tagE("octave-shift type=\"stop\" size=\"15\"");
            else
                  qDebug("ottava subtype %d not understood\n", st);
            }
      xml.etag();
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   pedal
//---------------------------------------------------------

void ExportMusicXml::pedal(Pedal const* const pd, int staff, int tick)
      {
      directionTag(xml, attr, pd);
      xml.stag("direction-type");
      if (pd->tick() == tick)
            xml.tagE("pedal type=\"start\" line=\"yes\"");
      else
            xml.tagE("pedal type=\"stop\" line=\"yes\"");
      xml.etag();
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   findBracket -- get index of bracket in bracket table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findBracket(const TextLine* tl) const
      {
      for (int i = 0; i < MAX_BRACKETS; ++i)
            if (bracket[i] == tl) return i;
      return -1;
      }

//---------------------------------------------------------
//   textLine
//---------------------------------------------------------

void ExportMusicXml::textLine(TextLine const* const tl, int staff, int tick)
      {
      QString rest;
      QPointF p;

      // special case: a dashed line w/o hooks is written as dashes
      bool dashes = tl->lineStyle() == Qt::DashLine && !tl->beginHook() && !tl->endHook();

      QString lineEnd = "none";
      QString type;
      bool hook = false;
      double hookHeight = 0.0;
      int n = 0;
      if (tl->tick() == tick) {
            if (!dashes) {
                  QString lineType;
                  switch (tl->lineStyle()) {
                        case Qt::SolidLine:
                              lineType = "solid";
                              break;
                        case Qt::DashLine:
                              lineType = "dashed";
                              break;
                        case Qt::DotLine:
                              lineType = "dotted";
                              break;
                        default:
                              lineType = "solid";
                        }
                  rest += QString(" line-type=\"%1\"").arg(lineType);
                  }
            hook = tl->beginHook();
            hookHeight = tl->beginHookHeight().val();
            p = tl->spannerSegments().first()->userOff();
            // offs = tl->mxmlOff();
            type = "start";
            }
      else {
            hook = tl->endHook();
            hookHeight = tl->endHookHeight().val();
            p = ((LineSegment*)tl->spannerSegments().last())->userOff2();
            // offs = tl->mxmlOff2();
            type = "stop";
            }

      if (hook) {
            if (hookHeight < 0.0) {
                  lineEnd = "up";
                  hookHeight *= -1.0;
                  }
            else
                  lineEnd = "down";
            rest += QString(" end-length=\"%1\"").arg(hookHeight * 10);
            }

      n = findBracket(tl);
      if (n >= 0)
            bracket[n] = 0;
      else {
            n = findBracket(0);
            bracket[n] = tl;
            }

      if (preferences.musicxmlExportLayout && p.x() != 0)
            rest += QString(" default-x=\"%1\"").arg(p.x() * 10 / tl->spatium());
      if (preferences.musicxmlExportLayout && p.y() != 0)
            rest += QString(" default-y=\"%1\"").arg(p.y() * -10 / tl->spatium());

      directionTag(xml, attr, tl);
      if (tl->beginText() && tl->tick() == tick) {
            xml.stag("direction-type");
            xml.tag("words", tl->beginText()->text());
            xml.etag();
            }
      xml.stag("direction-type");
      if (dashes)
            xml.tagE(QString("dashes type=\"%1\" number=\"%2\"").arg(type, QString::number(n + 1)));
      else
            xml.tagE(QString("bracket type=\"%1\" number=\"%2\" line-end=\"%3\"%4").arg(type, QString::number(n + 1), lineEnd, rest));
      xml.etag();
      /*
      if (offs)
            xml.tag("offset", offs);
      */
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   dynamic
//---------------------------------------------------------

// In MuseScore dynamics are essentially user-defined texts, therefore the ones
// supported by MusicXML need to be filtered out. Everything not recognized
// as MusicXML dynamics is written as words.

void ExportMusicXml::dynamic(Dynamic const* const dyn, int staff)
      {
      QString t = dyn->text();
      Dynamic::DynamicType st = dyn->dynamicType();

      directionTag(xml, attr, dyn);
      xml.stag("direction-type");
      if (st == Dynamic::DYNAMIC_p
          || st == Dynamic::DYNAMIC_pp
          || st == Dynamic::DYNAMIC_ppp
          || st == Dynamic::DYNAMIC_pppp
          || st == Dynamic::DYNAMIC_ppppp
          || st == Dynamic::DYNAMIC_pppppp
          || st == Dynamic::DYNAMIC_f
          || st == Dynamic::DYNAMIC_ff
          || st == Dynamic::DYNAMIC_fff
          || st == Dynamic::DYNAMIC_ffff
          || st == Dynamic::DYNAMIC_fffff
          || st == Dynamic::DYNAMIC_ffffff
          || st == Dynamic::DYNAMIC_mp
          || st == Dynamic::DYNAMIC_mf
          || st == Dynamic::DYNAMIC_sf
          || st == Dynamic::DYNAMIC_sfp
          || st == Dynamic::DYNAMIC_sfpp
          || st == Dynamic::DYNAMIC_fp
          || st == Dynamic::DYNAMIC_rf
          || st == Dynamic::DYNAMIC_rfz
          || st == Dynamic::DYNAMIC_sfz
          || st == Dynamic::DYNAMIC_sffz
          || st == Dynamic::DYNAMIC_fz) {
            xml.stag("dynamics");
            xml.tagE(dyn->dynamicTypeName());
            xml.etag();
            }
      else if (st == Dynamic::DYNAMIC_m || st == Dynamic::DYNAMIC_z) {
            xml.stag("dynamics");
            xml.tag("other-dynamics", dyn->dynamicTypeName());
            xml.etag();
            }
      else
            xml.tag("words", t);
      xml.etag();
      /*
      int offs = dyn->mxmlOff();
      if (offs)
            xml.tag("offset", offs);
      */
      if (staff)
            xml.tag("staff", staff);

      if (dyn->velocity() > 0)
            xml.tagE(QString("sound dynamics=\"%1\"").arg(QString::number(dyn->velocity() * 100.0 / 90.0, 'f', 2)));

      xml.etag();
      }

//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

// TODO: remove dependency on symbol name and replace by a more stable interface
// changes in sym.cpp r2494 broke MusicXML export of pedals (again)

void ExportMusicXml::symbol(Symbol const* const sym, int staff)
      {
      QString name = Sym::id2name(sym->sym());
      const char* mxmlName = "";
      if (name == "pedal.Ped")
            mxmlName = "pedal type=\"start\"";
      else if (name == "pedal.*")
            mxmlName = "pedal type=\"stop\"";
      else {
            qDebug("ExportMusicXml::symbol(): %s not supported", name.toLatin1().data());
            return;
            }
      directionTag(xml, attr, sym);
      xml.stag("direction-type");
      xml.tagE(mxmlName);
      xml.etag();
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   lyrics
//---------------------------------------------------------

void ExportMusicXml::lyrics(const QList<Lyrics*>* ll, const int trk)
      {
      foreach(const Lyrics* l, *ll) {
            if (l) {
                  if ((l)->track() == trk) {
                        xml.stag(QString("lyric number=\"%1\"").arg((l)->no() + 1));
                        int syl   = (l)->syllabic();
                        QString s = "";
                        switch (syl) {
                              case Lyrics::SINGLE: s = "single"; break;
                              case Lyrics::BEGIN:  s = "begin";  break;
                              case Lyrics::END:    s = "end";    break;
                              case Lyrics::MIDDLE: s = "middle"; break;
                              default:
                                    qDebug("unknown syllabic %d\n", syl);
                              }
                        xml.tag("syllabic", s);
                        xml.tag("text", (l)->text());
                        /*
                         Temporarily disabled because it doesn't work yet (and thus breaks the regression test).
                         See MusicXml::xmlLyric: "// TODO-WS      l->setTick(tick);"
                        if((l)->endTick() > 0)
                              xml.tagE("extend");
                        */
                        xml.etag();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   directionJump -- write jump
//---------------------------------------------------------

// LVIFIX: TODO coda and segno should be numbered uniquely

static void directionJump(Xml& xml, const Jump* const jp)
      {
      JumpType jtp = jp->jumpType();
      QString words = "";
      QString type  = "";
      QString sound = "";
      if (jtp == JumpType::DC) {
            if (jp->text() == "")
                  words = "D.C.";
            else
                  words = jp->text();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == JumpType::DC_AL_FINE) {
            if (jp->text() == "")
                  words = "D.C. al Fine";
            else
                  words = jp->text();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == JumpType::DC_AL_CODA) {
            if (jp->text() == "")
                  words = "D.C. al Coda";
            else
                  words = jp->text();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == JumpType::DS_AL_CODA) {
            if (jp->text() == "")
                  words = "D.S. al Coda";
            else
                  words = jp->text();
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else if (jtp == JumpType::DS_AL_FINE) {
            if (jp->text() == "")
                  words = "D.S. al Fine";
            else
                  words = jp->text();
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else if (jtp == JumpType::DS) {
            words = "D.S.";
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else
            qDebug("jump type=%d not implemented\n", jtp);
      if (sound != "") {
            xml.stag("direction placement=\"above\"");
            xml.stag("direction-type");
            if (type != "") xml.tagE(type);
            if (words != "") xml.tag("words", words);
            xml.etag();
            if (sound != "") xml.tagE(QString("sound ") + sound);
            xml.etag();
            }
      }

//---------------------------------------------------------
//   directionMarker -- write marker
//---------------------------------------------------------

static void directionMarker(Xml& xml, const Marker* const m)
      {
      MarkerType mtp = m->markerType();
      QString words = "";
      QString type  = "";
      QString sound = "";
      if (mtp == MarkerType::CODA) {
            type = "coda";
            if (m->label() == "")
                  sound = "coda=\"1\"";
            else
                  // LVIFIX hack: force label to "coda" to match to coda label
                  // sound = "coda=\"" + m->label() + "\"";
                  sound = "coda=\"coda\"";
            }
      else if (mtp == MarkerType::SEGNO) {
            type = "segno";
            if (m->label() == "")
                  sound = "segno=\"1\"";
            else
                  sound = "segno=\"" + m->label() + "\"";
            }
      else if (mtp == MarkerType::FINE) {
            words = "Fine";
            sound = "fine=\"yes\"";
            }
      else if (mtp == MarkerType::TOCODA) {
            if (m->text() == "")
                  words = "To Coda";
            else
                  words = m->text();
            if (m->label() == "")
                  sound = "tocoda=\"1\"";
            else
                  sound = "tocoda=\"" + m->label() + "\"";
            }
      else
            qDebug("marker type=%d not implemented\n", mtp);
      if (sound != "") {
            xml.stag("direction placement=\"above\"");
            xml.stag("direction-type");
            if (type != "") xml.tagE(type);
            if (words != "") xml.tag("words", words);
            xml.etag();
            if (sound != "") xml.tagE(QString("sound ") + sound);
            xml.etag();
            }
      }

//---------------------------------------------------------
//  findTrackForAnnotations
//---------------------------------------------------------

// An annotation is attched to the staff, with track set
// to the lowest track in the staff. Find a track for it
// (the lowest track in this staff that has a chord or rest)

static int findTrackForAnnotations(int track, Segment* seg)
      {
      if (seg->segmentType() != Segment::SegChordRest)
            return -1;

      int staff = track / VOICES;
      int strack = staff * VOICES;      // start track of staff containing track
      int etrack = strack + VOICES;     // end track of staff containing track + 1

      for (int i = strack; i < etrack; i++)
            if (seg->element(i))
                  return i;

      return -1;
      }

//---------------------------------------------------------
//  repeatAtMeasureStart -- write repeats at begin of measure
//---------------------------------------------------------

static void repeatAtMeasureStart(Xml& xml, Attributes& attr, Measure* m, int strack, int etrack, int track)
      {
      // loop over all segments
      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            if (seg->segmentType() == Segment::SegChordRest) {
                  foreach(const Element* e, seg->annotations()) {
#ifdef DEBUG_REPEATS
                        qDebug("repeatAtMeasureStart seg %p elem %p type %d (%s) track %d",
                               seg, e, e->type(), qPrintable(e->subtypeName()), e->track());
#endif
                        int wtrack = -1; // track to write jump
                        if (strack <= e->track() && e->track() < etrack)
                              wtrack = findTrackForAnnotations(e->track(), seg);
                        if (track == wtrack) {
                              switch (e->type()) {
                                    case Element::SYMBOL:
                                    case Element::TEMPO_TEXT:
                                    case Element::STAFF_TEXT:
                                    case Element::TEXT:
                                    case Element::DYNAMIC:
                                    case Element::HARMONY:
                                    case Element::FIGURED_BASS:
                                    case Element::REHEARSAL_MARK:
                                    case Element::FRET_DIAGRAM:
                                    case Element::JUMP: // note: all jumps are handled at measure stop
                                          break;
                                    case Element::MARKER:
                                          {
                                          // filter out the markers at measure Start
                                          const Marker* const mk = static_cast<const Marker* const>(e);
                                          MarkerType mtp = mk->markerType();
#ifdef DEBUG_REPEATS
                                          qDebug("repeatAtMeasureStart: marker type %d", mtp);
#endif
                                          if (   mtp == MarkerType::SEGNO
                                                 || mtp == MarkerType::CODA
                                                 ) {
                                                qDebug(" -> handled");
                                                attr.doAttr(xml, false);
                                                directionMarker(xml, mk);
                                                }
                                          else if (   mtp == MarkerType::FINE
                                                      || mtp == MarkerType::TOCODA
                                                      ) {
#ifdef DEBUG_REPEATS
                                                qDebug(" -> ignored");
#endif
                                                // ignore
                                                }
                                          else {
#ifdef DEBUG_REPEATS
                                                qDebug(" -> not implemented");
#endif
                                                qDebug("repeatAtMeasureStart: marker %d not implemented", mtp);
                                                }
                                          }
                                          break;
                                    default:
                                          qDebug("repeatAtMeasureStart: direction type %s at tick %d not implemented",
                                                 Element::name(e->type()), seg->tick());
                                          break;
                                    }
                              }
                        else {
#ifdef DEBUG_REPEATS
                              qDebug("repeatAtMeasureStart: no track found");
#endif
                              }
                        } // foreach
                  }
            }
      }

//---------------------------------------------------------
//  repeatAtMeasureStop -- write repeats at end of measure
//---------------------------------------------------------

static void repeatAtMeasureStop(Xml& xml, Measure* m, int strack, int etrack, int track)
      {
      // loop over all segments
      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            if (seg->segmentType() == Segment::SegChordRest) {
                  foreach(const Element* e, seg->annotations()) {
#ifdef DEBUG_REPEATS
                        qDebug("repeatAtMeasureStop seg %p elem %p type %d (%s) track %d",
                               seg, e, e->type(), qPrintable(e->subtypeName()), e->track());
#endif
                        int wtrack = -1; // track to write jump
                        if (strack <= e->track() && e->track() < etrack)
                              wtrack = findTrackForAnnotations(e->track(), seg);
                        if (track == wtrack) {
                              switch (e->type()) {
                                    case Element::SYMBOL:
                                    case Element::TEMPO_TEXT:
                                    case Element::STAFF_TEXT:
                                    case Element::TEXT:
                                    case Element::DYNAMIC:
                                    case Element::HARMONY:
                                    case Element::FIGURED_BASS:
                                    case Element::REHEARSAL_MARK:
                                    case Element::FRET_DIAGRAM:
                                          break;
                                    case Element::MARKER:
                                          {
                                          // filter out the markers at measure stop
                                          const Marker* const mk = static_cast<const Marker* const>(e);
                                          MarkerType mtp = mk->markerType();
#ifdef DEBUG_REPEATS
                                          qDebug("repeatAtMeasureStop: marker type %d", mtp);
#endif
                                          if (   mtp == MarkerType::FINE
                                                 || mtp == MarkerType::TOCODA
                                                 ) {
#ifdef DEBUG_REPEATS
                                                qDebug(" -> handled");
#endif
                                                directionMarker(xml, mk);
                                                }
                                          else if (   mtp == MarkerType::SEGNO
                                                      || mtp == MarkerType::CODA
                                                      ) {
#ifdef DEBUG_REPEATS
                                                qDebug(" -> ignored");
#endif
                                                // ignore
                                                }
                                          else {
#ifdef DEBUG_REPEATS
                                                qDebug(" -> not implemented");
#endif
                                                qDebug("repeatAtMeasureStop: marker %d not implemented", mtp);
                                                }
                                          }
                                          break;
                                    case Element::JUMP:
                                          directionJump(xml, static_cast<const Jump* const>(e));
                                          break;
                                    default:
                                          qDebug("repeatAtMeasureStop: direction type %s at tick %d not implemented",
                                                 Element::name(e->type()), seg->tick());
                                          break;
                                    }
                              }
                        else {
#ifdef DEBUG_REPEATS
                              qDebug("repeatAtMeasureStop: no track found");
#endif
                              }
                        } // foreach
                  }
            }
      }

//---------------------------------------------------------
//  work -- write the <work> element
//  note that order must be work-number, work-title
//  also write <movement-number> and <movement-title>
//  data is taken from the score metadata instead of the Text elements
//---------------------------------------------------------

void ExportMusicXml::work(const MeasureBase* /*measure*/)
      {
      QString workTitle  = _score->metaTag("workTitle");
      QString workNumber = _score->metaTag("workNumber");
      if (!(workTitle.isEmpty() && workNumber.isEmpty())) {
            xml.stag("work");
            if (!workNumber.isEmpty())
                  xml.tag("work-number", workNumber);
            if (!workTitle.isEmpty())
                  xml.tag("work-title", workTitle);
            xml.etag();
            }
      if (!_score->metaTag("movementNumber").isEmpty())
            xml.tag("movement-number", _score->metaTag("movementNumber"));
      if (!_score->metaTag("movementTitle").isEmpty())
            xml.tag("movement-title", _score->metaTag("movementTitle"));
      }

#if 0
//---------------------------------------------------------
//   elementRighter // used for harmony order
//---------------------------------------------------------

static bool elementRighter(const Element* e1, const Element* e2)
      {
      return e1->x() < e2->x();
      }
#endif

//---------------------------------------------------------
//  measureStyle -- write measure-style
//---------------------------------------------------------

// this is done at the first measure of a multi-meaure rest
// note: the measure count is stored in the last measure
// see measure.h _multiMeasure

static void measureStyle(Xml& xml, Attributes& attr, Measure* m)
      {
      if (m->isMMRest()) {
            attr.doAttr(xml, true);
            xml.stag("measure-style");
            xml.tag("multiple-rest", 3); // TODO MM    m->multiMeasure());
            xml.etag();
            }
      }

//---------------------------------------------------------
//  findFretDiagram
//---------------------------------------------------------

static const FretDiagram* findFretDiagram(int strack, int etrack, int track, Segment* seg)
      {
      if (seg->segmentType() == Segment::SegChordRest) {
            foreach(const Element* e, seg->annotations()) {

                  int wtrack = -1; // track to write annotation

                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack && e->type() == Element::FRET_DIAGRAM)
                        return static_cast<const FretDiagram*>(e);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//  annotations
//---------------------------------------------------------

// In MuseScore, Element::FRET_DIAGRAM and Element::HARMONY are separate annotations,
// in MusicXML they are combined in the harmony element. This means they have to be matched.
// TODO: replace/repair current algorithm (which can only handle one FRET_DIAGRAM and one HARMONY)

static void annotations(ExportMusicXml* exp, Xml& xml, int strack, int etrack, int track, int sstaff, Segment* seg)
      {
      if (seg->segmentType() == Segment::SegChordRest) {

            const FretDiagram* fd = findFretDiagram(strack, etrack, track, seg);
            // if (fd) qDebug("annotations seg %p found fret diagram %p", seg, fd);

            foreach(const Element* e, seg->annotations()) {

                  int wtrack = -1; // track to write annotation

                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack) {
                        switch (e->type()) {
                              case Element::SYMBOL:
                                    exp->symbol(static_cast<const Symbol*>(e), sstaff);
                                    break;
                              case Element::TEMPO_TEXT:
                                    exp->tempoText(static_cast<const TempoText*>(e), sstaff);
                                    break;
                              case Element::STAFF_TEXT:
                              case Element::TEXT:
                                    exp->words(static_cast<const Text*>(e), sstaff);
                                    break;
                              case Element::DYNAMIC:
                                    exp->dynamic(static_cast<const Dynamic*>(e), sstaff);
                                    break;
                              case Element::HARMONY:
                                    // qDebug("annotations seg %p found harmony %p", seg, e);
                                    exp->harmony(static_cast<const Harmony*>(e), fd /*, sstaff */);
                                    fd = 0; // make sure to write only once ...
                                    break;
                              case Element::REHEARSAL_MARK:
                                    exp->rehearsal(static_cast<const RehearsalMark*>(e), sstaff);
                                    break;
                              case Element::FIGURED_BASS: // handled separately by figuredBass()
                              case Element::FRET_DIAGRAM: // handled using findFretDiagram()
                              case Element::JUMP:         // ignore
                                    break;
                              default:
                                    qDebug("annotations: direction type %s at tick %d not implemented\n",
                                           Element::name(e->type()), seg->tick());
                                    break;
                              }
                        }
                  } // foreach
            if (fd)
                  // found fd but no harmony, cannot write (MusicXML would be invalid)
                  qDebug("annotations seg %p found fret diagram %p w/o harmony: cannot write",
                         seg, fd);
            }
      }

//---------------------------------------------------------
//  figuredBass
//---------------------------------------------------------

static void figuredBass(Xml& xml, int strack, int etrack, int track, const ChordRest* cr, FigBassMap& fbMap)
      {
      Segment* seg = cr->segment();
      if (seg->segmentType() == Segment::SegChordRest) {
            foreach(const Element* e, seg->annotations()) {

                  int wtrack = -1; // track to write annotation

                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack) {
                        if (e->type() == Element::FIGURED_BASS) {
                              const FiguredBass* fb = static_cast<const FiguredBass*>(e);
                              //qDebug("figuredbass() track %d seg %p fb %p seg %p tick %d ticks %d cr %p tick %d ticks %d",
                              //       track, seg, fb, fb->segment(), fb->segment()->tick(), fb->ticks(), cr, cr->tick(), cr->actualTicks());
                              bool extend = fb->ticks() > cr->actualTicks();
                              if (extend) {
                                    //qDebug("figuredbass() extend to %d + %d = %d",
                                    //       cr->tick(), fb->ticks(), cr->tick() + fb->ticks());
                                    fbMap.insert(strack, fb);
                                    }
                              else
                                    fbMap.remove(strack);
                              fb->writeMusicXML(xml, true, extend);
                              // there can only be one FB, if one was found
                              // no extend can be pending
                              return;
                              }
                        }
                  }
            // check for extend pending
            if (fbMap.contains(strack)) {
                  const FiguredBass* fb = fbMap.value(strack);
                  int endTick = fb->segment()->tick() + fb->ticks();
                  if (cr->tick() < endTick) {
                        //qDebug("figuredbass() at tick %d extend only", cr->tick());
                        // write figured bass element with extend only
                        fb->writeMusicXML(xml, false, true);
                        }
                  if (endTick <= cr->tick() + cr->actualTicks()) {
                        //qDebug("figuredbass() at tick %d extend done", cr->tick() + cr->actualTicks());
                        fbMap.remove(strack);
                        }
                  }
            }
      }

//---------------------------------------------------------
//  spannerStart
//---------------------------------------------------------

// for each spanner start:
// find start track
// find stop track
// if stop track < start track
//   get data from list of already stopped spanners
// else
//   calculate data
// write start if in right track

static void spannerStart(ExportMusicXml* exp, int strack, int etrack, int track, int sstaff, Segment* seg)
      {
      if (seg->segmentType() == Segment::SegChordRest) {
            int stick = seg->tick();
            for (auto it = exp->score()->spanner().lower_bound(stick); it != exp->score()->spanner().upper_bound(stick); ++it) {
                  Spanner* e = it->second;

                  int wtrack = -1; // track to write spanner
                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack) {
                        switch (e->type()) {
                              case Element::HAIRPIN:
                                    exp->hairpin(static_cast<const Hairpin*>(e), sstaff, seg->tick());
                                    break;
                              case Element::OTTAVA:
                                    exp->ottava(static_cast<const Ottava*>(e), sstaff, seg->tick());
                                    break;
                              case Element::PEDAL:
                                    exp->pedal(static_cast<const Pedal*>(e), sstaff, seg->tick());
                                    break;
                              case Element::TEXTLINE:
                                    exp->textLine(static_cast<const TextLine*>(e), sstaff, seg->tick());
                                    break;
                              case Element::TRILL:
                                    // ignore (written as <note><notations><ornaments><wavy-line>
                                    break;
                              default:
                                    qDebug("spannerStart: direction type %s at tick %d not implemented\n",
                                           Element::name(e->type()), seg->tick());
                                    break;
                              }
                        }
                  } // foreach
            }
      }

//---------------------------------------------------------
//  spannerStop
//---------------------------------------------------------

// see spanner start

static void spannerStop(ExportMusicXml* exp, int strack, int etrack, int track, int sstaff, Segment* seg)
      {
       for (auto it : exp->score()->spanner()) {
            Spanner* e = it.second;
            if (e->tick2() != seg->tick())
                  continue;

            int wtrack = -1; // track to write spanner

            if (strack <= e->track() && e->track() < etrack)
                  wtrack = findTrackForAnnotations(e->track(), seg);

            if (track == wtrack) {
                  switch (e->type()) {
                        case Element::HAIRPIN:
                              exp->hairpin(static_cast<const Hairpin*>(e), sstaff, -1);
                              break;
                        case Element::OTTAVA:
                              exp->ottava(static_cast<const Ottava*>(e), sstaff, -1);
                              break;
                        case Element::PEDAL:
                              exp->pedal(static_cast<const Pedal*>(e), sstaff, -1);
                              break;
                        case Element::TEXTLINE:
                              exp->textLine(static_cast<const TextLine*>(e), sstaff, -1);
                              break;
                        case Element::TRILL:
                              // ignore (written as <note><notations><ornaments><wavy-line>
                              break;
                        default:
                              qDebug("spannerStop: direction type %s at tick %d not implemented\n",
                                     Element::name(e->type()), seg->tick());
                              break;
                        }
                  }
            } // foreach
      }

//---------------------------------------------------------
//  keysigTimesig
//---------------------------------------------------------

/**
 Output attributes at start of measure: key, time
 */

void ExportMusicXml::keysigTimesig(Measure* m, int strack, int etrack)
      {
      // search all staves for non-generated key signatures
      QMap<int, KeySig*> keysigs; // map staff to key signature
      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            if (seg->tick() > m->tick())
                  break;
            for (int t = strack; t < etrack; t += VOICES) {
                  Element* el = seg->element(t);
                  if (!el)
                        continue;
                  if (el->type() == Element::KEYSIG) {
                        int st = (t - strack) / VOICES;
                        if (!el->generated())
                              keysigs[st] = static_cast<KeySig*>(el);
                        }
                  }
            }

      // write the key signatues
      if (!keysigs.isEmpty()) {
            // determine if all staves have a keysig and all keysigs are identical
            // in that case a single <key> is written, without number=... attribute
            int nstaves = (etrack - strack) / VOICES;
            bool singleKey = true;
            // check if all staves have a keysig
            for (int i = 0; i < nstaves; i++)
                  if (!keysigs.contains(i))
                        singleKey = false;
            // check if all keysigs are identical
            if (singleKey)
                  for (int i = 1; i < nstaves; i++)
                        if (!(keysigs.value(i)->keySignature() == keysigs.value(0)->keySignature()))
                              singleKey = false;

            // write the keysigs
            if (singleKey) {
                  // keysig applies to all staves
                  keysig(keysigs.value(0)->keySignature(), 0, keysigs.value(0)->visible());
                  }
            else {
                  // staff-specific keysigs
                  foreach(int st, keysigs.keys())
                  keysig(keysigs.value(st)->keySignature(), st + 1, keysigs.value(st)->visible());
                  }
            }
      else {
            // always write a keysig at tick = 0
            if (m->tick() == 0)
                  keysig(0);
            }

      TimeSig* tsig = 0;
      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            if (seg->tick() > m->tick())
                  break;
            Element* el = seg->element(strack);
            if (el && el->type() == Element::TIMESIG)
                  tsig = (TimeSig*) el;
            }
      if (tsig)
            timesig(tsig);
      }

//---------------------------------------------------------
//  identification -- write the identification
//---------------------------------------------------------

static void identification(Xml& xml, Score const* const score)
      {
      xml.stag("identification");

      QStringList creators;
      // the creator types commonly found in MusicXML
      creators << "arranger" << "composer" << "lyricist" << "poet" << "translator";
      foreach (QString type, creators) {
            QString creator = score->metaTag(type);
            if (!creator.isEmpty())
                  xml.tag(QString("creator type=\"%1\"").arg(type), creator);
            }

      if (!score->metaTag("copyright").isEmpty())
            xml.tag("rights", score->metaTag("copyright"));

      xml.stag("encoding");
      if (MScore::debugMode) {
            xml.tag("software", QString("MuseScore 0.7.0"));
            xml.tag("encoding-date", QString("2007-09-10"));
            }
      else {
            xml.tag("software", QString("MuseScore ") + QString(VERSION));
            xml.tag("encoding-date", QDate::currentDate().toString(Qt::ISODate));
            }
      xml.etag();

      if (!score->metaTag("source").isEmpty())
            xml.tag("source", score->metaTag("source"));

      xml.etag();
      }

//---------------------------------------------------------
//  findPartGroupNumber
//---------------------------------------------------------

static int findPartGroupNumber(int* partGroupEnd)
      {
      // find part group number
      for (int number = 0; number < MAX_PART_GROUPS; ++number)
            if (partGroupEnd[number] == -1)
                  return number;
      qDebug("no free part group number");
      return MAX_PART_GROUPS;
      }
            
//---------------------------------------------------------
//  write
//---------------------------------------------------------

/**
 Write the score to \a dev in MusicXML format.
 */

void ExportMusicXml::write(QIODevice* dev)
      {

      calcDivisions();

      for (int i = 0; i < MAX_BRACKETS; ++i)
            bracket[i] = 0;

      xml.setDevice(dev);
      xml.setCodec("UTF-8");
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml << "<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 3.0 Partwise//EN\" \"http://www.musicxml.org/dtds/partwise.dtd\">\n";
      xml.stag("score-partwise");

      const MeasureBase* measure = _score->measures()->first();
      work(measure);

      identification(xml, _score);

      if (preferences.musicxmlExportLayout) {
            defaults(xml, _score, millimeters, tenths);
            credits(xml);
            }

      xml.stag("part-list");
      const QList<Part*>& il = _score->parts();
      int staffCount = 0;                       // count sum of # staves in parts
      int partGroupEnd[MAX_PART_GROUPS];        // staff where part group ends (bracketSpan is in staves, not parts)
      for (int i = 0; i < MAX_PART_GROUPS; i++)
            partGroupEnd[i] = -1;
      for (int idx = 0; idx < il.size(); ++idx) {
            Part* part = il.at(idx);
            bool bracketFound = false;
            // handle brackets
            for (int i = 0; i < part->nstaves(); i++) {
                  Staff* st = part->staff(i);
                  if (st) {
                        for (int j = 0; j < st->bracketLevels(); j++) {
                              if (st->bracket(j) != NO_BRACKET) {
                                    bracketFound = true;
                                    if (i == 0) {
                                          // OK, found bracket in first staff of part
                                          // filter out implicit brackets
                                          if (!(st->bracketSpan(j) == part->nstaves()
                                                && st->bracket(j) == BRACKET_BRACE)) {
                                                // add others
                                                int number = findPartGroupNumber(partGroupEnd);
                                                if (number < MAX_PART_GROUPS) {
                                                      partGroupStart(xml, number + 1, st->bracket(j));
                                                      partGroupEnd[number] = staffCount + st->bracketSpan(j);
                                                      }
                                                }
                                          }
                                    else {
                                          // bracket in other staff not supported in MusicXML
                                          qDebug("bracket starting in staff %d not supported\n", i + 1);
                                          }
                                    }
                              }
                        }
                  }
            // handle bracket none
            if (!bracketFound && part->nstaves() > 1) {
                  int number = findPartGroupNumber(partGroupEnd);
                  if (number < MAX_PART_GROUPS) {
                        partGroupStart(xml, number + 1, NO_BRACKET);
                        partGroupEnd[number] = idx + part->nstaves();
                        }
                  }

            xml.stag(QString("score-part id=\"P%1\"").arg(idx+1));
            xml.tag("part-name", part->longName().toPlainText());
            if (!part->shortName().isEmpty())
                  xml.tag("part-abbreviation", part->shortName().toPlainText());

            if (part->instr()->useDrumset()) {
                  Drumset* drumset = part->instr()->drumset();
                  for (int i = 0; i < 128; ++i) {
                        DrumInstrument di = drumset->drum(i);
                        if (di.notehead >= 0) {
                              xml.stag(QString("score-instrument id=\"P%1-I%2\"").arg(idx+1).arg(i + 1));
                              xml.tag("instrument-name", di.name);
                              xml.etag();
                              }
                        }
                  for (int i = 0; i < 128; ++i) {
                        DrumInstrument di = drumset->drum(i);
                        if (di.notehead >= 0) {
                              xml.stag(QString("midi-instrument id=\"P%1-I%2\"").arg(idx+1).arg(i + 1));
                              if (part->midiChannel() >= 0) // <0 is not valid
                                    xml.tag("midi-channel", part->midiChannel() + 1);
                              if (part->midiProgram() >= 0) // <0 is not valid
                                    xml.tag("midi-program", part->midiProgram() + 1);
                              xml.tag("midi-unpitched", i + 1);
                              xml.tag("volume", (part->volume() / 127.0) * 100);  //percent
                              xml.tag("pan", ((int)((part->pan() - 63.5) / 63.5)) * 90); //-90 hard left, +90 hard right
                              xml.etag();
                              }
                        }
                  }
            else {
                  xml.stag(QString("score-instrument id=\"P%1-I%2\"").arg(idx+1).arg(3));
                  xml.tag("instrument-name", part->longName().toPlainText());
                  xml.etag();

                  xml.stag(QString("midi-instrument id=\"P%1-I%2\"").arg(idx+1).arg(3));
                  if (part->midiChannel() >= 0) // <0 is not valid
                        xml.tag("midi-channel", part->midiChannel() + 1);
                  if (part->midiProgram() >= 0) // <0 is not valid
                        xml.tag("midi-program", part->midiProgram() + 1);
                  xml.tag("volume", (part->volume() / 127.0) * 100);  //percent
                  xml.tag("pan", ((int)((part->pan() - 63.5) / 63.5)) * 90); //-90 hard left, +90 hard right
                  xml.etag();
                  }

            xml.etag();
            staffCount += part->nstaves();
            for (int i = MAX_PART_GROUPS - 1; i >= 0; i--) {
                  int end = partGroupEnd[i];
                  if (end >= 0) {
                        if (staffCount >= end) {
                              xml.tagE("part-group type=\"stop\" number=\"%d\"", i + 1);
                              partGroupEnd[i] = -1;
                              }
                        }
                  }
            }
      xml.etag();

      staffCount = 0;

      for (int idx = 0; idx < il.size(); ++idx) {
            Part* part = il.at(idx);
            tick = 0;
            xml.stag(QString("part id=\"P%1\"").arg(idx+1));

            int staves = part->nstaves();
            int strack = _score->staffIdx(part) * VOICES;
            int etrack = strack + staves * VOICES;

            trillStart.clear();
            trillStop.clear();

            int measureNo = 1;          // number of next regular measure
            int irregularMeasureNo = 1; // number of next irregular measure
            int pickupMeasureNo = 1;    // number of next pickup measure

            FigBassMap fbMap;           // pending figure base extends

            for (MeasureBase* mb = _score->measures()->first(); mb; mb = mb->next()) {
                  if (mb->type() != Element::MEASURE)
                        continue;
                  Measure* m = static_cast<Measure*>(mb);
                  const PageFormat* pf = _score->pageFormat();


                  // pickup and other irregular measures need special care
                  QString measureTag = "measure number=";
                  if ((irregularMeasureNo + measureNo) == 2 && m->irregular()) {
                        measureTag += "\"0\" implicit=\"yes\"";
                        pickupMeasureNo++;
                        }
                  else if (m->irregular())
                        measureTag += QString("\"X%1\" implicit=\"yes\"").arg(irregularMeasureNo++);
                  else
                        measureTag += QString("\"%1\"").arg(measureNo++);
                  if (preferences.musicxmlExportLayout)
                        measureTag += QString(" width=\"%1\"").arg(QString::number(m->bbox().width() / MScore::DPMM / millimeters * tenths,'f',2));
                  xml.stag(measureTag);

                  // Handle the <print> element.
                  // When exporting layout and all breaks, a <print> with layout informations
                  // is generated for the measure types TopSystem, NewSystem and newPage.
                  // When exporting layout but only manual or no breaks, a <print> with
                  // layout informations is generated only for the measure type TopSystem,
                  // as it is assumed the system layout is broken by the importing application
                  // anyway and is thus useless.

                  int currentSystem = NoSystem;
                  Measure* previousMeasure = 0;

                  for (MeasureBase* currentMeasureB = m->prev(); currentMeasureB; currentMeasureB = currentMeasureB->prev()) {
                        if (currentMeasureB->type() == Element::MEASURE) {
                              previousMeasure = (Measure*) currentMeasureB;
                              break;
                              }
                        }

                  if (!previousMeasure)
                        currentSystem = TopSystem;
                  else if (m->parent()->parent() != previousMeasure->parent()->parent())
                        currentSystem = NewPage;
                  else if (m->parent() != previousMeasure->parent())
                        currentSystem = NewSystem;

                  bool prevMeasLineBreak = false;
                  bool prevMeasPageBreak = false;
                  if (previousMeasure) {
                        prevMeasLineBreak = previousMeasure->lineBreak();
                        prevMeasPageBreak = previousMeasure->pageBreak();
                        }

                  if (currentSystem != NoSystem) {

                        // determine if a new-system or new-page is required
                        QString newThing; // new-[system|page]="yes" or empty
                        if (preferences.musicxmlExportBreaks == ALL_BREAKS) {
                              if (currentSystem == NewSystem)
                                    newThing = " new-system=\"yes\"";
                              else if (currentSystem == NewPage)
                                    newThing = " new-page=\"yes\"";
                              }
                        else if (preferences.musicxmlExportBreaks == MANUAL_BREAKS) {
                              if (currentSystem == NewSystem && prevMeasLineBreak)
                                    newThing = " new-system=\"yes\"";
                              else if (currentSystem == NewPage && prevMeasPageBreak)
                                    newThing = " new-page=\"yes\"";
                              }

                        // determine if layout information is required
                        bool doLayout = false;
                        if (preferences.musicxmlExportLayout) {
                              if (currentSystem == TopSystem
                                  || (preferences.musicxmlExportBreaks == ALL_BREAKS && newThing != "")) {
                                    doLayout = true;
                                    }
                              }

                        if (doLayout) {
                              xml.stag(QString("print%1").arg(newThing));
                              const double pageWidth  = getTenthsFromInches(pf->size().width());
                              const double lm = getTenthsFromInches(pf->oddLeftMargin());
                              const double rm = getTenthsFromInches(pf->oddRightMargin());
                              const double tm = getTenthsFromInches(pf->oddTopMargin());

                              // System Layout
                              // Put the system print suggestions only for the first part in a score...
                              if (idx == 0) {
                                    // Find the right margin of the system.
                                    double systemLM = getTenthsFromDots(m->pagePos().x() - m->system()->page()->pagePos().x()) - lm;
                                    double systemRM = pageWidth - rm - (getTenthsFromDots(m->system()->bbox().width()) + lm);

                                    xml.stag("system-layout");
                                    xml.stag("system-margins");
                                    xml.tag("left-margin", QString("%1").arg(QString::number(systemLM,'f',2)));
                                    xml.tag("right-margin", QString("%1").arg(QString::number(systemRM,'f',2)) );
                                    xml.etag();

                                    if (currentSystem == NewPage || currentSystem == TopSystem)
                                          xml.tag("top-system-distance", QString("%1").arg(QString::number(getTenthsFromDots(m->pagePos().y()) - tm,'f',2)) );
                                    if (currentSystem == NewSystem)
                                          xml.tag("system-distance", QString("%1").arg(QString::number(getTenthsFromDots(m->pagePos().y() - previousMeasure->pagePos().y() - previousMeasure->bbox().height()),'f',2)));

                                    xml.etag();
                                    }

                              // Staff layout elements.
                              for (int staffIdx = (staffCount == 0) ? 1 : 0; staffIdx < staves; staffIdx++) {
                                    xml.stag(QString("staff-layout number=\"%1\"").arg(staffIdx + 1));
                                    xml.tag("staff-distance", QString("%1").arg(QString::number(getTenthsFromDots(mb->system()->staff(staffCount + staffIdx - 1)->distanceDown()),'f',2)));
                                    xml.etag();
                                    }

                              xml.etag();
                              }
                        else {
                              // !doLayout
                              if (newThing != "")
                                    xml.tagE(QString("print%1").arg(newThing));
                              }

                        } // if (currentSystem ...

                  attr.start();

                  findTrills(m, strack, etrack, trillStart, trillStop);

                  // barline left must be the first element in a measure
                  barlineLeft(m);

                  // output attributes with the first actual measure (pickup or regular)
                  if ((irregularMeasureNo + measureNo + pickupMeasureNo) == 4) {
                        attr.doAttr(xml, true);
                        xml.tag("divisions", MScore::division / div);
                        }
                  // output attributes at start of measure: key, time
                  keysigTimesig(m, strack, etrack);
                  // output attributes with the first actual measure (pickup or regular) only
                  if ((irregularMeasureNo + measureNo + pickupMeasureNo) == 4) {
                        if (staves > 1)
                              xml.tag("staves", staves);
                        }

                  {
                  Measure* prevMeasure = m->prevMeasure();
                  int tick             = m->tick();
                  Segment* cs1;
                  Segment* cs2         = m->findSegment(Segment::SegClef, tick);
                  Segment* seg         = 0;

                  if (prevMeasure)
                        cs1 = prevMeasure->findSegment(Segment::SegClef,  tick);
                  else
                        cs1 = 0;

                  if (cs1 && cs2)         // should not happen
                        seg = cs2;
                  else if (cs1)
                        seg = cs1;
                  else
                        seg = cs2;

                  // output attribute at start of measure: clef
                  if (seg) {
                        for (int st = strack; st < etrack; st += VOICES) {
                              // sstaff - xml staff number, counting from 1 for this
                              // instrument
                              // special number 0 -> dont show staff number in
                              // xml output (because there is only one staff)

                              int sstaff = (staves > 1) ? st - strack + VOICES : 0;
                              sstaff /= VOICES;

                              Clef* cle = static_cast<Clef*>(seg->element(st));
                              if (cle) {
                                    ClefType ct = cle->clefType();
                                    clefDebug("exportxml: clef at start measure ti=%d ct=%d gen=%d", tick, int(ct), cle->generated());
                                    // output only clef changes, not generated clefs at line beginning
                                    // exception: at tick=0, export clef anyway
                                    if (tick == 0 || !cle->generated()) {
                                          clefDebug("exportxml: clef exported");
                                          clef(sstaff, ct);
                                          }
                                    else {
                                          clefDebug("exportxml: clef not exported");
                                          }
                                    }
                              }
                        }
                  }

                  // output attributes with the first actual measure (pickup or regular) only
                  if ((irregularMeasureNo + measureNo + pickupMeasureNo) == 4) {
                        const Instrument* instrument = part->instr();

                        // staff details
                        // TODO: decide how to handle linked regular / TAB staff
                        //       currently exported as a two staff part ...
                        for (int i = 0; i < staves; i++) {
                              Staff* st = part->staff(i);
                              if (st->lines() != 5) {
                                    if (staves > 1)
                                          xml.stag(QString("staff-details number=\"%1\"").arg(i+1));
                                    else
                                          xml.stag("staff-details");
                                    xml.tag("staff-lines", st->lines());
                                    if (st->isTabStaff() && instrument->stringData()) {
                                          QList<int> l = instrument->stringData()->stringList();
                                          for (int i = 0; i < l.size(); i++) {
                                                char step  = ' ';
                                                int alter  = 0;
                                                int octave = 0;
                                                midipitch2xml(l.at(i), step, alter, octave);
                                                xml.stag(QString("staff-tuning line=\"%1\"").arg(i+1));
                                                xml.tag("tuning-step", QString("%1").arg(step));
                                                if (alter)
                                                      xml.tag("tuning-alter", alter);
                                                xml.tag("tuning-octave", octave);
                                                xml.etag();
                                                }
                                          }
                                    xml.etag();
                                    }
                              }
                        // instrument details
                        if (instrument->transpose().chromatic) {
                              xml.stag("transpose");
                              xml.tag("diatonic",  instrument->transpose().diatonic);
                              xml.tag("chromatic", instrument->transpose().chromatic);
                              xml.etag();
                              }
                        }

                  // output attribute at start of measure: measure-style
                  measureStyle(xml, attr, m);

                  // MuseScore limitation: repeats are always in the first part
                  // and are implicitly placed at either measure start or stop
                  if (idx == 0)
                        repeatAtMeasureStart(xml, attr, m, strack, etrack, strack);

                  for (int st = strack; st < etrack; ++st) {
                        // sstaff - xml staff number, counting from 1 for this
                        // instrument
                        // special number 0 -> dont show staff number in
                        // xml output (because there is only one staff)

                        int sstaff = (staves > 1) ? st - strack + VOICES : 0;
                        sstaff /= VOICES;
                        for (Segment* seg = m->first(); seg; seg = seg->next()) {
                              Element* el = seg->element(st);
                              if (!el) {
                                    continue;
                                    }
                              // must ignore start repeat to prevent spurious backup/forward
                              if (el->type() == Element::BAR_LINE && static_cast<BarLine*>(el)->barLineType() == START_REPEAT)
                                    continue;

                              // generate backup or forward to the start time of the element
                              // but not for breath, which has the same start time as the
                              // previous note, while tick is already at the end of that note
                              if (tick != seg->tick()) {
                                    attr.doAttr(xml, false);
                                    if (el->type() != Element::BREATH)
                                          moveToTick(seg->tick());
                                    }

                              // handle annotations and spanners (directions attached to this note or rest)
                              if (el->isChordRest()) {
                                    attr.doAttr(xml, false);
                                    annotations(this, xml, strack, etrack, st, sstaff, seg);
                                    // look for more harmony
                                    for (Segment* seg1 = seg->next(); seg1; seg1 = seg1->next()) {
                                          if(seg1->isChordRest()) {
                                                Element* el1 = seg1->element(st);
                                                if (el1) // found a ChordRest, next harmony will be attach to this one
                                                      break;
                                                foreach (Element* annot, seg1->annotations()) {
                                                      if(annot->type() == Element::HARMONY && annot->track() == st)
                                                            harmony(static_cast<Harmony*>(annot), 0, (seg1->tick() - seg->tick()) / div);
                                                      }
                                                }
                                          }
                                    figuredBass(xml, strack, etrack, st, static_cast<const ChordRest*>(el), fbMap);
                                    spannerStop(this, strack, etrack, st, sstaff, seg);
                                    spannerStart(this, strack, etrack, st, sstaff, seg);
                                    }

                              switch (el->type()) {

                                    case Element::CLEF:
                                          {
                                          // output only clef changes, not generated clefs
                                          // at line beginning
                                          // also ignore clefs at the start of a measure,
                                          // these have already been output
                                          // also ignore clefs at the end of a measure
                                          //
                                          ClefType ct = ((Clef*)el)->clefType();
                                          int ti = seg->tick();
                                          clefDebug("exportxml: clef in measure ti=%d ct=%d gen=%d", ti, ct, el->generated());
                                          if (el->generated()) {
                                                clefDebug("exportxml: generated clef not exported");
                                                break;
                                                }
                                          if (!el->generated() && ti != m->tick() && ti != m->endTick())
                                                clef(sstaff, ct);
                                          else {
                                                clefDebug("exportxml: clef not exported");
                                                }
                                          }
                                          break;

                                    case Element::KEYSIG:
                                          // ignore
                                          break;

                                    case Element::TIMESIG:
                                          // ignore
                                          break;

                                    case Element::CHORD:
                                          {
                                          Chord* c                 = static_cast<Chord*>(el);
                                          const QList<Lyrics*>* ll = &c->lyricsList();
                                          for (Chord* g : c->graceNotes()) {
                                                chord(g, sstaff, ll, part->instr()->useDrumset());
                                                }
                                          chord(c, sstaff, ll, part->instr()->useDrumset());
                                          break;
                                          }
                                    case Element::REST:
                                          rest((Rest*)el, sstaff);
                                          break;

                                    case Element::BAR_LINE:
                                          // Following must be enforced (ref MusicXML barline.dtd):
                                          // If location is left, it should be the first element in the measure;
                                          // if location is right, it should be the last element.
                                          // implementation note: START_REPEAT already written by barlineLeft()
                                          // any bars left should be "middle"
                                          // TODO: print barline only if middle
                                          // if (el->subtype() != START_REPEAT)
                                          //       bar((BarLine*) el);
                                          break;
                                    case Element::BREATH:
                                          // ignore, already exported as note articulation
                                          break;

                                    default:
                                          qDebug("ExportMusicXml::write unknown segment type %s\n", el->name());
                                          break;
                                    }
                              } // for (Segment* seg = ...
                        attr.stop(xml);
                        } // for (int st = ...
                  // move to end of measure (in case of incomplete last voice)
#ifdef DEBUG_TICK
                  qDebug("end of measure");
#endif
                  moveToTick(m->tick() + m->ticks());
                  if (idx == 0)
                        repeatAtMeasureStop(xml, m, strack, etrack, strack);
                  // note: don't use "m->repeatFlags() & RepeatEnd" here, because more
                  // barline types need to be handled besides repeat end ("light-heavy")
                  barlineRight(m);
                  xml.etag();
                  }
            staffCount += staves;
            xml.etag();
            }

      xml.etag();
      }

//---------------------------------------------------------
//   saveXml
//    return false on error
//---------------------------------------------------------

/**
 Save Score as MusicXML file \a name.

 Return false on error.
 */

bool saveXml(Score* score, const QString& name)
      {
      QFile f(name);
      if (!f.open(QIODevice::WriteOnly))
            return false;
      ExportMusicXml em(score);
      em.write(&f);
      return f.error() == QFile::NoError;
      }


//---------------------------------------------------------
//   saveMxl
//    return false on error
//---------------------------------------------------------

/**
 Save Score as compressed MusicXML file \a name.

 Return false on error.
 */

// META-INF/container.xml:
// <?xml version="1.0" encoding="UTF-8"?>
// <container>
//     <rootfiles>
//         <rootfile full-path="testHello.xml"/>
//     </rootfiles>
// </container>

bool saveMxl(Score* score, const QString& name)
      {
      MQZipWriter uz(name);

      QFileInfo fi(name);
#if 0
      QDateTime dt;
      if (MScore::debugMode)
            dt = QDateTime(QDate(2007, 9, 10), QTime(12, 0));
      else
            dt = QDateTime::currentDateTime();
#endif
      QString fn = fi.completeBaseName() + ".xml";

      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      Xml xml;
      xml.setDevice(&cbuf);
      xml.setCodec("UTF-8");
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(fn));
      xml.etag();
      xml.etag();
      xml.etag();
      cbuf.seek(0);
      //uz.addDirectory("META-INF");
      uz.addFile("META-INF/container.xml", cbuf.data());

      QBuffer dbuf;
      dbuf.open(QIODevice::ReadWrite);
      ExportMusicXml em(score);
      em.write(&dbuf);
      dbuf.seek(0);
      uz.addFile(fn, dbuf.data());
      uz.close();
      return true;
      }

double ExportMusicXml::getTenthsFromInches(double inches)
      {
      return inches * INCH / millimeters * tenths;
      }

double ExportMusicXml::getTenthsFromDots(double dots)
      {
      return dots / MScore::DPMM / millimeters * tenths;
      }

//---------------------------------------------------------
//   harmony
//---------------------------------------------------------

void ExportMusicXml::harmony(Harmony const* const h, FretDiagram const* const fd, int offset)
      {
      double rx = h->userOff().x()*10;
      QString relative;
      if (rx > 0) {
            relative = QString(" relative-x=\"%1\"").arg(QString::number(rx,'f',2));
            }
      int rootTpc = h->rootTpc();
      if (rootTpc != INVALID_TPC) {
            if (h->hasFrame())
                  xml.stag(QString("harmony print-frame=\"yes\"").append(relative));
            else
                  xml.stag(QString("harmony print-frame=\"no\"").append(relative));
            xml.stag("root");
            xml.tag("root-step", tpc2stepName(rootTpc));
            int alter = tpc2alter(rootTpc);
            if (alter)
                  xml.tag("root-alter", alter);
            xml.etag();

            if (!h->xmlKind().isEmpty()) {
                  QString s = "kind";
                  if (h->xmlText() != "")
                        s += " text=\"" + h->xmlText() + "\"";
                  if (h->xmlSymbols() == "yes")
                        s += " use-symbols=\"yes\"";
                  if (h->xmlParens() == "yes")
                        s += " parentheses-degrees=\"yes\"";
                  xml.tag(s, h->xmlKind());
                  QStringList l = h->xmlDegrees();
                  if (!l.isEmpty()) {
                        foreach(QString tag, l) {
                              xml.stag("degree");
                              int alter = 0;
                              int idx = 3;
                              if (tag[idx] == '#') {
                                    alter = 1;
                                    ++idx;
                                    }
                              else if (tag[idx] == 'b') {
                                    alter = -1;
                                    ++idx;
                                    }
                              xml.tag("degree-value", tag.mid(idx));
                              xml.tag("degree-alter", alter);     // finale insists on this even if 0
                              if (tag.startsWith("add"))
                                    xml.tag("degree-type", "add");
                              else if (tag.startsWith("sub"))
                                    xml.tag("degree-type", "subtract");
                              else if (tag.startsWith("alt"))
                                    xml.tag("degree-type", "alter");
                              xml.etag();
                              }
                        }
                  }
            else {
                  if (h->extensionName() == 0)
                        xml.tag("kind", "");
                  else
                        xml.tag(QString("kind text=\"%1\"").arg(h->extensionName()), "");
                  }

            int baseTpc = h->baseTpc();
            if (baseTpc != INVALID_TPC) {
                  xml.stag("bass");
                  xml.tag("bass-step", tpc2stepName(baseTpc));
                  int alter = tpc2alter(baseTpc);
                  if (alter) {
                        xml.tag("bass-alter", alter);
                        }
                  xml.etag();
                  }
            if(offset > 0)
                  xml.tag("offset", offset);
            if (fd)
                  fd->writeMusicXML(xml);

            xml.etag();
            }
      else {
            //
            // export an unrecognized Chord
            // which may contain arbitrary text
            //
            if (h->hasFrame())
                  xml.stag(QString("harmony print-frame=\"yes\"").append(relative));
            else
                  xml.stag(QString("harmony print-frame=\"no\"").append(relative));
            xml.stag("root");
            xml.tag("root-step text=\"\"", "C");
            xml.etag();       // root
            QString k = "kind text=\"" + h->hTextName() + "\"";
            xml.tag(k, "none");
            xml.etag();       // harmony
#if 0
// prior to 2.0, MuseScore exported unrecognized chords as plain text
            xml.stag("direction");
            xml.stag("direction-type");
            xml.tag("words", h->text());
            xml.etag();
            xml.etag();
#endif
            }
#if 0
// this is very old code that may never have actually been used
      xml.tag(QString("kind text=\"%1\"").arg(h->extensionName()), extension);
      for (int i = 0; i < h->numberOfDegrees(); i++) {
            HDegree hd = h->degree(i);
            int tp = hd.type();
            if (tp == ADD || tp == ALTER || tp == SUBTRACT) {
                  xml.stag("degree");
                  xml.tag("degree-value", hd.value());
                  xml.tag("degree-alter", hd.alter());
                  switch (tp) {
                        case ADD:
                              xml.tag("degree-type", "add");
                              break;
                        case ALTER:
                              xml.tag("degree-type", "alter");
                              break;
                        case SUBTRACT:
                              xml.tag("degree-type", "subtract");
                              break;
                        default:
                              break;
                        }
                  xml.etag();
                  }
            }
#endif
      }

}

