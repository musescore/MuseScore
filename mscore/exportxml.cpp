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
#include "libmscore/stringdata.h"
#include "libmscore/rehearsalmark.h"
#include "thirdparty/qzip/qzipwriter_p.h"
#include "libmscore/fret.h"
#include "libmscore/tie.h"
#include "musicxmlfonthandler.h"

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
      void doSlurStart(Chord* chord, Notations& notations, Xml& xml, bool grace = false);
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
      void chord(Chord* chord, int staff, const QList<Lyrics*>* ll, DrumsetKind useDrumset);
      void rest(Rest* chord, int staff);
      void clef(int staff, ClefType clef);
      void timesig(TimeSig* tsig);
      void keysig(Key, int staff = 0, bool visible = true);
      void barlineLeft(Measure* m);
      void barlineRight(Measure* m);
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

static QString slurTieLineStyle(const SlurTie* s)
      {
      QString lineType;
      QString rest;
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
            rest = QString(" line-type=\"%1\"").arg(lineType);
      return rest;
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

void SlurHandler::doSlurStart(Chord* chord, Notations& notations, Xml& xml, bool grace)
      {
      // slurs on grace notes are not in spanner list, therefore:
      if (grace){
            foreach(Element* el, chord->el()){
                  if (el->type() == Element::Type::SLUR){
                        const Slur* s = static_cast<const Slur*>(el);
                        //define line type
                        QString rest = slurTieLineStyle(s);
                        if (chord->isGraceBefore()){
                             int i = findSlur(0);
                             if (i >= 0) {
                                   slur[i] = s;
                                   started[i] = true;
                                   notations.tag(xml);
                                   xml.tagE(QString("slur%1 type=\"start\" number=\"%2\"").arg(rest).arg(i + 1));
                                   }
                             else
                                   qDebug("no free slur slot");
                             }
                        else if (chord->isGraceAfter()){
                             int i = findSlur(s);
                             if (i >= 0) {
                                   // remove from list and print stop
                                   slur[i] = 0;
                                   started[i] = false;
                                   notations.tag(xml);
                                   xml.tagE(QString("slur%1 type=\"stop\"%2 number=\"%3\"").arg(rest).arg(s->slurDirection() == MScore::Direction::UP ? " placement=\"above\"" : "").arg(i + 1));
                                   }
                             }
                        }
                  }
             return;
            }
      // search for slur(s) starting at this chord
      int tick = chord->tick();
      auto sl = chord->score()->spanner();
      for (auto it = sl.lower_bound(tick); it != sl.upper_bound(tick); ++it) {
            Spanner* sp = it->second;
            if (sp->type() != Element::Type::SLUR || sp->track() != chord->track())
                  continue;
            const Slur* s = static_cast<const Slur*>(sp);
            // check if on slur list (i.e. stop already seen)
            int i = findSlur(s);
            //define line type
            QString rest = slurTieLineStyle(s);
            if (i >= 0) {
                  // remove from list and print start
                  slur[i] = 0;
                  started[i] = false;
                  notations.tag(xml);
                  xml.tagE(QString("slur%1 type=\"start\"%2 number=\"%3\"").arg(rest).arg(s->slurDirection() == MScore::Direction::UP ? " placement=\"above\"" : "").arg(i + 1));
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
            if (sp->type() != Element::Type::SLUR || sp->tick2() != chord->tick() || sp->track() != chord->track())
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
      // surch for slurs to grace notes after
      QList<Chord*> graceNotesAfter;
      chord->getGraceNotesAfter(&graceNotesAfter);
      for (Chord* g : graceNotesAfter) {
            foreach(Element* el, g->el()){
                  if (el->type() == Element::Type::SLUR){
                        const Slur* s = static_cast<const Slur*>(el);
                        //define line type
                        QString rest = slurTieLineStyle(s);
                        int i = findSlur(0);
                        if (i >= 0) {
                              slur[i] = s;
                              started[i] = true;
                              notations.tag(xml);
                              xml.tagE(QString("slur%1 type=\"start\" number=\"%2\"").arg(rest).arg(i + 1));
                              }
                        else
                              qDebug("no free slur slot"); break;
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
      Glissando::Type st = gli->glissandoType();
      switch (st) {
            case Glissando::Type::STRAIGHT:
                  notations.tag(xml);
                  xml.tagE("slide line-type=\"solid\" number=\"%d\" type=\"%s\"",
                           number, start ? "start" : "stop");
                  break;
            case Glissando::Type::WAVY:
                  notations.tag(xml);
                  xml.tagE("glissando line-type=\"wavy\" number=\"%d\" type=\"%s\"",
                           number, start ? "start" : "stop");
                  break;
            default:
                  qDebug("unknown glissando subtype %hhd", st);
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
      Glissando::Type st = chord->glissando()->glissandoType();
      if (st != Glissando::Type::STRAIGHT && st != Glissando::Type::WAVY) {
            qDebug("doGlissandoStart: unknown glissando subtype %hhd", st);
            return;
            }
      // check if on chord list
      int i = findChord(chord, int(st));
      if (i >= 0) {
            // print error and remove from list
            qDebug("doGlissandoStart: chord %p already on list", chord);
            if (st == Glissando::Type::STRAIGHT) slideChrd[i] = 0;
            if (st == Glissando::Type::WAVY) glissChrd[i] = 0;
            }
      // find free slot to store it
      i = findChord(0, int(st));
      if (i >= 0) {
            if (st == Glissando::Type::STRAIGHT) slideChrd[i] = chord;
            if (st == Glissando::Type::WAVY) glissChrd[i] = chord;
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
      Glissando::Type st = chord->glissando()->glissandoType();
      if (st != Glissando::Type::STRAIGHT && st != Glissando::Type::WAVY) {
            qDebug("doGlissandoStart: unknown glissando subtype %hhd", st);
            return;
            }
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (st == Glissando::Type::STRAIGHT && slideChrd[i] == chord) {
                  slideChrd[i] = 0;
                  glissando(chord->glissando(), i + 1, false, notations, xml);
                  return;
                  }
            if (st == Glissando::Type::WAVY && glissChrd[i] == chord) {
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
            if (el->type() != Element::Type::CHORD)
                  continue;
            startChord = static_cast<Chord*>(el);
            Segment* s = trill->score()->tick2segmentEnd(strack + i, endTick);
            if (!s)
                  continue;
            el = s->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != Element::Type::CHORD)
                  continue;
            stopChord = static_cast<Chord*>(el);
            return;
            }
      // try to find start/stop chords in different tracks
      for (int i = 0; i < VOICES; ++i) {
            Element* el = seg->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != Element::Type::CHORD)
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
            if (el->type() != Element::Type::CHORD)
                  continue;
            stopChord = static_cast<Chord*>(el);
            break;      // first chord found is OK
            }
      }

// find all trills in this measure and this part

static void findTrills(Measure* measure, int strack, int etrack, TrillHash& trillStart, TrillHash& trillStop)
      {
      // loop over all spanners in this measure
      int stick = measure->tick();
      int etick = measure->tick() + measure->ticks();
      for (auto it = measure->score()->spanner().lower_bound(stick); it != measure->score()->spanner().upper_bound(etick); ++it) {
            Spanner* e = it->second;
            //qDebug("findTrills 1 trill %p type %d track %d tick %d", e, e->type(), e->track(), e->tick());
            if (e->type() == Element::Type::TRILL && strack <= e->track() && e->track() < etrack
               && e->tick() >= measure->tick() && e->tick() < (measure->tick() + measure->ticks()))
                  {
                  //qDebug("findTrills 2 trill %p", e);
                  // a trill is found starting in this segment, trill end time is known
                  // determine notes to write trill start and stop
                  const Trill* tr = static_cast<const Trill*>(e);
                  Chord* startChord = 0;  // chord where trill starts
                  Chord* stopChord = 0;   // chord where trill stops

                  findTrillAnchors(tr, startChord, stopChord);
                  //qDebug("findTrills 3 startChord %p stopChord %p", startChord, stopChord);

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
            qDebug("backup %d", tick - t);
#endif
            addInteger(tick - t);
            }
      else if (t > tick) {
#ifdef DEBUG_TICK
            qDebug("forward %d", t - tick);
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

                  if (mb->type() != Element::Type::MEASURE)
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
                              if (el->type() == Element::Type::BAR_LINE && static_cast<BarLine*>(el)->barLineType() == BarLineType::START_REPEAT)
                                    continue;

                              if (tick != seg->tick())
                                    calcDivMoveToTick(seg->tick());

                              if (el->isChordRest()) {
                                    int l = static_cast<ChordRest*>(el)->actualTicks();
                                    if (el->type() == Element::Type::CHORD) {
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

      // font defaults
      // as MuseScore supports dozens of different styles, while MusicXML only has defaults
      // for music (TODO), words and lyrics, use TextStyleType STAFF (typically used for words)
      // and LYRIC1 to get MusicXML defaults
      const TextStyle tsStaff = s->textStyle(TextStyleType::STAFF);
      const TextStyle tsLyric = s->textStyle(TextStyleType::LYRIC1);
      // TODO xml.tagE("music-font font-family=\"TBD\" font-size=\"TBD\"");
      xml.tagE(QString("word-font font-family=\"%1\" font-size=\"%2\"").arg(tsStaff.family()).arg(tsStaff.size()));
      xml.tagE(QString("lyric-font font-family=\"%1\" font-size=\"%2\"").arg(tsLyric.family()).arg(tsLyric.size()));
      xml.etag();
      }


//---------------------------------------------------------
//   creditWords
//---------------------------------------------------------

static void creditWords(Xml& xml, Score* s, double x, double y, QString just, QString val, QString words, const TextStyle& ts)
      {
      xml.stag("credit page=\"1\"");
      QString attr = QString(" default-x=\"%1\"").arg(x);
      attr += QString(" default-y=\"%1\"").arg(y);
      attr += " justify=\"" + just + "\"";
      attr += " valign=\"" + val + "\"";
      MScoreTextToMXML mttm("credit-words", attr, words, s->textStyle(TextStyleType::STAFF), ts);
      mttm.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   parentHeight
//---------------------------------------------------------

static double parentHeight(const Element* element)
      {
            const Element* parent = element->parent();

            if (!parent)
                  return 0;

            if (parent->type() == Element::Type::VBOX) {
                  return parent->height();
                  }

            return 0;
      }

//---------------------------------------------------------
//   credits
//---------------------------------------------------------

void ExportMusicXml::credits(Xml& xml)
      {
      const MeasureBase* measure = _score->measures()->first();
      QString rights = _score->metaTag("copyright");

      // determine page formatting
      const PageFormat* pf = _score->pageFormat();
      if (!pf) return;
      const double h  = getTenthsFromInches(pf->size().height());
      const double w  = getTenthsFromInches(pf->size().width());
      const double lm = getTenthsFromInches(pf->oddLeftMargin());
      const double rm = getTenthsFromInches(pf->oddRightMargin());
      //const double tm = getTenthsFromInches(pf->oddTopMargin());
      const double bm = getTenthsFromInches(pf->oddBottomMargin());
      //qDebug("page h=%g w=%g lm=%g rm=%g tm=%g bm=%g", h, w, lm, rm, tm, bm);

      // write the credits
      if (measure) {
            foreach(const Element* element, *measure->el()) {
                  if (element->type() == Element::Type::TEXT) {
                        const Text* text = (const Text*)element;
                        const double ph = getTenthsFromDots(parentHeight(text));

                        double tx = w / 2;
                        double ty = h - getTenthsFromDots(text->pagePos().y());
                        QString styleName = text->textStyle().name();

                        Align al = text->textStyle().align();
                        QString just;
                        QString val;

                        if (al & AlignmentFlags::RIGHT) {
                              just = "right";
                              tx   = w - rm;
                              }
                        else if (al & AlignmentFlags::HCENTER) {
                              just = "center";
                              // tx already set correctly
                              }
                        else {
                              just = "left";
                              tx   = lm;
                              }

                        if (al & AlignmentFlags::BOTTOM) {
                              val = "bottom";
                              ty -= ph;
                              }
                        else if (al & AlignmentFlags::VCENTER) {
                              val = "middle";
                              ty -= ph / 2;
                              }
                        else if (al & AlignmentFlags::BASELINE) {
                              val = "baseline";
                              ty -= ph / 2;
                              }
                        else {
                              val = "top";
                              // ty already set correctly
                              }

                        creditWords(xml, _score, tx, ty, just, val, text->text(), text->textStyle());
                        }
                  }
            }

      if (!rights.isEmpty()) {
            creditWords(xml, _score, w / 2, bm, "center", "bottom", rights, _score->textStyle(TextStyleType::FOOTER));
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

static void tabpitch2xml(const int pitch, const int tpc, QString& s, int& alter, int& octave)
      {
      s      = tpc2stepName(tpc);
      alter  = tpc2alterByKey(tpc, Key::C);
      octave = (pitch - alter) / 12 - 1;
      if (alter < -2 || 2 < alter)
            qDebug("tabpitch2xml(pitch %d, tpc %d) problem:  step %s, alter %d, octave %d",
                   pitch, tpc, qPrintable(s), alter, octave);
      else
            qDebug("tabpitch2xml(pitch %d, tpc %d) step %s, alter %d, octave %d",
                   pitch, tpc, qPrintable(s), alter, octave);
      }

//---------------------------------------------------------
//   pitch2xml
//---------------------------------------------------------

// TODO validation

static void pitch2xml(const Note* note, QString& s, int& alter, int& octave)
      {

      const Staff* st = note->staff();
      const Instrument* instr = st->part()->instr();
      const Interval intval = instr->transpose();

      s      = tpc2stepName(note->tpc());
      alter  = tpc2alterByKey(note->tpc(), Key::C);
      // note that pitch must be converted to concert pitch
      // in order to calculate the correct octave
      octave = (note->pitch() - intval.chromatic - alter) / 12 - 1;

      //
      // HACK:
      // On percussion clefs there is no relationship between
      // note->pitch() and note->line()
      // note->line() is determined by drumMap
      //
      int tick        = note->chord()->tick();
      ClefType ct     = st->clef(tick);
      if (ct == ClefType::PERC || ct == ClefType::PERC2) {
            alter = 0;
            octave = line2pitch(note->line(), ct, Key::C) / 12 - 1;
      }

      // correct for ottava lines
      int ottava = 0;
      switch (note->ppitch() - note->pitch()) {
            case  24: ottava =  2; break;
            case  12: ottava =  1; break;
            case   0: ottava =  0; break;
            case -12: ottava = -1; break;
            case -24: ottava = -2; break;
            default:  qDebug("pitch2xml() tick=%d pitch()=%d ppitch()=%d",
                             tick, note->pitch(), note->ppitch());
            }
      octave += ottava;

      //qDebug("pitch2xml(pitch %d, tpc %d, ottava %d clef %hhd) step    %s, alter    %d, octave    %d",
      //       note->pitch(), note->tpc(), ottava, clef, qPrintable(s), alter, octave);
      }

// unpitch2xml -- calculate display-step and display-octave for an unpitched note
// note:
// even though this produces the correct step/octave according to Recordare's tutorial
// Finale Notepad 2012 does not import a three line staff with percussion clef correctly
// Same goes for Sibelius 6 in case of three or five line staff with percussion clef

static void unpitch2xml(const Note* note, QString& s, int& octave)
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
      s               = table1[stepIdx];
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
            if (el->type() != Element::Type::VOLTA)
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
            Volta::Type st = v->voltaType();
            switch (st) {
                  case Volta::Type::OPEN:
                        type = "discontinue";
                        break;
                  case Volta::Type::CLOSED:
                        type = "stop";
                        break;
                  default:
                        qDebug("unknown volta subtype %hhd", st);
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
      bool rs = m->repeatFlags() & Repeat::START;
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
      BarLineType bst = m->endBarLineType();
      bool visible = m->endBarLineVisible();
      bool needBarStyle = (bst != BarLineType::NORMAL && bst != BarLineType::START_REPEAT) || !visible;
      Volta* volta = findVolta(m, false);
      if (!needBarStyle && !volta)
            return;
      xml.stag(QString("barline location=\"right\""));
      if (needBarStyle) {
            if (!visible) {
                  xml.tag("bar-style", QString("none"));
                  } else {
                  switch (bst) {
                        case BarLineType::DOUBLE:
                              xml.tag("bar-style", QString("light-light"));
                              break;
                        case BarLineType::END_REPEAT:
                              xml.tag("bar-style", QString("light-heavy"));
                              break;
                        case BarLineType::BROKEN:
                              xml.tag("bar-style", QString("dashed"));
                              break;
                        case BarLineType::DOTTED:
                              xml.tag("bar-style", QString("dotted"));
                              break;
                        case BarLineType::END:
                        case BarLineType::END_START_REPEAT:
                              xml.tag("bar-style", QString("light-heavy"));
                              break;
                        default:
                              qDebug("ExportMusicXml::bar(): bar subtype %hhd not supported", bst);
                              break;
                        }
                  }
            }
      if (volta)
            ending(xml, volta, false);
      if (bst == BarLineType::END_REPEAT || bst == BarLineType::END_START_REPEAT)
      {
          if (m->repeatCount() > 2) {
              xml.tagE("repeat direction=\"backward\" times=\"%i\"",m->repeatCount());
          }else{
              xml.tagE("repeat direction=\"backward\"");
          }
      }
      xml.etag();
      }

//---------------------------------------------------------
//   moveToTick
//---------------------------------------------------------

void ExportMusicXml::moveToTick(int t)
      {
      // qDebug("ExportMusicXml::moveToTick(t=%d) tick=%d", t, tick);
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
      TimeSigType st = tsig->timeSigType();
      Fraction ts = tsig->sig();
      int z = ts.numerator();
      int n = ts.denominator();
      QString ns = tsig->numeratorString();

      attr.doAttr(xml, true);
      if (st == TimeSigType::FOUR_FOUR)
            xml.stag("time symbol=\"common\"");
      else if (st == TimeSigType::ALLA_BREVE)
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

void ExportMusicXml::keysig(Key key, int staff, bool visible)
      {
      QString tg = "key";
      if (staff)
            tg += QString(" number=\"%1\"").arg(staff);
      if (!visible)
            tg += " print-object=\"no\"";
      attr.doAttr(xml, true);
      xml.stag(tg);
      xml.tag("fifths", int(key));
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
            if (t->numberType() == Tuplet::NumberType::SHOW_RELATION)
                  tupletTag += " show-number=\"both\"";
            if (t->numberType() == Tuplet::NumberType::NO_TEXT)
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
      if (s->segmentType() == Segment::Type::Breath)
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
            TremoloType st = tr->tremoloType();
            QString type = "";

            if (chord->tremoloChordType() == TremoloChordType::TremoloSingle) {
                  type = "single";
                  switch (st) {
                        case TremoloType::R8:  count = 1; break;
                        case TremoloType::R16: count = 2; break;
                        case TremoloType::R32: count = 3; break;
                        case TremoloType::R64: count = 4; break;
                        default: qDebug("unknown tremolo single %hhd", st); break;
                        }
                  }
            else if (chord->tremoloChordType() == TremoloChordType::TremoloFirstNote) {
                  type = "start";
                  switch (st) {
                        case TremoloType::C8:  count = 1; break;
                        case TremoloType::C16: count = 2; break;
                        case TremoloType::C32: count = 3; break;
                        case TremoloType::C64: count = 4; break;
                        default: qDebug("unknown tremolo double %hhd", st); break;
                        }
                  }
            else if (chord->tremoloChordType() == TremoloChordType::TremoloSecondNote) {
                  type = "stop";
                  switch (st) {
                        case TremoloType::C8:  count = 1; break;
                        case TremoloType::C16: count = 2; break;
                        case TremoloType::C32: count = 3; break;
                        case TremoloType::C64: count = 4; break;
                        default: qDebug("unknown tremolo double %hhd", st); break;
                        }
                  }
            else qDebug("unknown tremolo subtype %hhd", st);


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
            if (at == ArticulationType::Fermata
                || at == ArticulationType::Shortfermata
                || at == ArticulationType::Longfermata
                || at == ArticulationType::Verylongfermata) {
                  notations.tag(xml);
                  QString type = a->up() ? "upright" : "inverted";
                  if (at == ArticulationType::Fermata)
                        xml.tagE(QString("fermata type=\"%1\"").arg(type));
                  else if (at == ArticulationType::Shortfermata)
                        xml.tag(QString("fermata type=\"%1\"").arg(type), "angled");
                  // MusicXML does not support the very long fermata,
                  // export as long fermata (better than not exporting at all)
                  else if (at == ArticulationType::Longfermata
                           || at == ArticulationType::Verylongfermata)
                        xml.tag(QString("fermata type=\"%1\"").arg(type), "square");
                  }
            }

      // then the attributes whose elements are children of <articulations>
      Articulations articulations;
      foreach (const Articulation* a, na) {
            switch (a->articulationType()) {
                  case ArticulationType::Fermata:
                  case ArticulationType::Shortfermata:
                  case ArticulationType::Longfermata:
                  case ArticulationType::Verylongfermata:
                        // ignore, already handled
                        break;
                  case ArticulationType::Sforzatoaccent:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("accent");
                        }
                        break;
                  case ArticulationType::Staccato:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("staccato");
                        }
                        break;
                  case ArticulationType::Staccatissimo:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("staccatissimo");
                        }
                        break;
                  case ArticulationType::Tenuto:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("tenuto");
                        }
                        break;
                  case ArticulationType::Marcato:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        if (a->up())
                              xml.tagE("strong-accent type=\"up\"");
                        else
                              xml.tagE("strong-accent type=\"down\"");
                        }
                        break;
                  case ArticulationType::Portato:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("detached-legato");
                        }
                        break;
                  case ArticulationType::Reverseturn:
                  case ArticulationType::Turn:
                  case ArticulationType::Trill:
                  case ArticulationType::Prall:
                  case ArticulationType::Mordent:
                  case ArticulationType::PrallPrall:
                  case ArticulationType::PrallMordent:
                  case ArticulationType::UpPrall:
                  case ArticulationType::DownPrall:
                  case ArticulationType::UpMordent:
                  case ArticulationType::DownMordent:
                  case ArticulationType::PrallDown:
                  case ArticulationType::PrallUp:
                  case ArticulationType::LinePrall:
                        // ignore, handled with ornaments
                        break;
                  case ArticulationType::Plusstop:
                  case ArticulationType::Upbow:
                  case ArticulationType::Downbow:
                  case ArticulationType::Snappizzicato:
                  case ArticulationType::ThumbPosition:
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
            qDebug("chordAttributes: el %p type %hhd (%s)", e, e->type(), e->name());
            if (e->type() == Element::Type::CHORDLINE) {
                  ChordLine const* const cl = static_cast<ChordLine const* const>(e);
                  QString subtype;
                  switch (cl->chordLineType()) {
                        case ChordLineType::FALL:
                              subtype = "falloff";
                              break;
                        case ChordLineType::DOIT:
                              subtype = "doit";
                              break;
                        case ChordLineType::PLOP:
                              subtype = "plop";
                              break;
                        case ChordLineType::SCOOP:
                              subtype = "scoop";
                              break;
                        default:
                              qDebug("unknown ChordLine subtype %hhd", cl->chordLineType());
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
                  case ArticulationType::Fermata:
                  case ArticulationType::Shortfermata:
                  case ArticulationType::Longfermata:
                  case ArticulationType::Verylongfermata:
                  case ArticulationType::Sforzatoaccent:
                  case ArticulationType::Staccato:
                  case ArticulationType::Staccatissimo:
                  case ArticulationType::Tenuto:
                  case ArticulationType::Marcato:
                  case ArticulationType::Portato:
                        // ignore, already handled
                        break;
                  case ArticulationType::Reverseturn:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-turn");
                        }
                        break;
                  case ArticulationType::Turn:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("turn");
                        }
                        break;
                  case ArticulationType::Trill:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("trill-mark");
                        }
                        break;
                  case ArticulationType::Prall:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent");
                        }
                        break;
                  case ArticulationType::Mordent:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent");
                        }
                        break;
                  case ArticulationType::PrallPrall:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\"");
                        }
                        break;
                  case ArticulationType::PrallMordent:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent long=\"yes\"");
                        }
                        break;
                  case ArticulationType::UpPrall:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" approach=\"below\"");
                        }
                        break;
                  case ArticulationType::DownPrall:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" approach=\"above\"");
                        }
                        break;
                  case ArticulationType::UpMordent:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent long=\"yes\" approach=\"below\"");
                        }
                        break;
                  case ArticulationType::DownMordent:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent long=\"yes\" approach=\"above\"");
                        }
                        break;
                  case ArticulationType::PrallDown:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" departure=\"below\"");
                        }
                        break;
                  case ArticulationType::PrallUp:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" departure=\"above\"");
                        }
                        break;
                  case ArticulationType::LinePrall:
                        {
                        // MusicXML 3.0 does not distinguish between downprall and lineprall
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" approach=\"above\"");
                        }
                        break;
                  case ArticulationType::Schleifer:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("schleifer");
                        }
                        break;
                  case ArticulationType::Plusstop:
                  case ArticulationType::Upbow:
                  case ArticulationType::Downbow:
                  case ArticulationType::Snappizzicato:
                  case ArticulationType::ThumbPosition:
                        // ignore, handled with technical
                        break;
                  default:
                        qDebug("unknown chord attribute %s", qPrintable(a->subtypeUserName()));
                        break;
                  }
            }
      tremoloSingleStartStop(chord, notations, ornaments, xml);
      wavyLineStartStop(chord, notations, ornaments, xml, trillStart, trillStop);
      ornaments.etag(xml);

      // and finally the attributes whose elements are children of <technical>
      foreach (const Articulation* a, na) {
            switch (a->articulationType()) {
                  case ArticulationType::Plusstop:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("stopped");
                        }
                        break;
                  case ArticulationType::Upbow:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("up-bow");
                        }
                        break;
                  case ArticulationType::Downbow:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("down-bow");
                        }
                        break;
                  case ArticulationType::Snappizzicato:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("snap-pizzicato");
                        }
                        break;
                  case ArticulationType::Ouvert:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("open-string");
                        }
                        break;
                  case ArticulationType::ThumbPosition:
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
                  qDebug("unknown arpeggio subtype %d", int(arp->arpeggioType()));
                  break;
            }
      }

// find the next chord in the same track

static Chord* nextChord(Chord* ch)
      {
      Segment* s = ch->segment();
      s = s->next1();
      while (s) {
            if (s->segmentType() == Segment::Type::ChordRest && s->element(ch->track()))
                  break;
            s = s->next1();
            }
      if (s == 0) {
            // qDebug("no segment for second note of glissando found");
            return 0;
            }
      Chord* c = static_cast<Chord*>(s->element(ch->track()));
      if (c == 0 || c->type() != Element::Type::CHORD) {
            // qDebug("no second note for glissando found, track %d", track());
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
//      qDebug("determineTupletNormalTicks t %p baselen %d", t, t->baseLen().ticks());
//      for (int i = 0; i < t->elements().size(); ++i)
//            qDebug("determineTupletNormalTicks t %p i %d ticks %d", t, i, t->elements().at(i)->duration().ticks());
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
            qDebug("Beam::writeMusicXml(): cannot find ChordRest");
            return;
            }
      int blp = -1; // beam level previous chord
      int blc = -1; // beam level current chord
      int bln = -1; // beam level next chord
      // find beam level previous chord
      for (int i = idx - 1; blp == -1 && i >= 0; --i) {
            ChordRest* crst = elements[i];
            if (crst->type() == Element::Type::CHORD)
                  blp = (static_cast<Chord*>(crst))->beams();
            }
      // find beam level current chord
      if (cr->type() == Element::Type::CHORD)
            blc = (static_cast<Chord*>(cr))->beams();
      // find beam level next chord
      for (int i = idx + 1; bln == -1 && i < elements.size(); ++i) {
            ChordRest* crst = elements[i];
            if (crst->type() == Element::Type::CHORD)
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

void ExportMusicXml::chord(Chord* chord, int staff, const QList<Lyrics*>* ll, DrumsetKind useDrumset)
      {
      /*
      qDebug("chord() %p parent %p isgrace %d #gracenotes %d graceidx %d",
             chord, chord->parent(), chord->isGrace(), chord->graceNotes().size(), chord->graceIndex());
      foreach(Element* e, chord->el())
            qDebug("chord %p el %p", chord, e);
       */
      QList<Note*> nl = chord->notes();
      bool grace = chord->isGrace();
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
            if (note->veloType() == Note::ValueType::USER_VAL) {
                  int velo = note->veloOffset();
                  noteTag += QString(" dynamics=\"%1\"").arg(QString::number(velo * 100.0 / 90.0,'f',2));
                  }
            xml.stag(noteTag);

            if (grace) {
                  if (note->noteType() == NoteType::ACCIACCATURA)
                        xml.tagE("grace slash=\"yes\"");
                  else
                        xml.tagE("grace");
                  }
            if (note != nl.front())
                  xml.tagE("chord");

            // step / alter / octave
            QString step;
            int alter = 0;
            int octave = 0;
            if (chord->staff() && chord->staff()->isTabStaff()) {
                  tabpitch2xml(note->pitch(), note->tpc(), step, alter, octave);
            }
            else {
                  if (useDrumset == DrumsetKind::NONE) {
                        pitch2xml(note, step, alter, octave);
                  }
                  else {
                        unpitch2xml(note, step, octave);
                  }
            }
            xml.stag(useDrumset != DrumsetKind::NONE ? "unpitched" : "pitch");
            xml.tag(useDrumset != DrumsetKind::NONE ? "display-step" : "step", step);
            // Check for microtonal accidentals and overwrite "alter" tag
            Accidental* acc = note->accidental();
            double alter2 = 0.0;
            if (acc) {
                  switch (acc->accidentalType()) {
                        case Accidental::Type::MIRRORED_FLAT:  alter2 = -0.5; break;
                        case Accidental::Type::SHARP_SLASH:    alter2 = 0.5;  break;
                        case Accidental::Type::MIRRORED_FLAT2: alter2 = -1.5; break;
                        case Accidental::Type::SHARP_SLASH4:   alter2 = 1.5;  break;
                        default:                                             break;
                        }
                  }
            if (alter && !alter2)
                  xml.tag("alter", alter);
            if (!alter && alter2)
                  xml.tag("alter", alter2);
            // TODO what if both alter and alter2 are present? For Example: playing with transposing instruments
            xml.tag(useDrumset != DrumsetKind::NONE ? "display-octave" : "octave", octave);
            xml.etag();

            // duration
            if (!grace)
                  xml.tag("duration", note->chord()->actualTicks() / div);

            if (note->tieBack())
                  xml.tagE("tie type=\"stop\"");
            if (note->tieFor())
                  xml.tagE("tie type=\"start\"");

            // instrument for unpitched
            if (useDrumset != DrumsetKind::NONE)
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
                  qDebug("no note type found for ticks %d",
                         note->chord()->actualTicks());
                  }
            xml.tag("type", s);
            for (int ni = dots; ni > 0; ni--)
                  xml.tagE("dot");

            // accidental
            if (acc) {
                  /*
                        MusicXML 2.0 accidental names include:
                        sharp,natural, flat, double-sharp, sharp-sharp, flat-flat,
                        natural-sharp, natural-flat, quarter-flat, quarter-sharp,
                        three-quarters-flat, and three-quarters-sharp.
                        Added in MusicXml 3.0: sharp-down, sharp-up, natural-down, natural-up,
                        flat-down, flat-up, triple-sharp, triple-flat, slash-quarter-sharp,
                        slash-sharp, slash-flat, double-slash-flat, sharp-1, sharp-2,
                        sharp-3, sharp-5, flat-1, flat-2, flat-3, flat-4, sori, and koron.
                    */
                  QString s;
                  switch (acc->accidentalType()) {
                        case Accidental::Type::SHARP:              s = "sharp";                break;
                        case Accidental::Type::FLAT:               s = "flat";                 break;
                        case Accidental::Type::SHARP2:             s = "double-sharp";         break;
                        case Accidental::Type::FLAT2:              s = "flat-flat";            break;
                        case Accidental::Type::NATURAL:            s = "natural";              break;
                        case Accidental::Type::FLAT_SLASH:         s = "slash-flat";           break;
                        case Accidental::Type::MIRRORED_FLAT:      s = "quarter-flat";         break; // (recommended by Michael)
                        case Accidental::Type::FLAT_ARROW_UP:      s = "flat-up";              break;
                        case Accidental::Type::NATURAL_ARROW_DOWN: s = "natural-down";         break;
                        case Accidental::Type::SHARP_SLASH:        s = "quarter-sharp";        break; // (recommended by Michael)
                        case Accidental::Type::SHARP_ARROW_DOWN:   s = "sharp-down";           break;
                        case Accidental::Type::NATURAL_ARROW_UP:   s = "natural-up";           break;
                        case Accidental::Type::MIRRORED_FLAT2:     s = "three-quarters-flat";  break; // (recommended by Michael)
                        case Accidental::Type::FLAT_FLAT_SLASH:    s = "three-quarters-flat";  break; // (alternative) - not used ?
                        case Accidental::Type::FLAT_ARROW_DOWN:    s = "flat-down";            break;
                        case Accidental::Type::SHARP_SLASH4:       s = "three-quarters-sharp"; break; // (recommended by Michael)
                        case Accidental::Type::SHARP_ARROW_UP:     s = "sharp-up";             break;
                        case Accidental::Type::SHARP_SLASH3:       s = "slash-quarter-sharp";  break;
                        case Accidental::Type::FLAT_SLASH2:        s = "double-slash-flat";    break;
                        case Accidental::Type::SHARP_SLASH2:       s = "slash-sharp";          break;
                        case Accidental::Type::SORI:               s = "sori";                 break; //sori
                        case Accidental::Type::KORON:              s = "koron";                break; //koron
                        default:
                              qDebug("unknown accidental %d", int(acc->accidentalType()));
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
                  //qDebug("nrmTicks %d", nrmTicks);
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
            bool leftParenthesis, rightParenthesis = false;
            for (Element* elem : note->el()) {
                  if (elem->type() == Element::Type::SYMBOL) {
                        Symbol* s = static_cast<Symbol*>(elem);
                        if(s->sym() == SymId::noteheadParenthesisLeft)
                              leftParenthesis = true;
                        else if (s->sym() == SymId::noteheadParenthesisRight)
                              rightParenthesis = true;
                        }
                  }
            if (rightParenthesis && leftParenthesis)
                  noteheadTagname += " parentheses=\"yes\"";
            if (note->headGroup() == NoteHead::Group::HEAD_SLASH)
                  xml.tag(noteheadTagname, "slash");
            else if (note->headGroup() == NoteHead::Group::HEAD_TRIANGLE)
                  xml.tag(noteheadTagname, "triangle");
            else if (note->headGroup() == NoteHead::Group::HEAD_DIAMOND)
                  xml.tag(noteheadTagname, "diamond");
            else if (note->headGroup() == NoteHead::Group::HEAD_CROSS)
                  xml.tag(noteheadTagname, "x");
            else if (note->headGroup() == NoteHead::Group::HEAD_XCIRCLE)
                  xml.tag(noteheadTagname, "circle-x");
            else if (note->headGroup() == NoteHead::Group::HEAD_DO)
                  xml.tag(noteheadTagname, "do");
            else if (note->headGroup() == NoteHead::Group::HEAD_RE)
                  xml.tag(noteheadTagname, "re");
            else if (note->headGroup() == NoteHead::Group::HEAD_MI)
                  xml.tag(noteheadTagname, "mi");
            else if (note->headGroup() == NoteHead::Group::HEAD_FA)
                  xml.tag(noteheadTagname, "fa");
            else if (note->headGroup() == NoteHead::Group::HEAD_LA)
                  xml.tag(noteheadTagname, "la");
            else if (note->headGroup() == NoteHead::Group::HEAD_TI)
                  xml.tag(noteheadTagname, "ti");
            else if (note->headGroup() == NoteHead::Group::HEAD_SOL)
                  xml.tag(noteheadTagname, "so");
            else if (noteheadColor != MScore::defaultColor)
                  xml.tag(noteheadTagname, "normal");
            else if (rightParenthesis && leftParenthesis)
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

            const Tie* tieBack = note->tieBack();
            if (tieBack) {
                  notations.tag(xml);
                  xml.tagE("tied type=\"stop\"");
                  }
            const Tie* tieFor = note->tieFor();
            if (tieFor) {
                  notations.tag(xml);
                  QString rest = slurTieLineStyle(tieFor);
                  xml.tagE(QString("tied type=\"start\"%1").arg(rest));
                  }

            if (note == nl.front()) {
                  if (grace){
                        sh.doSlurStart(chord, notations, xml, true);
                        }
                  else {
                        tupletStartStop(chord, notations, xml);
                        sh.doSlurStop(chord, notations, xml);
                        sh.doSlurStart(chord, notations, xml);
                        }
                  chordAttributes(chord, notations, technical, xml, trillStart, trillStop);
                  }
            foreach (const Element* e, note->el()) {
                  if (e->type() == Element::Type::FINGERING) {
                        Text* f = (Text*)e;
                        notations.tag(xml);
                        technical.tag(xml);
                        QString t = MScoreTextToMXML::toPlainText(f->text());
                        if (f->textStyleType() == TextStyleType::FINGERING) {
                              // p, i, m, a, c represent the plucking finger
                              if (t == "p" || t == "i" || t == "m" || t == "a" || t == "c")
                                    xml.tag("pluck", t);
                              else
                                    xml.tag("fingering", t);
                              }
                        else if (f->textStyleType() == TextStyleType::STRING_NUMBER) {
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
      if (d.type() == TDuration::DurationType::V_MEASURE) {
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
      if (d.type() != TDuration::DurationType::V_MEASURE) {
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
             qDebug("directionTag() spatium=%g elem=%p tp=%d (%s)\ndirectionTag()  x=%g y=%g xsp,ysp=%g,%g w=%g h=%g userOff.y=%g",
                    el->spatium(),
                    el,
                    el->type(),
                    el->name(),
                    el->x(), el->y(),
                    el->x()/el->spatium(), el->y()/el->spatium(),
                    el->width(), el->height(),
                    el->userOff().y()
                   );
             */
            const Element* pel = 0;
            const LineSegment* seg = 0;
            if (el->type() == Element::Type::HAIRPIN || el->type() == Element::Type::OTTAVA
                || el->type() == Element::Type::PEDAL || el->type() == Element::Type::TEXTLINE) {
                  // handle elements derived from SLine
                  // find the system containing the first linesegment
                  const SLine* sl = static_cast<const SLine*>(el);
                  if (sl->spannerSegments().size() > 0) {
                        seg = (LineSegment*)sl->spannerSegments().at(0);
                        /*
                         qDebug("directionTag()  seg=%p x=%g y=%g w=%g h=%g cpx=%g cpy=%g userOff.y=%g",
                                seg, seg->x(), seg->y(),
                                seg->width(), seg->height(),
                                seg->pagePos().x(), seg->pagePos().y(),
                                seg->userOff().y());
                         */
                        pel = seg->parent();
                        }
                  }
            else if (el->type() == Element::Type::DYNAMIC
                     || el->type() == Element::Type::STAFF_TEXT
                     || el->type() == Element::Type::REHEARSAL_MARK
                     || el->type() == Element::Type::SYMBOL
                     || el->type() == Element::Type::TEXT) {
                  // handle other elements attached (e.g. via Segment / Measure) to a system
                  // find the system containing this element
                  for (const Element* e = el; e; e = e->parent()) {
                        if (e->type() == Element::Type::SYSTEM) pel = e;
                        }
                  }
            else
                  qDebug("directionTag() element %p tp=%hhd (%s) not supported",
                         el, el->type(), el->name());

            /*
             if (pel) {
             qDebug("directionTag()  prnt tp=%d (%s) x=%g y=%g w=%g h=%g userOff.y=%g",
                    pel->type(),
                    pel->name(),
                    pel->x(), pel->y(),
                    pel->width(), pel->height(),
                    pel->userOff().y());
                  }
             */

            if (pel && pel->type() == Element::Type::SYSTEM) {
                  const System* sys = static_cast<const System*>(pel);
                  QRectF bb = sys->staff(el->staffIdx())->bbox();
                  /*
                  qDebug("directionTag()  syst=%p sys x=%g y=%g cpx=%g cpy=%g",
                         sys, sys->pos().x(),  sys->pos().y(),
                         sys->pagePos().x(),
                         sys->pagePos().y()
                        );
                  qDebug("directionTag()  staf x=%g y=%g w=%g h=%g",
                         bb.x(), bb.y(),
                         bb.width(), bb.height());
                  // element is above the staff if center of bbox is above center of staff
                  qDebug("directionTag()  center diff=%g", el->y() + el->height() / 2 - bb.y() - bb.height() / 2);
                   */

                  if (el->type() == Element::Type::HAIRPIN || el->type() == Element::Type::OTTAVA
                      || el->type() == Element::Type::PEDAL || el->type() == Element::Type::TEXTLINE) {
                        // for the line type elements the reference point is vertically centered
                        // actual position info is in the segments
                        // compare the segment's canvas ypos with the staff's center height
                        if (seg->pagePos().y() < sys->pagePos().y() + bb.y() + bb.height() / 2)
                              tagname += " placement=\"above\"";
                        else
                              tagname += " placement=\"below\"";
                        }
                  else {
                        /*
                        qDebug("directionTag()  staf ely=%g elh=%g bby=%g bbh=%g",
                               el->y(), el->height(),
                               bb.y(), bb.height());
                         */
                        if (el->y() + el->height() / 2 < /*bb.y() +*/ bb.height() / 2)
                              tagname += " placement=\"above\"";
                        else
                              tagname += " placement=\"below\"";
                        }
                  } // if (pel && ...
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

static void partGroupStart(Xml& xml, int number, BracketType bracket)
      {
      xml.stag(QString("part-group type=\"start\" number=\"%1\"").arg(number));
      QString br = "";
      switch (bracket) {
            case BracketType::NO_BRACKET:
                  br = "none";
                  break;
            case BracketType::NORMAL:
                  br = "bracket";
                  break;
            case BracketType::BRACE:
                  br = "brace";
                  break;
            case BracketType::LINE:
                  br = "line";
                  break;
            case BracketType::SQUARE:
                  br = "square";
                  break;
            default:
                  qDebug("bracket subtype %hhd not understood", bracket);
            }
      if (br != "")
            xml.tag("group-symbol", br);
      xml.etag();
      }

//---------------------------------------------------------
//   words
//---------------------------------------------------------

static bool findUnit(TDuration::DurationType val, QString& unit)
      {
      unit = "";
      switch (val) {
            case TDuration::DurationType::V_HALF: unit = "half"; break;
            case TDuration::DurationType::V_QUARTER: unit = "quarter"; break;
            case TDuration::DurationType::V_EIGHTH: unit = "eighth"; break;
            default: qDebug("findUnit: unknown DurationType %hhd", val);
            }
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
      //qDebug("findMetronome('%s')", qPrintable(words));
      wordsLeft  = "";
      hasParen   = false;
      metroLeft  = "";
      metroRight = "";
      wordsRight = "";
      int indEq  = words.indexOf('=');
      if (indEq <= 0)
            return false;
      int len1 = 0;
      TDuration dur;
      // find first note, limiting search to the part left of the first '=',
      // to prevent matching the second note in a "note1 = note2" metronome
      int pos1 = TempoText::findTempoDuration(words.left(indEq), len1, dur);
      QRegExp eq("\\s*=\\s*");
      int pos2 = eq.indexIn(words, pos1 + len1);
      if (pos1 != -1 && pos2 == pos1 + len1) {
            int len2 = eq.matchedLength();
            if (words.length() > pos2 + len2) {
                  QString s1 = words.mid(0, pos1);     // string to the left of metronome
                  QString s2 = words.mid(pos1, len1);  // first note
                  QString s3 = words.mid(pos2, len2);  // equals sign
                  QString s4 = words.mid(pos2 + len2); // string to the right of equals sign
                  /*
                  qDebug("found note and equals: '%s'%s'%s'%s'",
                         qPrintable(s1),
                         qPrintable(s2),
                         qPrintable(s3),
                         qPrintable(s4)
                         );
                   */
                  // now determine what is to the right of the equals sign
                  // must have either a (dotted) note or a number at start of s4
                  int len3 = 0;
                  QRegExp nmb("\\d+");
                  int pos3 = TempoText::findTempoDuration(s4, len3, dur);
                  if (pos3 == -1) {
                        // did not find note, try to find a number
                        pos3 = nmb.indexIn(s4);
                        if (pos3 == 0)
                              len3 = nmb.matchedLength();
                        }
                  if (pos3 == -1)
                        // neither found
                        return false;

                  QString s5 = s4.mid(0, len3); // number or second note
                  QString s6 = s4.mid(len3);    // string to the right of metronome
                  /*
                  qDebug("found right part: '%s'%s'",
                         qPrintable(s5),
                         qPrintable(s6)
                         );
                   */

                  // determine if metronome has parentheses
                  // left part of string must end with parenthesis plus optional spaces
                  // right part of string must have parenthesis (but not in first pos)
                  int lparen = s1.indexOf("(");
                  int rparen = s6.indexOf(")");
                  hasParen = (lparen == s1.length() - 1 && rparen == 0);
                  //qDebug(" lparen=%d rparen=%d hasP=%d", lparen, rparen, hasParen);

                  if (hasParen)
                        wordsLeft = s1.mid(0, lparen);
                  else
                        wordsLeft = s1;
                  metroLeft = s2;
                  metroRight = s5;
                  if (hasParen)
                        wordsRight = s6.mid(1);
                  else
                        metroRight = s5;

                  /*
                  qDebug(" '%s'%s'%s'%s'",
                         qPrintable(wordsLeft),
                         qPrintable(metroLeft),
                         qPrintable(metroRight),
                         qPrintable(wordsRight)
                         );
                   */
                  return true;
                  }
            }
      return false;
      }

static void beatUnit(Xml& xml, const TDuration dur)
      {
      int dots = dur.dots();
      QString unit;
      findUnit(dur.type(), unit);
      xml.tag("beat-unit", unit);
      while (dots > 0) {
            xml.tagE("beat-unit-dot");
            --dots;
            }
      }

static void wordsMetrome(Xml& xml, Score* s, Text const* const text)
      {
      QString wordsLeft;  // words left of metronome
      bool hasParen;      // parenthesis
      QString metroLeft;  // left part of metronome
      QString metroRight; // right part of metronome
      QString wordsRight; // words right of metronome
      if (findMetronome(text->text(), wordsLeft, hasParen, metroLeft, metroRight, wordsRight)) {
            if (wordsLeft != "") {
                  xml.stag("direction-type");
                  QString attr; // TODO TBD
                  MScoreTextToMXML mttm("words", attr, wordsLeft, s->textStyle(TextStyleType::STAFF), s->textStyle(TextStyleType::STAFF) /* TODO: verify correct value */);
                  mttm.write(xml);
                  xml.etag();
                  }
            xml.stag("direction-type");
            xml.stag(QString("metronome parentheses=\"%1\"").arg(hasParen ? "yes" : "no"));
            int len1 = 0;
            TDuration dur;
            TempoText::findTempoDuration(metroLeft, len1, dur);
            beatUnit(xml, dur);

            if (TempoText::findTempoDuration(metroRight, len1, dur) != -1)
                  beatUnit(xml, dur);
            else
                  xml.tag("per-minute", metroRight);

            xml.etag();
            xml.etag();
            if (wordsRight != "") {
                  xml.stag("direction-type");
                  QString attr; // TODO TBD
                  MScoreTextToMXML mttm("words", attr, wordsRight, s->textStyle(TextStyleType::STAFF), s->textStyle(TextStyleType::STAFF) /* TODO: verify correct value */);
                  mttm.write(xml);
                  xml.etag();
                  }
            }
      else {
            xml.stag("direction-type");
            QString attr; // TODO TBD
            MScoreTextToMXML mttm("words", attr, text->text(), s->textStyle(TextStyleType::STAFF), s->textStyle(TextStyleType::STAFF));
            mttm.write(xml);
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
      wordsMetrome(xml, _score, text);
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
      qDebug("ExportMusicXml::words userOff.x=%f userOff.y=%f text='%s'",
             text->userOff().x(), text->userOff().y(), qPrintable(text->text()));
      */

      if (text->text() == "") {
            // sometimes empty Texts are present, exporting would result
            // in invalid MusicXML (as an empty direction-type would be created)
            return;
            }

      directionTag(xml, attr, text);
      if (text->type() == Element::Type::REHEARSAL_MARK) {
            // TODO: check if dead code (see rehearsal below)
            xml.stag("direction-type");
            xml.tag("rehearsal", text->text());
            xml.etag();
            }
      else
            wordsMetrome(xml, _score, text);
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
      if (hp->tick() == tick){
          if( hp->hairpinType() == Hairpin::Type::CRESCENDO ){
              if( hp->hairpinCircledTip() ){
                xml.tagE( "wedge type=\"crescendo\" niente=\"yes\"" );
              }
              else{
                xml.tagE( "wedge type=\"crescendo\"" );
              }
          }
          else{
              xml.tagE( "wedge type=\"diminuendo\"" );
          }
      }
      else{
          if( hp->hairpinCircledTip() && hp->hairpinType() == Hairpin::Type::DECRESCENDO )
                xml.tagE( "wedge type=\"stop\" niente=\"yes\"" );
          else
                xml.tagE( "wedge type=\"stop\"" );

      }
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
      Ottava::Type st = ot->ottavaType();
      directionTag(xml, attr, ot);
      xml.stag("direction-type");
      if (ot->tick() == tick) {
            const char* sz = 0;
            const char* tp = 0;
            switch (st) {
                  case Ottava::Type::OTTAVA_8VA:
                        sz = "8";
                        tp = "down";
                        break;
                  case Ottava::Type::OTTAVA_15MA:
                        sz = "15";
                        tp = "down";
                        break;
                  case Ottava::Type::OTTAVA_8VB:
                        sz = "8";
                        tp = "up";
                        break;
                  case Ottava::Type::OTTAVA_15MB:
                        sz = "15";
                        tp = "up";
                        break;
                  default:
                        qDebug("ottava subtype %hhd not understood", st);
                  }
            if (sz && tp)
                  xml.tagE("octave-shift type=\"%s\" size=\"%s\"", tp, sz);
            }
      else {
            if (st == Ottava::Type::OTTAVA_8VA || st == Ottava::Type::OTTAVA_8VB)
                  xml.tagE("octave-shift type=\"stop\" size=\"8\"");
            else if (st == Ottava::Type::OTTAVA_15MA || st == Ottava::Type::OTTAVA_15MB)
                  xml.tagE("octave-shift type=\"stop\" size=\"15\"");
            else
                  qDebug("ottava subtype %hhd not understood", st);
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
      if (!tl->beginText().isEmpty() && tl->tick() == tick) {
            xml.stag("direction-type");
            xml.tag("words", tl->beginText());
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
      Dynamic::Type st = dyn->dynamicType();

      directionTag(xml, attr, dyn);
      xml.stag("direction-type");
      if (st == Dynamic::Type::P
          || st == Dynamic::Type::PP
          || st == Dynamic::Type::PPP
          || st == Dynamic::Type::PPPP
          || st == Dynamic::Type::PPPPP
          || st == Dynamic::Type::PPPPPP
          || st == Dynamic::Type::F
          || st == Dynamic::Type::FF
          || st == Dynamic::Type::FFF
          || st == Dynamic::Type::FFFF
          || st == Dynamic::Type::FFFFF
          || st == Dynamic::Type::FFFFFF
          || st == Dynamic::Type::MP
          || st == Dynamic::Type::MF
          || st == Dynamic::Type::SF
          || st == Dynamic::Type::SFP
          || st == Dynamic::Type::SFPP
          || st == Dynamic::Type::FP
          || st == Dynamic::Type::RF
          || st == Dynamic::Type::RFZ
          || st == Dynamic::Type::SFZ
          || st == Dynamic::Type::SFFZ
          || st == Dynamic::Type::FZ) {
            xml.stag("dynamics");
            xml.tagE(dyn->dynamicTypeName());
            xml.etag();
            }
      else if (st == Dynamic::Type::M || st == Dynamic::Type::Z) {
            xml.stag("dynamics");
            xml.tag("other-dynamics", dyn->dynamicTypeName());
            xml.etag();
            }
      else  {
            QString attr; // TODO TBD
            MScoreTextToMXML mttm("words", attr, t, _score->textStyle(TextStyleType::STAFF), _score->textStyle(TextStyleType::DYNAMICS));
            mttm.write(xml);
            }
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
      if (name == "keyboardPedalPed")
            mxmlName = "pedal type=\"start\"";
      else if (name == "keyboardPedalUp")
            mxmlName = "pedal type=\"stop\"";
      else {
            qDebug("ExportMusicXml::symbol(): %s not supported", qPrintable(name));
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
                        Lyrics::Syllabic syl = (l)->syllabic();
                        QString s = "";
                        switch (syl) {
                              case Lyrics::Syllabic::SINGLE: s = "single"; break;
                              case Lyrics::Syllabic::BEGIN:  s = "begin";  break;
                              case Lyrics::Syllabic::END:    s = "end";    break;
                              case Lyrics::Syllabic::MIDDLE: s = "middle"; break;
                              default:
                                    qDebug("unknown syllabic %hhd", syl);
                              }
                        xml.tag("syllabic", s);
                        QString attr; // TODO TBD
                        MScoreTextToMXML mttm("text", attr, (l)->text(), _score->textStyle(TextStyleType::LYRIC1), _score->textStyle(TextStyleType::LYRIC1));
                        mttm.write(xml);
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
      Jump::Type jtp = jp->jumpType();
      QString words = "";
      QString type  = "";
      QString sound = "";
      if (jtp == Jump::Type::DC) {
            if (jp->text() == "")
                  words = "D.C.";
            else
                  words = jp->text();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == Jump::Type::DC_AL_FINE) {
            if (jp->text() == "")
                  words = "D.C. al Fine";
            else
                  words = jp->text();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == Jump::Type::DC_AL_CODA) {
            if (jp->text() == "")
                  words = "D.C. al Coda";
            else
                  words = jp->text();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == Jump::Type::DS_AL_CODA) {
            if (jp->text() == "")
                  words = "D.S. al Coda";
            else
                  words = jp->text();
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else if (jtp == Jump::Type::DS_AL_FINE) {
            if (jp->text() == "")
                  words = "D.S. al Fine";
            else
                  words = jp->text();
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else if (jtp == Jump::Type::DS) {
            words = "D.S.";
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else
            qDebug("jump type=%hhd not implemented", jtp);
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
      Marker::Type mtp = m->markerType();
      QString words = "";
      QString type  = "";
      QString sound = "";
      if (mtp == Marker::Type::CODA) {
            type = "coda";
            if (m->label() == "")
                  sound = "coda=\"1\"";
            else
                  // LVIFIX hack: force label to "coda" to match to coda label
                  // sound = "coda=\"" + m->label() + "\"";
                  sound = "coda=\"coda\"";
            }
      else if (mtp == Marker::Type::SEGNO) {
            type = "segno";
            if (m->label() == "")
                  sound = "segno=\"1\"";
            else
                  sound = "segno=\"" + m->label() + "\"";
            }
      else if (mtp == Marker::Type::FINE) {
            words = "Fine";
            sound = "fine=\"yes\"";
            }
      else if (mtp == Marker::Type::TOCODA) {
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
            qDebug("marker type=%hhd not implemented", mtp);
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
      if (seg->segmentType() != Segment::Type::ChordRest)
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
      for (Element* e : *m->el()) {
            int wtrack = -1; // track to write jump
            if (strack <= e->track() && e->track() < etrack)
                  wtrack = findTrackForAnnotations(e->track(), m->first(Segment::Type::ChordRest));
            if (track != wtrack)
                  continue;
            switch (e->type()) {
                  case Element::Type::MARKER:
                        {
                        // filter out the markers at measure Start
                        const Marker* const mk = static_cast<const Marker* const>(e);
                        Marker::Type mtp = mk->markerType();
                        if (   mtp == Marker::Type::SEGNO
                               || mtp == Marker::Type::CODA
                               ) {
                              qDebug(" -> handled");
                              attr.doAttr(xml, false);
                              directionMarker(xml, mk);
                              }
                        else if (   mtp == Marker::Type::FINE
                                    || mtp == Marker::Type::TOCODA
                                    ) {
                              // ignore
                              }
                        else {
                              qDebug("repeatAtMeasureStart: marker %hhd not implemented", mtp);
                              }
                        }
                        break;
                  default:
                        qDebug("repeatAtMeasureStart: direction type %s at tick %d not implemented",
                               Element::name(e->type()), m->tick());
                        break;
                  }
            }
      }

//---------------------------------------------------------
//  repeatAtMeasureStop -- write repeats at end of measure
//---------------------------------------------------------

static void repeatAtMeasureStop(Xml& xml, Measure* m, int strack, int etrack, int track)
      {
      for (Element* e : *m->el()) {
            int wtrack = -1; // track to write jump
            if (strack <= e->track() && e->track() < etrack)
                  wtrack = findTrackForAnnotations(e->track(), m->first(Segment::Type::ChordRest));
            if (track != wtrack)
                  continue;
            switch (e->type()) {
                  case Element::Type::MARKER:
                        {
                        // filter out the markers at measure stop
                        const Marker* const mk = static_cast<const Marker* const>(e);
                        Marker::Type mtp = mk->markerType();
                        if (mtp == Marker::Type::FINE || mtp == Marker::Type::TOCODA) {
                              directionMarker(xml, mk);
                              }
                        else if (mtp == Marker::Type::SEGNO || mtp == Marker::Type::CODA) {
                              // ignore
                              }
                        else {
                              qDebug("repeatAtMeasureStop: marker %hhd not implemented", mtp);
                              }
                        }
                        break;
                  case Element::Type::JUMP:
                        directionJump(xml, static_cast<const Jump* const>(e));
                        break;
                  default:
                        qDebug("repeatAtMeasureStop: direction type %s at tick %d not implemented",
                               Element::name(e->type()), m->tick());
                        break;
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
      if (seg->segmentType() == Segment::Type::ChordRest) {
            foreach(const Element* e, seg->annotations()) {

                  int wtrack = -1; // track to write annotation

                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack && e->type() == Element::Type::FRET_DIAGRAM)
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

static void annotations(ExportMusicXml* exp, Xml&, int strack, int etrack, int track, int sstaff, Segment* seg)
      {
      if (seg->segmentType() == Segment::Type::ChordRest) {

            const FretDiagram* fd = findFretDiagram(strack, etrack, track, seg);
            // if (fd) qDebug("annotations seg %p found fretboard diagram %p", seg, fd);

            foreach(const Element* e, seg->annotations()) {

                  int wtrack = -1; // track to write annotation

                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack) {
                        switch (e->type()) {
                              case Element::Type::SYMBOL:
                                    exp->symbol(static_cast<const Symbol*>(e), sstaff);
                                    break;
                              case Element::Type::TEMPO_TEXT:
                                    exp->tempoText(static_cast<const TempoText*>(e), sstaff);
                                    break;
                              case Element::Type::STAFF_TEXT:
                              case Element::Type::TEXT:
                                    exp->words(static_cast<const Text*>(e), sstaff);
                                    break;
                              case Element::Type::DYNAMIC:
                                    exp->dynamic(static_cast<const Dynamic*>(e), sstaff);
                                    break;
                              case Element::Type::HARMONY:
                                    // qDebug("annotations seg %p found harmony %p", seg, e);
                                    exp->harmony(static_cast<const Harmony*>(e), fd /*, sstaff */);
                                    fd = 0; // make sure to write only once ...
                                    break;
                              case Element::Type::REHEARSAL_MARK:
                                    exp->rehearsal(static_cast<const RehearsalMark*>(e), sstaff);
                                    break;
                              case Element::Type::FIGURED_BASS: // handled separately by figuredBass()
                              case Element::Type::FRET_DIAGRAM: // handled using findFretDiagram()
                              case Element::Type::JUMP:         // ignore
                                    break;
                              default:
                                    qDebug("annotations: direction type %s at tick %d not implemented",
                                           Element::name(e->type()), seg->tick());
                                    break;
                              }
                        }
                  } // foreach
            if (fd)
                  // found fd but no harmony, cannot write (MusicXML would be invalid)
                  qDebug("annotations seg %p found fretboard diagram %p w/o harmony: cannot write",
                         seg, fd);
            }
      }

//---------------------------------------------------------
//  figuredBass
//---------------------------------------------------------

static void figuredBass(Xml& xml, int strack, int etrack, int track, const ChordRest* cr, FigBassMap& fbMap)
      {
      Segment* seg = cr->segment();
      if (seg->segmentType() == Segment::Type::ChordRest) {
            foreach(const Element* e, seg->annotations()) {

                  int wtrack = -1; // track to write annotation

                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack) {
                        if (e->type() == Element::Type::FIGURED_BASS) {
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
      if (seg->segmentType() == Segment::Type::ChordRest) {
            int stick = seg->tick();
            for (auto it = exp->score()->spanner().lower_bound(stick); it != exp->score()->spanner().upper_bound(stick); ++it) {
                  Spanner* e = it->second;

                  int wtrack = -1; // track to write spanner
                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack) {
                        switch (e->type()) {
                              case Element::Type::HAIRPIN:
                                    exp->hairpin(static_cast<const Hairpin*>(e), sstaff, seg->tick());
                                    break;
                              case Element::Type::OTTAVA:
                                    exp->ottava(static_cast<const Ottava*>(e), sstaff, seg->tick());
                                    break;
                              case Element::Type::PEDAL:
                                    exp->pedal(static_cast<const Pedal*>(e), sstaff, seg->tick());
                                    break;
                              case Element::Type::TEXTLINE:
                                    exp->textLine(static_cast<const TextLine*>(e), sstaff, seg->tick());
                                    break;
                              case Element::Type::TRILL:
                                    // ignore (written as <note><notations><ornaments><wavy-line>)
                                    break;
                              case Element::Type::SLUR:
                                    // ignore (written as <note><notations><slur>)
                                    break;
                              default:
                                    qDebug("spannerStart: direction type %s at tick %d not implemented",
                                           Element::name(e->type()), seg->tick());
                                    break;
                              }
                        }
                  } // for
            }
      }

//---------------------------------------------------------
//  spannerStop
//---------------------------------------------------------

// called after writing each chord or rest to check if a spanner must be stopped
// loop over all spanners and find spanners in strack ending at tick2
// note that more than one voice may contains notes ending at tick2,
// remember which spanners have already been stopped (the "stopped" set)

static void spannerStop(ExportMusicXml* exp, int strack, int tick2, int sstaff, QSet<const Spanner*>& stopped)
      {
      for (auto it : exp->score()->spanner()) {
            Spanner* e = it.second;

            if (e->tick2() != tick2 || e->track() != strack)
                  continue;

            if (!stopped.contains(e)) {
                  stopped.insert(e);
                  switch (e->type()) {
                        case Element::Type::HAIRPIN:
                              exp->hairpin(static_cast<const Hairpin*>(e), sstaff, -1);
                              break;
                        case Element::Type::OTTAVA:
                              exp->ottava(static_cast<const Ottava*>(e), sstaff, -1);
                              break;
                        case Element::Type::PEDAL:
                              exp->pedal(static_cast<const Pedal*>(e), sstaff, -1);
                              break;
                        case Element::Type::TEXTLINE:
                              exp->textLine(static_cast<const TextLine*>(e), sstaff, -1);
                              break;
                        case Element::Type::TRILL:
                              // ignore (written as <note><notations><ornaments><wavy-line>
                              break;
                        case Element::Type::SLUR:
                              // ignore (written as <note><notations><slur>)
                              break;
                        default:
                              qDebug("spannerStop: direction type %s at tick2 %d not implemented",
                                     Element::name(e->type()), tick2);
                              break;
                        }
                  }
            } // for
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
                  if (el->type() == Element::Type::KEYSIG) {
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
                        if (!(keysigs.value(i)->key() == keysigs.value(0)->key()))
                              singleKey = false;

            // write the keysigs
            if (singleKey) {
                  // keysig applies to all staves
                  keysig(keysigs.value(0)->key(), 0, keysigs.value(0)->visible());
                  }
            else {
                  // staff-specific keysigs
                  foreach(int st, keysigs.keys())
                  keysig(keysigs.value(st)->key(), st + 1, keysigs.value(st)->visible());
                  }
            }
      else {
            // always write a keysig at tick = 0
            if (m->tick() == 0)
                  keysig(Key::C);
            }

      TimeSig* tsig = 0;
      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            if (seg->tick() > m->tick())
                  break;
            Element* el = seg->element(strack);
            if (el && el->type() == Element::Type::TIMESIG)
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

      // must export in transposed pitch to prevent
      // losing the transposition information
      // if necessary, switch concert pitch mode off
      // before export and restore it after export
      bool concertPitch = score()->styleB(StyleIdx::concertPitch);
      if (concertPitch) {
            score()->cmdConcertPitchChanged(false, false);
            score()->doLayout();
            }

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
                              if (st->bracket(j) != BracketType::NO_BRACKET) {
                                    bracketFound = true;
                                    if (i == 0) {
                                          // OK, found bracket in first staff of part
                                          // filter out implicit brackets
                                          if (!(st->bracketSpan(j) == part->nstaves()
                                                && st->bracket(j) == BracketType::BRACE)) {
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
                                          qDebug("bracket starting in staff %d not supported", i + 1);
                                          }
                                    }
                              }
                        }
                  }
            // handle bracket none
            if (!bracketFound && part->nstaves() > 1) {
                  int number = findPartGroupNumber(partGroupEnd);
                  if (number < MAX_PART_GROUPS) {
                        partGroupStart(xml, number + 1, BracketType::NO_BRACKET);
                        partGroupEnd[number] = idx + part->nstaves();
                        }
                  }

            xml.stag(QString("score-part id=\"P%1\"").arg(idx+1));
            xml.tag("part-name", MScoreTextToMXML::toPlainText(part->longName()));
            if (!part->shortName().isEmpty())
                  xml.tag("part-abbreviation", MScoreTextToMXML::toPlainText(part->shortName()));

            if (part->instr()->useDrumset() != DrumsetKind::NONE) {
                  Drumset* drumset = part->instr()->drumset();
                  for (int i = 0; i < 128; ++i) {
                        DrumInstrument di = drumset->drum(i);
                        if (di.notehead != NoteHead::Group::HEAD_INVALID) {
                              xml.stag(QString("score-instrument id=\"P%1-I%2\"").arg(idx+1).arg(i + 1));
                              xml.tag("instrument-name", di.name);
                              xml.etag();
                              }
                        }
                  for (int i = 0; i < 128; ++i) {
                        DrumInstrument di = drumset->drum(i);
                        if (di.notehead != NoteHead::Group::HEAD_INVALID) {
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
                  xml.tag("instrument-name", MScoreTextToMXML::toPlainText(part->longName()));
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
                  if (mb->type() != Element::Type::MEASURE)
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
                        if (currentMeasureB->type() == Element::Type::MEASURE) {
                              previousMeasure = (Measure*) currentMeasureB;
                              break;
                              }
                        }

                  if (!previousMeasure)
                        currentSystem = TopSystem;
                  else if (m->parent() && previousMeasure->parent()) {
                        if (m->parent()->parent() != previousMeasure->parent()->parent())
                              currentSystem = NewPage;
                        else if (m->parent() != previousMeasure->parent())
                              currentSystem = NewSystem;
                        }

                  bool prevMeasLineBreak = false;
                  bool prevMeasPageBreak = false;
                  if (previousMeasure) {
                        prevMeasLineBreak = previousMeasure->lineBreak();
                        prevMeasPageBreak = previousMeasure->pageBreak();
                        }

                  if (currentSystem != NoSystem) {

                        // determine if a new-system or new-page is required
                        QString newThing; // new-[system|page]="yes" or empty
                        if (preferences.musicxmlExportBreaks == MusicxmlExportBreaks::ALL) {
                              if (currentSystem == NewSystem)
                                    newThing = " new-system=\"yes\"";
                              else if (currentSystem == NewPage)
                                    newThing = " new-page=\"yes\"";
                              }
                        else if (preferences.musicxmlExportBreaks == MusicxmlExportBreaks::MANUAL) {
                              if (currentSystem == NewSystem && prevMeasLineBreak)
                                    newThing = " new-system=\"yes\"";
                              else if (currentSystem == NewPage && prevMeasPageBreak)
                                    newThing = " new-page=\"yes\"";
                              }

                        // determine if layout information is required
                        bool doLayout = false;
                        if (preferences.musicxmlExportLayout) {
                              if (currentSystem == TopSystem
                                  || (preferences.musicxmlExportBreaks == MusicxmlExportBreaks::ALL && newThing != "")) {
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

                                    if (currentSystem == NewPage || currentSystem == TopSystem) {
                                          const double topSysDist = getTenthsFromDots(m->pagePos().y()) - tm;
                                          xml.tag("top-system-distance", QString("%1").arg(QString::number(topSysDist,'f',2)) );
                                          }
                                    if (currentSystem == NewSystem) {
                                          // see System::layout2() for the factor 2 * score()->spatium()
                                          const double sysDist = getTenthsFromDots(m->pagePos().y()
                                                                                   - previousMeasure->pagePos().y()
                                                                                   - previousMeasure->bbox().height()
                                                                                   + 2 * score()->spatium()
                                                                                   );
                                          xml.tag("system-distance",
                                                  QString("%1").arg(QString::number(sysDist,'f',2)));
                                          }

                                    xml.etag();
                                    }

                              // Staff layout elements.
                              for (int staffIdx = (staffCount == 0) ? 1 : 0; staffIdx < staves; staffIdx++) {
                                    xml.stag(QString("staff-layout number=\"%1\"").arg(staffIdx + 1));
                                    const double staffDist =
                                          getTenthsFromDots(mb->system()->staff(staffCount + staffIdx - 1)->distanceDown());
                                    xml.tag("staff-distance", QString("%1").arg(QString::number(staffDist,'f',2)));
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
                  Segment* cs2         = m->findSegment(Segment::Type::Clef, tick);
                  Segment* seg         = 0;

                  if (prevMeasure)
                        cs1 = prevMeasure->findSegment(Segment::Type::Clef,  tick);
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
                                          QList<instrString> l = instrument->stringData()->stringList();
                                          for (int i = 0; i < l.size(); i++) {
                                                char step  = ' ';
                                                int alter  = 0;
                                                int octave = 0;
                                                midipitch2xml(l.at(i).pitch, step, alter, octave);
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
                              xml.tag("diatonic",  instrument->transpose().diatonic % 7);
                              xml.tag("chromatic", instrument->transpose().chromatic % 12);
                              int octaveChange = instrument->transpose().chromatic / 12;
                              if (octaveChange != 0)
                                    xml.tag("octave-change", octaveChange);
                              xml.etag();
                              }
                        }

                  // output attribute at start of measure: measure-style
                  measureStyle(xml, attr, m);

                  // set of spanners already stopped in this measure
                  // required to prevent multiple spanner stops for the same spanner
                  QSet<const Spanner*> spannersStopped;

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
                              if (el->type() == Element::Type::BAR_LINE && static_cast<BarLine*>(el)->barLineType() == BarLineType::START_REPEAT)
                                    continue;

                              // generate backup or forward to the start time of the element
                              // but not for breath, which has the same start time as the
                              // previous note, while tick is already at the end of that note
                              if (tick != seg->tick()) {
                                    attr.doAttr(xml, false);
                                    if (el->type() != Element::Type::BREATH)
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
                                                      if(annot->type() == Element::Type::HARMONY && annot->track() == st)
                                                            harmony(static_cast<Harmony*>(annot), 0, (seg1->tick() - seg->tick()) / div);
                                                      }
                                                }
                                          }
                                    figuredBass(xml, strack, etrack, st, static_cast<const ChordRest*>(el), fbMap);
                                    spannerStart(this, strack, etrack, st, sstaff, seg);
                                    }

                              switch (el->type()) {

                                    case Element::Type::CLEF:
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

                                    case Element::Type::KEYSIG:
                                          // ignore
                                          break;

                                    case Element::Type::TIMESIG:
                                          // ignore
                                          break;

                                    case Element::Type::CHORD:
                                          {
                                          Chord* c                 = static_cast<Chord*>(el);
                                          const QList<Lyrics*>* ll = &c->lyricsList();
                                   // ise grace after
                                          if(c){
                                                QList<Chord*> graceNotesBefore;
                                                c->getGraceNotesBefore(&graceNotesBefore);
                                                for (Chord* g : graceNotesBefore) {
                                                      chord(g, sstaff, ll, part->instr()->useDrumset());
                                                      }
                                                chord(c, sstaff, ll, part->instr()->useDrumset());
                                                QList<Chord*> graceNotesAfter;
                                                 c->getGraceNotesAfter(&graceNotesAfter);
                                                 for (Chord* g : graceNotesAfter) {
                                                       chord(g, sstaff, ll, part->instr()->useDrumset());
                                                       }
                                                }
                                          break;
                                          }
                                    case Element::Type::REST:
                                          rest((Rest*)el, sstaff);
                                          break;

                                    case Element::Type::BAR_LINE:
                                          // Following must be enforced (ref MusicXML barline.dtd):
                                          // If location is left, it should be the first element in the measure;
                                          // if location is right, it should be the last element.
                                          // implementation note: BarLineType::START_REPEAT already written by barlineLeft()
                                          // any bars left should be "middle"
                                          // TODO: print barline only if middle
                                          // if (el->subtype() != BarLineType::START_REPEAT)
                                          //       bar((BarLine*) el);
                                          break;
                                    case Element::Type::BREATH:
                                          // ignore, already exported as note articulation
                                          break;

                                    default:
                                          qDebug("ExportMusicXml::write unknown segment type %s", el->name());
                                          break;
                                    }

                              // handle annotations and spanners (directions attached to this note or rest)
                              if (el->isChordRest()) {
                                    int spannerStaff = (st / VOICES) * VOICES;
                                    spannerStop(this, spannerStaff, tick, sstaff, spannersStopped);
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
                  // note: don't use "m->repeatFlags() & Repeat::END" here, because more
                  // barline types need to be handled besides repeat end ("light-heavy")
                  barlineRight(m);
                  xml.etag();
                  }
            staffCount += staves;
            xml.etag();
            }

      xml.etag();

      if (concertPitch) {
            // restore concert pitch
            score()->cmdConcertPitchChanged(true, false);
            score()->doLayout();
            }
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
      if (rootTpc != Tpc::TPC_INVALID) {
            if (h->textStyle().hasFrame())
                  xml.stag(QString("harmony print-frame=\"yes\"").append(relative));
            else
                  xml.stag(QString("harmony print-frame=\"no\"").append(relative));
            xml.stag("root");
            xml.tag("root-step", tpc2stepName(rootTpc));
            int alter = int(tpc2alter(rootTpc));
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
            if (baseTpc != Tpc::TPC_INVALID) {
                  xml.stag("bass");
                  xml.tag("bass-step", tpc2stepName(baseTpc));
                  int alter = int(tpc2alter(baseTpc));
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
            if (h->textStyle().hasFrame())
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
            HDegreeType tp = hd.type();
            if (tp == HDegreeType::ADD || tp == HDegreeType::ALTER || tp == HDegreeType::SUBTRACT) {
                  xml.stag("degree");
                  xml.tag("degree-value", hd.value());
                  xml.tag("degree-alter", hd.alter());
                  switch (tp) {
                        case HDegreeType::ADD:
                              xml.tag("degree-type", "add");
                              break;
                        case HDegreeType::ALTER:
                              xml.tag("degree-type", "alter");
                              break;
                        case HDegreeType::SUBTRACT:
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

