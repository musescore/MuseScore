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
#include "libmscore/undo.h"
#include "libmscore/textline.h"
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
#define clefDebug(...) {}
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
      void doSlurs(Chord* chord, Notations& notations, Xml& xml);

private:
      void doSlurStart(const Slur* s, Notations& notations, Xml& xml);
      void doSlurStop(const Slur* s, Notations& notations, Xml& xml);
      };

//---------------------------------------------------------
//   glissando handler -- prints <glissando> tags
//---------------------------------------------------------

class GlissandoHandler {
      const Note* glissNote[MAX_NUMBER_LEVEL];
      const Note* slideNote[MAX_NUMBER_LEVEL];
      int findNote(const Note* note, int type) const;

public:
      GlissandoHandler();
      void doGlissandoStart(Glissando* gliss, Notations& notations, Xml& xml);
      void doGlissandoStop(Glissando* gliss, Notations& notations, Xml& xml);
      };

//---------------------------------------------------------
//   ExportMusicXml
//---------------------------------------------------------

typedef QHash<const Chord*, const Trill*> TrillHash;
typedef QMap<const Instrument*, int> MxmlInstrumentMap;

class ExportMusicXml {
      Score* _score;
      Xml xml;
      SlurHandler sh;
      GlissandoHandler gh;
      int tick;
      Attributes attr;
      TextLine const* brackets[MAX_NUMBER_LEVEL];
      Hairpin const* hairpins[MAX_NUMBER_LEVEL];
      Ottava const* ottavas[MAX_NUMBER_LEVEL];
      Trill const* trills[MAX_NUMBER_LEVEL];
      int div;
      double millimeters;
      int tenths;
      TrillHash trillStart;
      TrillHash trillStop;
      MxmlInstrumentMap instrMap;

      int findHairpin(const Hairpin* tl) const;
      int findBracket(const TextLine* tl) const;
      int findOttava(const Ottava* tl) const;
      int findTrill(const Trill* tl) const;
      void chord(Chord* chord, int staff, const std::vector<Lyrics*>* ll, bool useDrumset);
      void rest(Rest* chord, int staff);
      void clef(int staff, const Clef* clef);
      void timesig(TimeSig* tsig);
      void keysig(const KeySig* ks, ClefType ct, int staff = 0, bool visible = true);
      void barlineLeft(Measure* m);
      void barlineRight(Measure* m);
      void lyrics(const std::vector<Lyrics*>* ll, const int trk);
      void work(const MeasureBase* measure);
      void calcDivMoveToTick(int t);
      void calcDivisions();
      double getTenthsFromInches(double);
      double getTenthsFromDots(double);
      void keysigTimesig(const Measure* m, const Part* p);
      void chordAttributes(Chord* chord, Notations& notations, Technical& technical,
                           TrillHash& trillStart, TrillHash& trillStop);
      void wavyLineStartStop(Chord* chord, Notations& notations, Ornaments& ornaments,
                             TrillHash& trillStart, TrillHash& trillStop);
      void print(Measure* m, int idx, int staffCount, int staves);
      void findAndExportClef(Measure* m, const int staves, const int strack, const int etrack);
      void writeElement(Element* el, const Measure* m, int sstaff, bool useDrumset);

public:
      ExportMusicXml(Score* s)
            {
            _score = s; tick = 0; div = 1; tenths = 40;
            millimeters = _score->spatium() * tenths / (10 * DPMM);
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
//   color2xml
//---------------------------------------------------------

/**
 Return \a el color.
 */

static QString color2xml(const Element* el)
      {
      if (el->color() != MScore::defaultColor)
            return QString(" color=\"%1\"").arg(el->color().name().toUpper());
      else
            return "";
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
//   findFirstChord -- find first chord (in musical order) for slur s
//   note that this is not necessarily the same as s->startElement()
//---------------------------------------------------------

static const Chord* findFirstChord(const Slur* s)
      {
      const Element* e1 = s->startElement();
      if (e1 == 0 || e1->type() != Element::Type::CHORD) {
            qDebug("no valid start chord for slur %p", s);
            return 0;
            }

      const Element* e2 = s->endElement();
      if (e2 == 0 || e2->type() != Element::Type::CHORD) {
            qDebug("no valid stop chord for slur %p", s);
            return 0;
            }

      const Chord* c1 = static_cast<const Chord*>(e1);
      const Chord* c2 = static_cast<const Chord*>(e2);

      if (c1->tick() < c2->tick())
            return c1;
      else if (c1->tick() > c2->tick())
            return c2;
      else {
            // c1->tick() == c2->tick()
            if (!c1->isGrace() && !c2->isGrace()) {
                  // slur between two regular notes at the same tick
                  // probably shouldn't happen but handle just in case
                  qDebug("invalid slur between chords %p and %p at tick %d", c1, c2, c1->tick());
                  return 0;
                  }
            else if (c1->isGraceBefore() && !c2->isGraceBefore())
                  return c1;  // easy case: c1 first
            else if (c1->isGraceAfter() && !c2->isGraceAfter())
                  return c2;  // easy case: c2 first
            else if (c2->isGraceBefore() && !c1->isGraceBefore())
                  return c2;  // easy case: c2 first
            else if (c2->isGraceAfter() && !c1->isGraceAfter())
                  return c1;  // easy case: c1 first
            else {
                  // both are grace before or both are grace after -> compare grace indexes
                  // (note: higher means closer to the non-grace chord it is attached to)
                  if ((c1->isGraceBefore() && c1->graceIndex() < c2->graceIndex())
                      || (c1->isGraceAfter() && c1->graceIndex() > c2->graceIndex()))
                        return c1;
                  else
                        return c2;
                  }
            }

      // not reached
      return 0;
      }

//---------------------------------------------------------
//   doSlurs
//---------------------------------------------------------

void SlurHandler::doSlurs(Chord* chord, Notations& notations, Xml& xml)
      {
      // loop over all slurs twice, first to handle the stops, then the starts
      for (int i = 0; i < 2; ++i) {
            // search for slur(s) starting or stopping at this chord
            for (auto it : chord->score()->spanner()) {
                  Spanner* sp = it.second;
                  if (sp->generated() || sp->type() != Element::Type::SLUR)
                        continue;
                  if (chord == sp->startElement() || chord == sp->endElement()) {
                        const Slur* s = static_cast<const Slur*>(sp);
                        const Chord* firstChord = findFirstChord(s);
                        if (firstChord) {
                              if (i == 0) {
                                    // first time: do slur stops
                                    if (firstChord != chord)
                                          doSlurStop(s, notations, xml);
                                    }
                              else {
                                    // second time: do slur starts
                                    if (firstChord == chord)
                                          doSlurStart(s, notations, xml);
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   doSlurStart
//---------------------------------------------------------

void SlurHandler::doSlurStart(const Slur* s, Notations& notations, Xml& xml)
      {
      // check if on slur list (i.e. stop already seen)
      int i = findSlur(s);
      // compose tag
      QString tagName = "slur";
      tagName += slurTieLineStyle(s); // define line type
      tagName += color2xml(s);
      tagName += QString(" type=\"start\"%1")
            .arg(s->slurDirection() == Direction::UP ? " placement=\"above\"" : "");

      if (i >= 0) {
            // remove from list and print start
            slur[i] = 0;
            started[i] = false;
            notations.tag(xml);
            tagName += QString(" number=\"%1\"").arg(i + 1);
            xml.tagE(tagName);
            }
      else {
            // find free slot to store it
            i = findSlur(0);
            if (i >= 0) {
                  slur[i] = s;
                  started[i] = true;
                  notations.tag(xml);
                  tagName += QString(" number=\"%1\"").arg(i + 1);
                  xml.tagE(tagName);
                  }
            else
                  qDebug("no free slur slot");
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

void SlurHandler::doSlurStop(const Slur* s, Notations& notations, Xml& xml)
      {
      // check if on slur list
      int i = findSlur(s);
      if (i < 0) {
            // if not, find free slot to store it
            i = findSlur(0);
            if (i >= 0) {
                  slur[i] = s;
                  started[i] = false;
                  notations.tag(xml);
                  xml.tagE(QString("slur type=\"stop\" number=\"%1\"").arg(i + 1));
                  }
            else
                  qDebug("no free slur slot");
            }
      else {
            // found (already started), stop it and remove from list
            slur[i] = 0;
            started[i] = false;
            notations.tag(xml);
            xml.tagE(QString("slur type=\"stop\" number=\"%1\"").arg(i + 1));
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

static void glissando(const Glissando* gli, int number, bool start, Notations& notations, Xml& xml)
      {
      Glissando::Type st = gli->glissandoType();
      QString tagName;
      switch (st) {
            case Glissando::Type::STRAIGHT:
                  tagName = "slide line-type=\"solid\"";
                  break;
            case Glissando::Type::WAVY:
                  tagName = "glissando line-type=\"wavy\"";
                  break;
            default:
                  qDebug("unknown glissando subtype %d", int(st));
                  return;
                  break;
            }
      tagName += QString(" number=\"%1\" type=\"%2\"").arg(number).arg(start ? "start" : "stop");
      tagName += color2xml(gli);
      notations.tag(xml);
      if (start && gli->showText() && gli->text() != "")
            xml.tag(tagName, gli->text());
      else
            xml.tagE(tagName);
      }

//---------------------------------------------------------
//   GlissandoHandler
//---------------------------------------------------------

GlissandoHandler::GlissandoHandler()
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            glissNote[i] = 0;
            slideNote[i] = 0;
            }
      }

//---------------------------------------------------------
//   findNote -- get index of Note in note table for subtype type
//   return -1 if not found
//---------------------------------------------------------

int GlissandoHandler::findNote(const Note* note, int type) const
      {
      if (type != 0 && type != 1) {
            qDebug("GlissandoHandler::findNote: unknown glissando subtype %d", type);
            return -1;
            }
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (type == 0 && slideNote[i] == note) return i;
            if (type == 1 && glissNote[i] == note) return i;
            }
      return -1;
      }

//---------------------------------------------------------
//   doGlissandoStart
//---------------------------------------------------------

void GlissandoHandler::doGlissandoStart(Glissando* gliss, Notations& notations, Xml& xml)
      {
      Glissando::Type type = gliss->glissandoType();
      if (type != Glissando::Type::STRAIGHT && type != Glissando::Type::WAVY) {
            qDebug("doGlissandoStart: unknown glissando subtype %d", int(type));
            return;
            }
      Note* note = static_cast<Note*>(gliss->startElement());
      // check if on chord list
      int i = findNote(note, int(type));
      if (i >= 0) {
            // print error and remove from list
            qDebug("doGlissandoStart: note for glissando/slide %p already on list", gliss);
            if (type == Glissando::Type::STRAIGHT) slideNote[i] = 0;
            if (type == Glissando::Type::WAVY) glissNote[i] = 0;
            }
      // find free slot to store it
      i = findNote(0, int(type));
      if (i >= 0) {
            if (type == Glissando::Type::STRAIGHT) slideNote[i] = note;
            if (type == Glissando::Type::WAVY) glissNote[i] = note;
            glissando(gliss, i + 1, true, notations, xml);
            }
      else
            qDebug("doGlissandoStart: no free slot");
      }

//---------------------------------------------------------
//   doGlissandoStop
//---------------------------------------------------------

void GlissandoHandler::doGlissandoStop(Glissando* gliss, Notations& notations, Xml& xml)
      {
      Glissando::Type type = gliss->glissandoType();
      if (type != Glissando::Type::STRAIGHT && type != Glissando::Type::WAVY) {
            qDebug("doGlissandoStart: unknown glissando subtype %d", int(type));
            return;
            }
      Note* note = static_cast<Note*>(gliss->startElement());
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (type == Glissando::Type::STRAIGHT && slideNote[i] == note) {
                  slideNote[i] = 0;
                  glissando(gliss, i + 1, false, notations, xml);
                  return;
                  }
            if (type == Glissando::Type::WAVY && glissNote[i] == note) {
                  glissNote[i] = 0;
                  glissando(gliss, i + 1, false, notations, xml);
                  return;
                  }
            }
      qDebug("doGlissandoStop: glissando note %p not found", note);
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
// trill handling
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
                  //qDebug("findTrills 3 tr %p startChord %p stopChord %p", tr, startChord, stopChord);

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

// _spatium = DPMM * (millimeter * 10.0 / tenths);

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

static void creditWords(Xml& xml, Score* s, double x, double y, QString just, QString val, const QList<TextFragment>& words)
      {
      // set the default words format
      const TextStyle tsStaff = s->textStyle(TextStyleType::STAFF);
      const QString mtf = s->styleSt(StyleIdx::MusicalTextFont);
      CharFormat defFmt;
      defFmt.setFontFamily(tsStaff.family());
      defFmt.setFontSize(tsStaff.size());

      // export formatted
      xml.stag("credit page=\"1\"");
      QString attr = QString(" default-x=\"%1\"").arg(x);
      attr += QString(" default-y=\"%1\"").arg(y);
      attr += " justify=\"" + just + "\"";
      attr += " valign=\"" + val + "\"";
      MScoreTextToMXML mttm("credit-words", attr, defFmt, mtf);
      mttm.writeTextFragments(words, xml);
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
            for (const Element* element : measure->el()) {
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

                        creditWords(xml, _score, tx, ty, just, val, text->fragmentList());
                        }
                  }
            }

      if (!rights.isEmpty()) {
            // put copyright at the bottom center of the page
            // note: as the copyright metatag contains plain text, special XML characters must be escaped
            TextFragment f(Xml::xmlString(rights));
            const TextStyle tsFooter = _score->textStyle(TextStyleType::FOOTER);
            f.changeFormat(FormatId::FontFamily, tsFooter.family());
            f.changeFormat(FormatId::FontSize, tsFooter.size());
            QList<TextFragment> list;
            list.append(f);
            creditWords(xml, _score, w / 2, bm, "center", "bottom", list);
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
      //qDebug("midipitch2xml(pitch %d) step %c, alter %d, octave %d", pitch, c, alter, octave);
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
      /*
      else
            qDebug("tabpitch2xml(pitch %d, tpc %d) step %s, alter %d, octave %d",
                   pitch, tpc, qPrintable(s), alter, octave);
       */
      }

//---------------------------------------------------------
//   pitch2xml
//---------------------------------------------------------

// TODO validation

static void pitch2xml(const Note* note, QString& s, int& alter, int& octave)
      {

      const Staff* st = note->staff();
      const Instrument* instr = st->part()->instrument();   // TODO: tick
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
      auto spanners = m->score()->spannerMap().findOverlapping(stick, etick);
      for (auto i : spanners) {
            Spanner* el = i.value;
            if (el->type() != Element::Type::VOLTA)
                  continue;
            if (left && el->tick() == stick)
                  return (Volta*) el;
            if (!left && el->tick2() == etick)
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
      for (int i : v->endings()) {
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
                        qDebug("unknown volta subtype %d", int(st));
                        type = "unknown";
                        break;
                  }
            }
      xml.tagE(QString("ending number=\"%1\" type=\"%2\"")
               .arg(number.toLatin1().data())
               .arg(type.toLatin1().data()));
      }

//---------------------------------------------------------
//   barlineLeft -- search for and handle barline left
//---------------------------------------------------------

void ExportMusicXml::barlineLeft(Measure* m)
      {
      bool rs = m->repeatStart();
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
      const Measure* mmR1 = m->mmRest1(); // the multi measure rest this measure is covered by
      const Measure* mmRLst = mmR1->isMMRest() ? mmR1->mmRestLast() : 0; // last measure of replaced sequence of empty measures
      // note: use barlinetype as found in multi measure rest for last measure of replaced sequence
      BarLineType bst = m == mmRLst ? mmR1->endBarLineType() : m->endBarLineType();
      bool visible = m->endBarLineVisible();

      bool needBarStyle = (bst != BarLineType::NORMAL && bst != BarLineType::START_REPEAT) || !visible;
      Volta* volta = findVolta(m, false);
      // detect short and tick barlines
      QString special = "";
      if (bst == BarLineType::NORMAL) {
            const BarLine* bl = m->endBarLine();
            if (bl) {
                  if (bl->span() == 1 && bl->spanFrom() == BARLINE_SPAN_TICK1_FROM && bl->spanTo() == BARLINE_SPAN_TICK1_TO)
                        special = "tick";
                  if (bl->span() == 1 && bl->spanFrom() == BARLINE_SPAN_TICK2_FROM && bl->spanTo() == BARLINE_SPAN_TICK2_TO)
                        special = "tick";
                  if (bl->span() == 1 && bl->spanFrom() == BARLINE_SPAN_SHORT1_FROM && bl->spanTo() == BARLINE_SPAN_SHORT1_TO)
                        special = "short";
                  if (bl->span() == 1 && bl->spanFrom() == BARLINE_SPAN_SHORT2_FROM && bl->spanTo() == BARLINE_SPAN_SHORT2_FROM)
                        special = "short";
                  }
            }
      if (!needBarStyle && !volta && special.isEmpty())
            return;
      xml.stag(QString("barline location=\"right\""));
      if (needBarStyle) {
            if (!visible) {
                  xml.tag("bar-style", QString("none"));
                  }
            else {
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
                              qDebug("ExportMusicXml::bar(): bar subtype %d not supported", int(bst));
                              break;
                        }
                  }
            }
      else if (!special.isEmpty()) {
            xml.tag("bar-style", special);
            }
      if (volta)
            ending(xml, volta, false);
      if (bst == BarLineType::END_REPEAT || bst == BarLineType::END_START_REPEAT)
            {
            if (m->repeatCount() > 2) {
                  xml.tagE(QString("repeat direction=\"backward\" times=\"%1\"").arg(m->repeatCount()));
                  } else {
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
      QString tagName = "time";
      if (st == TimeSigType::FOUR_FOUR)
            tagName += " symbol=\"common\"";
      else if (st == TimeSigType::ALLA_BREVE)
            tagName += " symbol=\"cut\"";
      if (!tsig->visible())
            tagName += " print-object=\"no\"";
      tagName += color2xml(tsig);
      xml.stag(tagName);

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
//   accSymId2alter
//---------------------------------------------------------

static double accSymId2alter(SymId id)
      {
      double res = 0;
      switch (id) {
            case SymId::accidentalDoubleFlat:                      res = -2;   break;
            case SymId::accidentalThreeQuarterTonesFlatZimmermann: res = -1.5; break;
            case SymId::accidentalFlat:                            res = -1;   break;
            case SymId::accidentalQuarterToneFlatStein:            res = -0.5; break;
            case SymId::accidentalNatural:                         res =  0;   break;
            case SymId::accidentalQuarterToneSharpStein:           res =  0.5; break;
            case SymId::accidentalSharp:                           res =  1;   break;
            case SymId::accidentalThreeQuarterTonesSharpStein:     res =  1.5; break;
            case SymId::accidentalDoubleSharp:                     res =  2;   break;
            default: qDebug("accSymId2alter: unsupported sym %s", Sym::id2name(id));
            }
      return res;
      }

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

void ExportMusicXml::keysig(const KeySig* ks, ClefType ct, int staff, bool visible)
      {
      static char table2[]  = "CDEFGAB";
      int po = ClefInfo::pitchOffset(ct); // actually 7 * oct + step for topmost staff line
      //qDebug("keysig st %d key %d custom %d ct %hhd st %d", staff, kse.key(), kse.custom(), ct, staff);
      //qDebug(" pitch offset clef %d stp %d oct %d ", po, po % 7, po / 7);

      QString tagName = "key";
      if (staff)
            tagName += QString(" number=\"%1\"").arg(staff);
      if (!visible)
            tagName += " print-object=\"no\"";
      tagName += color2xml(ks);
      attr.doAttr(xml, true);
      xml.stag(tagName);

      const KeySigEvent kse = ks->keySigEvent();
      const QList<KeySym> keysyms = kse.keySymbols();
      if (kse.custom() && !kse.isAtonal() && keysyms.size() > 0) {

            // non-traditional key signature
            // MusicXML order is left-to-right order, while KeySims in keySymbols()
            // are in insertion order -> sorting required

            // first put the KeySyms in a map
            QMap<qreal, KeySym> map;
            for (const KeySym& ksym : keysyms) {
                  map.insert(ksym.spos.x(), ksym);
                  }
            // then write them (automatically sorted on key)
            for (const KeySym& ksym : map) {
                  int line = static_cast<int>(round(2 * ksym.spos.y()));
                  int step = (po - line) % 7;
                  //qDebug(" keysym sym %d spos %g,%g pos %g,%g -> line %d step %d",
                  //       ksym.sym, ksym.spos.x(), ksym.spos.y(), ksym.pos.x(), ksym.pos.y(), line, step);
                  xml.tag("key-step", QString(QChar(table2[step])));
                  xml.tag("key-alter", accSymId2alter(ksym.sym));
                  xml.tag("key-accidental", accSymId2MxmlString(ksym.sym));
                  }
            }
      else {
            // traditional key signature
            xml.tag("fifths", static_cast<int>(kse.key()));
            switch (kse.mode()) {
                  case KeyMode::NONE:     xml.tag("mode", "none"); break;
                  case KeyMode::MAJOR:    xml.tag("mode", "major"); break;
                  case KeyMode::MINOR:    xml.tag("mode", "minor"); break;
                  case KeyMode::UNKNOWN:
                  default:
                        if (kse.custom())
                              xml.tag("mode", "none");
                  }
            }
      xml.etag();
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

void ExportMusicXml::clef(int staff, const Clef* clef)
      {
      ClefType ct = clef->clefType();
      clefDebug("ExportMusicXml::clef(staff %d, clef %hhd)", staff, ct);

      QString tagName = "clef";
      if (staff)
            tagName += QString(" number=\"%1\"").arg(staff);
      tagName += color2xml(clef);
      attr.doAttr(xml, true);
      xml.stag(tagName);

      QString sign = ClefInfo::sign(ct);
      int line   = ClefInfo::line(ct);
      xml.tag("sign", sign);
      xml.tag("line", line);
      if (ClefInfo::octChng(ct))
            xml.tag("clef-octave-change", ClefInfo::octChng(ct));
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
//   findTrill -- get index of trill in trill table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findTrill(const Trill* tr) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (trills[i] == tr) return i;
      return -1;
      }

//---------------------------------------------------------
//   wavyLineStartStop
//---------------------------------------------------------

void ExportMusicXml::wavyLineStartStop(Chord* chord, Notations& notations, Ornaments& ornaments,
                                       TrillHash& trillStart, TrillHash& trillStop)
      {
      if (trillStop.contains(chord)) {
            const Trill* tr = trillStop.value(chord);
            int n = findTrill(tr);
            if (n >= 0)
                  // trill stop after trill start
                  trills[n] = 0;
            else {
                  // trill stop before trill start
                  n = findTrill(0);
                  if (n >= 0)
                        trills[n] = tr;
                  else
                        qDebug("too many overlapping trills (chord %p staff %d tick %d)",
                               chord, chord->staffIdx(), chord->tick());
                  }
            if (n >= 0) {
                  notations.tag(xml);
                  ornaments.tag(xml);
                  xml.tagE(QString("wavy-line type=\"stop\" number=\"%1\"").arg(n + 1));
                  }
            trillStop.remove(chord);
            }
      if (trillStart.contains(chord)) {
            const Trill* tr = trillStart.value(chord);
            int n = findTrill(tr);
            if (n >= 0)
                  qDebug("wavyLineStartStop error");
            else {
                  n = findTrill(0);
                  if (n >= 0) {
                        trills[n] = tr;
                        // mscore only supports wavy-line with trill-mark
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("trill-mark");
                        QString tagName = "wavy-line type=\"start\"";
                        tagName += QString(" number=\"%1\"").arg(n + 1);
                        tagName += color2xml(tr);
                        xml.tagE(tagName);
                        }
                  else
                        qDebug("too many overlapping trills (chord %p staff %d tick %d)",
                               chord, chord->staffIdx(), chord->tick());
                  trillStart.remove(chord);
                  }
            }
      }

//---------------------------------------------------------
//   hasBreathMark - determine if chord has breath-mark
//---------------------------------------------------------

static Breath* hasBreathMark(Chord* ch)
      {
      int tick = ch->tick() + ch->actualTicks();
      Segment* s = ch->measure()->findSegment(Segment::Type::Breath, tick);
      return s ? static_cast<Breath*>(s->element(ch->track())) : 0;
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
                        default: qDebug("unknown tremolo single %d", int(st)); break;
                        }
                  }
            else if (chord->tremoloChordType() == TremoloChordType::TremoloFirstNote) {
                  type = "start";
                  switch (st) {
                        case TremoloType::C8:  count = 1; break;
                        case TremoloType::C16: count = 2; break;
                        case TremoloType::C32: count = 3; break;
                        case TremoloType::C64: count = 4; break;
                        default: qDebug("unknown tremolo double %d", int(st)); break;
                        }
                  }
            else if (chord->tremoloChordType() == TremoloChordType::TremoloSecondNote) {
                  type = "stop";
                  switch (st) {
                        case TremoloType::C8:  count = 1; break;
                        case TremoloType::C16: count = 2; break;
                        case TremoloType::C32: count = 3; break;
                        case TremoloType::C64: count = 4; break;
                        default: qDebug("unknown tremolo double %d", int(st)); break;
                        }
                  }
            else qDebug("unknown tremolo subtype %d", int(st));


            if (type != "" && count > 0) {
                  notations.tag(xml);
                  ornaments.tag(xml);
                  QString tagName = "tremolo";
                  tagName += QString(" type=\"%1\"").arg(type);
                  if (type == "single" || type == "start")
                        tagName += color2xml(tr);
                  xml.tag(tagName, count);
                  }
            }
      }


//---------------------------------------------------------
//   fermatas
//---------------------------------------------------------

static void fermatas(const QVector<Articulation*>& cra, Xml& xml, Notations& notations)
      {
      for (const Articulation* a : cra) {
            if (a->isFermata()) {
                  notations.tag(xml);
                  QString tagName = "fermata";
                  tagName += QString(" type=\"%1\"").arg(a->up() ? "upright" : "inverted");
                  tagName += color2xml(a);
                  SymId id = a->symId();
                  if (id == SymId::fermataAbove || id == SymId::fermataBelow)
                        xml.tagE(tagName);
                  else if (id == SymId::fermataShortAbove || id == SymId::fermataShortBelow)
                        xml.tag(tagName, "angled");
                  // MusicXML does not support the very long fermata,
                  // export as long fermata (better than not exporting at all)
                  else if (id == SymId::fermataLongAbove || id == SymId::fermataLongBelow
                           || id == SymId::fermataVeryLongAbove || id == SymId::fermataVeryLongBelow)
                        xml.tag(tagName, "square");
                  }
            }
      }

//---------------------------------------------------------
//   chordAttributes
//---------------------------------------------------------

void ExportMusicXml::chordAttributes(Chord* chord, Notations& notations, Technical& technical,
                                     TrillHash& trillStart, TrillHash& trillStop)
      {
      const QVector<Articulation*>& na = chord->articulations();
      // first output the fermatas
      fermatas(na, xml, notations);

      // then the attributes whose elements are children of <articulations>
      Articulations articulations;
      for (const Articulation* a : na) {
            switch (a->symId()) {
                  case SymId::fermataAbove:
                  case SymId::fermataBelow:
                  case SymId::fermataShortAbove:
                  case SymId::fermataShortBelow:
                  case SymId::fermataLongAbove:
                  case SymId::fermataLongBelow:
                  case SymId::fermataVeryLongAbove:
                  case SymId::fermataVeryLongBelow:
                        // ignore, already handled
                        break;

                  case SymId::articAccentAbove:
                  case SymId::articAccentBelow:
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("accent");
                        break;

                  case SymId::articStaccatoAbove:
                  case SymId::articStaccatoBelow:
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("staccato");
                        break;

                  case SymId::articStaccatissimoAbove:
                  case SymId::articStaccatissimoBelow:
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("staccatissimo");
                        break;

                  case SymId::articTenutoAbove:
                  case SymId::articTenutoBelow:
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("tenuto");
                        break;

                  case SymId::articMarcatoAbove:
                  case SymId::articMarcatoBelow:
                        notations.tag(xml);
                        articulations.tag(xml);
                        if (a->up())
                              xml.tagE("strong-accent type=\"up\"");
                        else
                              xml.tagE("strong-accent type=\"down\"");
                        break;

                  case SymId::articTenutoStaccatoAbove:
                  case SymId::articTenutoStaccatoBelow:
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("detached-legato");
                        break;

                  case SymId::ornamentTurnInverted:
                  case SymId::ornamentTurn:
                  case SymId::ornamentTrill:
                  case SymId::ornamentMordent:
                  case SymId::ornamentMordentInverted:
                  case SymId::ornamentTremblement:
                  case SymId::ornamentPrallMordent:
                  case SymId::ornamentUpPrall:
                  case SymId::ornamentDownPrall:
                  case SymId::ornamentUpMordent:
                  case SymId::ornamentDownMordent:
                  case SymId::ornamentPrallDown:
                  case SymId::ornamentPrallUp:
                  case SymId::ornamentLinePrall:
                  case SymId::ornamentPrecompSlide:
                        // ignore, handled with ornaments

                  case SymId::brassMuteOpen:
                  case SymId::brassMuteClosed:
                  case SymId::stringsUpBow:
                  case SymId::stringsDownBow:
                  case SymId::pluckedSnapPizzicatoAbove:
                  case SymId::stringsThumbPosition:
                        // ignore, handled with technical
                        break;
                  default:
                        qDebug("unknown chord attribute %s", qPrintable(a->userName()));
                        break;
                  }
            }

      if (Breath* b = hasBreathMark(chord)) {
            notations.tag(xml);
            articulations.tag(xml);
            xml.tagE(b->isCaesura() ? "caesura" : "breath-mark");
            }

      for (Element* e : chord->el()) {
            qDebug("chordAttributes: el %p type %d (%s)", e, int(e->type()), e->name());
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
                              qDebug("unknown ChordLine subtype %d", int(cl->chordLineType()));
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
      for (const Articulation* a : na) {
            switch (a->symId()) {
                  case SymId::fermataAbove:
                  case SymId::fermataBelow:
                  case SymId::fermataShortAbove:
                  case SymId::fermataShortBelow:
                  case SymId::fermataLongAbove:
                  case SymId::fermataLongBelow:
                  case SymId::fermataVeryLongAbove:
                  case SymId::fermataVeryLongBelow:
                  case SymId::articAccentAbove:
                  case SymId::articAccentBelow:
                  case SymId::articStaccatoAbove:
                  case SymId::articStaccatoBelow:
                  case SymId::articStaccatissimoAbove:
                  case SymId::articStaccatissimoBelow:
                  case SymId::articTenutoAbove:
                  case SymId::articTenutoBelow:
                  case SymId::articMarcatoAbove:
                  case SymId::articMarcatoBelow:
                  case SymId::articTenutoStaccatoAbove:
                  case SymId::articTenutoStaccatoBelow:
                        // ignore, already handled
                        break;

                  case SymId::ornamentTurnInverted:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-turn");
                        break;
                  case SymId::ornamentTurn:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("turn");
                        break;
                  case SymId::ornamentTrill:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("trill-mark");
                        break;
                  case SymId::ornamentMordentInverted:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent");
                        // xml.tagE("inverted-mordent");
                        break;
                  case SymId::ornamentMordent:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        // xml.tagE("mordent");
                        xml.tagE("inverted-mordent");
                        break;
                  case SymId::ornamentTremblement:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\"");
                        break;
                  case SymId::ornamentPrallMordent:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent long=\"yes\"");
                        break;
                  case SymId::ornamentUpPrall:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" approach=\"below\"");
                        break;
                  case SymId::ornamentDownPrall:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" approach=\"above\"");
                        break;
                  case SymId::ornamentUpMordent:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent long=\"yes\" approach=\"below\"");
                        break;
                  case SymId::ornamentDownMordent:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent long=\"yes\" approach=\"above\"");
                        break;
                  case SymId::ornamentPrallDown:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" departure=\"below\"");
                        break;
                  case SymId::ornamentPrallUp:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" departure=\"above\"");
                        break;
                  case SymId::ornamentLinePrall:
                        // MusicXML 3.0 does not distinguish between downprall and lineprall
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent long=\"yes\" approach=\"above\"");
                        break;
                  case SymId::ornamentPrecompSlide:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("schleifer");
                        break;
                  case SymId::brassMuteOpen:
                  case SymId::brassMuteClosed:
                  case SymId::stringsUpBow:
                  case SymId::stringsDownBow:
                  case SymId::pluckedSnapPizzicatoAbove:
                  case SymId::stringsThumbPosition:
                        // ignore, handled with technical
                        break;
                  default:
                        qDebug("unknown chord attribute %s", qPrintable(a->userName()));
                        break;
                  }
            }

      tremoloSingleStartStop(chord, notations, ornaments, xml);
      wavyLineStartStop(chord, notations, ornaments, trillStart, trillStop);
      ornaments.etag(xml);

      // and finally the attributes whose elements are children of <technical>
      for (const Articulation* a : na) {
            switch (a->symId()) {
                  case SymId::brassMuteClosed:
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("stopped");
                        break;
                  case SymId::stringsUpBow:
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("up-bow");
                        break;
                  case SymId::stringsDownBow:
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("down-bow");
                        break;
                  case SymId::pluckedSnapPizzicatoAbove:
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("snap-pizzicato");
                        break;
                  case SymId::brassMuteOpen:
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("open-string");
                        break;
                  case SymId::stringsThumbPosition:
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("thumb-position");
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
            case ArpeggioType::UP:          // fall through
            case ArpeggioType::UP_STRAIGHT: // not supported by MusicXML, export as normal arpeggio
                  notations.tag(xml);
                  xml.tagE("arpeggiate direction=\"up\"");
                  break;
            case ArpeggioType::DOWN:          // fall through
            case ArpeggioType::DOWN_STRAIGHT: // not supported by MusicXML, export as normal arpeggio
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
/* NO LONGER NEEDED
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
*/
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
      /*
      qDebug("determineTupletNormalTicks t %p baselen %d", t, t->baseLen().ticks());
      for (int i = 0; i < t->elements().size(); ++i)
            qDebug("determineTupletNormalTicks t %p i %d ticks %d", t, i, t->elements().at(i)->duration().ticks());
            */
      for (unsigned int i = 1; i < t->elements().size(); ++i)
            if (t->elements().at(0)->duration().ticks() != t->elements().at(i)->duration().ticks())
                  return t->baseLen().ticks();
      if (t->elements().size() != (unsigned)(t->ratio().numerator()))
            return t->baseLen().ticks();
      return 0;
      }

//---------------------------------------------------------
//   writeBeam
//---------------------------------------------------------

static void writeBeam(Xml& xml, ChordRest* cr, Beam* b)
      {
      const auto& elements = b->elements();
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
//   instrId
//---------------------------------------------------------

static QString instrId(int partNr, int instrNr)
      {
      return QString("id=\"P%1-I%2\"").arg(partNr).arg(instrNr);
      }

//---------------------------------------------------------
//   chord
//---------------------------------------------------------

/**
 Write \a chord on \a staff with lyriclist \a ll.

 For a single-staff part, \a staff equals zero, suppressing the <staff> element.
 */

void ExportMusicXml::chord(Chord* chord, int staff, const std::vector<Lyrics*>* ll, bool useDrumset)
      {
      Part* part = chord->score()->staff(chord->track() / VOICES)->part();
      int partNr = _score->parts().indexOf(part);
      int instNr = instrMap.value(part->instrument(tick), -1);
      /*
      qDebug("chord() %p parent %p isgrace %d #gracenotes %d graceidx %d",
             chord, chord->parent(), chord->isGrace(), chord->graceNotes().size(), chord->graceIndex());
      qDebug("track %d tick %d part %p nr %d instr %p nr %d",
             chord->track(), chord->tick(), part, partNr, part->instrument(tick), instNr);
      for (Element* e : chord->el())
            qDebug("chord %p el %p", chord, e);
       */
      std::vector<Note*> nl = chord->notes();
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

      for (Note* note : nl) {
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
            else if (note->chord()->small()) // need this only once per chord
                  xml.tagE("cue");

            // step / alter / octave
            QString step;
            int alter = 0;
            int octave = 0;
            if (chord->staff() && chord->staff()->isTabStaff()) {
                  tabpitch2xml(note->pitch(), note->tpc(), step, alter, octave);
                  }
            else {
                  if (!useDrumset) {
                        pitch2xml(note, step, alter, octave);
                        }
                  else {
                        unpitch2xml(note, step, octave);
                        }
                  }
            xml.stag(useDrumset ? "unpitched" : "pitch");
            xml.tag(useDrumset  ? "display-step" : "step", step);
            // Check for microtonal accidentals and overwrite "alter" tag
            Accidental* acc = note->accidental();
            double alter2 = 0.0;
            if (acc) {
                  switch (acc->accidentalType()) {
                        case AccidentalType::MIRRORED_FLAT:  alter2 = -0.5; break;
                        case AccidentalType::SHARP_SLASH:    alter2 = 0.5;  break;
                        case AccidentalType::MIRRORED_FLAT2: alter2 = -1.5; break;
                        case AccidentalType::SHARP_SLASH4:   alter2 = 1.5;  break;
                        default:                                             break;
                        }
                  }
            if (alter && !alter2)
                  xml.tag("alter", alter);
            if (!alter && alter2)
                  xml.tag("alter", alter2);
            // TODO what if both alter and alter2 are present? For Example: playing with transposing instruments
            xml.tag(useDrumset ? "display-octave" : "octave", octave);
            xml.etag();

            // time signature stretch factor
            const Fraction str = note->chord()->staff()->timeStretch(note->chord()->tick());
            // chord's actual ticks corrected for stretch
            const int strActTicks = note->chord()->actualTicks() * str.numerator() / str.denominator();

            // duration
            if (!grace)
                  xml.tag("duration", strActTicks / div);

            if (note->tieBack())
                  xml.tagE("tie type=\"stop\"");
            if (note->tieFor())
                  xml.tagE("tie type=\"start\"");

            // instrument for multi-instrument or unpitched parts
            if (!useDrumset) {
                  if (instrMap.size() > 1 && instNr >= 0)
                        xml.tagE(QString("instrument %1").arg(instrId(partNr + 1, instNr + 1)));
                  }
            else
                  xml.tagE(QString("instrument %1").arg(instrId(partNr + 1, note->pitch() + 1)));

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

            QString s = tick2xml(strActTicks * actNotes * tremCorr / nrmNotes, &dots);
            if (s.isEmpty())
                  qDebug("no note type found for ticks %d", strActTicks);

            if (note->small())
                  xml.tag("type size=\"cue\"", s);
            else
                  xml.tag("type", s);
            for (int ni = dots; ni > 0; ni--)
                  xml.tagE("dot");

            // accidental
            if (acc) {
                  QString s = accidentalType2MxmlString(acc->accidentalType());
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
            noteheadTagname += color2xml(note);
            bool leftParenthesis, rightParenthesis = false;
            for (Element* elem : note->el()) {
                  if (elem->type() == Element::Type::SYMBOL) {
                        Symbol* s = static_cast<Symbol*>(elem);
                        if (s->sym() == SymId::noteheadParenthesisLeft)
                              leftParenthesis = true;
                        else if (s->sym() == SymId::noteheadParenthesisRight)
                              rightParenthesis = true;
                        }
                  }
            if (rightParenthesis && leftParenthesis)
                  noteheadTagname += " parentheses=\"yes\"";
            if (note->headType() == NoteHead::Type::HEAD_QUARTER)
                  noteheadTagname += " filled=\"yes\"";
            else if ((note->headType() == NoteHead::Type::HEAD_HALF) || (note->headType() == NoteHead::Type::HEAD_WHOLE))
                  noteheadTagname += " filled=\"no\"";
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
            else if (note->color() != MScore::defaultColor)
                  xml.tag(noteheadTagname, "normal");
            else if (rightParenthesis && leftParenthesis)
                  xml.tag(noteheadTagname, "normal");
            else if (note->headType() != NoteHead::Type::HEAD_AUTO)
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
                  if (!grace)
                        tupletStartStop(chord, notations, xml);

                  sh.doSlurs(chord, notations, xml);

                  chordAttributes(chord, notations, technical, trillStart, trillStop);
                  }

            for (const Element* e : note->el()) {
                  if (e->type() == Element::Type::FINGERING) {
                        Text* f = (Text*)e;
                        notations.tag(xml);
                        technical.tag(xml);
                        QString t = MScoreTextToMXML::toPlainText(f->xmlText());
                        if (f->textStyleType() == TextStyleType::RH_GUITAR_FINGERING)
                              xml.tag("pluck", t);
                        else if (f->textStyleType() == TextStyleType::LH_GUITAR_FINGERING)
                              xml.tag("fingering", t);
                        else if (f->textStyleType() == TextStyleType::FINGERING) {
                              // for generic fingering, try to detect plucking
                              // (backwards compatibility with MuseScore 1.x)
                              // p, i, m, a, c represent the plucking finger
                              if (t == "p" || t == "i" || t == "m" || t == "a" || t == "c")
                                    xml.tag("pluck", t);
                              else
                                    xml.tag("fingering", t);
                              }
                        else if (f->textStyleType() == TextStyleType::STRING_NUMBER) {
                              bool ok;
                              int i = t.toInt(&ok);
                              if (ok) {
                                    if (i == 0)
                                          xml.tagE("open-string");
                                    else if (i > 0)
                                          xml.tag("string", t);
                                    }
                              if (!ok || i < 0)
                                    qDebug("invalid string number '%s'", qPrintable(t));
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
            for (Spanner* spanner : note->spannerFor())
                  if (spanner->type() == Element::Type::GLISSANDO) {
                        gh.doGlissandoStart(static_cast<Glissando*>(spanner), notations, xml);
                        }
            for (Spanner* spanner : note->spannerBack())
                  if (spanner->type() == Element::Type::GLISSANDO) {
                        gh.doGlissandoStop(static_cast<Glissando*>(spanner), notations, xml);
                        }
            // write glissando (only for last note)
            /*
            Chord* ch = nextChord(chord);
            if ((note == nl.back()) && ch && ch->glissando()) {
                  gh.doGlissandoStart(ch, notations, xml);
                  }
            if (chord->glissando()) {
                  gh.doGlissandoStop(chord, notations, xml);
                  }
            */
            notations.etag(xml);
            // write lyrics (only for first note)
            if (!grace && (note == nl.front()) && ll)
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

      if (clef != ClefType::TAB && clef != ClefType::TAB_SERIF && clef != ClefType::TAB4 && clef != ClefType::TAB4_SERIF) {
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
            if (rest->small())
                  xml.tag("type size=\"cue\"", s);
            else
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
      fermatas(rest->articulations(), xml, notations);
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
                     || el->type() == Element::Type::INSTRUMENT_CHANGE
                     || el->type() == Element::Type::REHEARSAL_MARK
                     || el->type() == Element::Type::STAFF_TEXT
                     || el->type() == Element::Type::SYMBOL
                     || el->type() == Element::Type::TEXT) {
                  // handle other elements attached (e.g. via Segment / Measure) to a system
                  // find the system containing this element
                  for (const Element* e = el; e; e = e->parent()) {
                        if (e->type() == Element::Type::SYSTEM) pel = e;
                        }
                  }
            else
                  qDebug("directionTag() element %p tp=%d (%s) not supported",
                         el, int(el->type()), el->name());

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

                  if (el->isHairpin() || el->isOttava() || el->isPedal() || el->isTextLine()) {
                        // for the line type elements the reference point is vertically centered
                        // actual position info is in the segments
                        // compare the segment's canvas ypos with the staff's center height
                        if (seg->pagePos().y() < sys->pagePos().y() + bb.y() + bb.height() / 2)
                              tagname += " placement=\"above\"";
                        else
                              tagname += " placement=\"below\"";
                        }
                  else if (el->isDynamic()) {
                        tagname += " placement=\"";
                        tagname += el->placement() == Element::Placement::ABOVE
                              ? "above" : "below";
                        tagname += "\"";
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
                  qDebug("bracket subtype %d not understood", int(bracket));
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
            default: qDebug("findUnit: unknown DurationType %d", int(val));
            }
      return true;
      }

static bool findMetronome(const QList<TextFragment>& list,
                          QList<TextFragment>& wordsLeft,  // words left of metronome
                          bool& hasParen,      // parenthesis
                          QString& metroLeft,  // left part of metronome
                          QString& metroRight, // right part of metronome
                          QList<TextFragment>& wordsRight // words right of metronome
                          )
      {
      QString words = MScoreTextToMXML::toPlainTextPlusSymbols(list);
      //qDebug("findMetronome('%s')", qPrintable(words));
      hasParen   = false;
      metroLeft  = "";
      metroRight = "";
      int metroPos = -1;   // metronome start position
      int metroLen = 0;    // metronome length

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

                  metroLeft = s2;
                  metroRight = s5;

                  metroPos = pos1;               // metronome position
                  metroLen = len1 + len2 + len3; // metronome length
                  if (hasParen) {
                        metroPos -= 1;           // move left one position
                        metroLen += 2;           // add length of '(' and ')'
                        }

                  // calculate starting position corrected for surrogate pairs
                  // (which were ignored by toPlainTextPlusSymbols())
                  int corrPos = metroPos;
                  for (int i = 0; i < metroPos; ++i)
                        if (words.at(i).isHighSurrogate())
                              --corrPos;
                  metroPos = corrPos;

                  /*
                  qDebug("-> found '%s'%s' hasParen %d metro pos %d len %d",
                         qPrintable(metroLeft),
                         qPrintable(metroRight),
                         hasParen, metroPos, metroLen
                         );
                   */
                  QList<TextFragment> mid; // not used
                  MScoreTextToMXML::split(list, metroPos, metroLen, wordsLeft, mid, wordsRight);
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
      //qDebug("wordsMetrome('%s')", qPrintable(text->xmlText()));
      const QList<TextFragment> list = text->fragmentList();
      QList<TextFragment>       wordsLeft;  // words left of metronome
      bool hasParen;                        // parenthesis
      QString metroLeft;                    // left part of metronome
      QString metroRight;                   // right part of metronome
      QList<TextFragment>       wordsRight; // words right of metronome

      // set the default words format
      const TextStyle tsStaff = s->textStyle(TextStyleType::STAFF);
      const QString mtf = s->styleSt(StyleIdx::MusicalTextFont);
      CharFormat defFmt;
      defFmt.setFontFamily(tsStaff.family());
      defFmt.setFontSize(tsStaff.size());

      if (findMetronome(list, wordsLeft, hasParen, metroLeft, metroRight, wordsRight)) {

            if (wordsLeft.size() > 0) {
                  xml.stag("direction-type");
                  QString attr; // TODO TBD
                  MScoreTextToMXML mttm("words", attr, defFmt, mtf);
                  mttm.writeTextFragments(wordsLeft, xml);
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

            if (wordsRight.size() > 0) {
                  xml.stag("direction-type");
                  QString attr; // TODO TBD
                  MScoreTextToMXML mttm("words", attr, defFmt, mtf);
                  mttm.writeTextFragments(wordsRight, xml);
                  xml.etag();
                  }
            }

      else {
            xml.stag("direction-type");
            QString attr;
            if (text->textStyle().hasFrame()) {
                  if (text->textStyle().circle())
                        attr = " enclosure=\"circle\"";
                  else
                        attr = " enclosure=\"rectangle\"";
                  }
            MScoreTextToMXML mttm("words", attr, defFmt, mtf);
            //qDebug("words('%s')", qPrintable(text->text()));
            mttm.writeTextFragments(text->fragmentList(), xml);
            xml.etag();
            }
      }

void ExportMusicXml::tempoText(TempoText const* const text, int staff)
      {
      /*
      qDebug("ExportMusicXml::tempoText(TempoText='%s')", qPrintable(text->xmlText()));
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
      qDebug("ExportMusicXml::words userOff.x=%f userOff.y=%f xmlText='%s' plainText='%s'",
             text->userOff().x(), text->userOff().y(),
             qPrintable(text->xmlText()),
             qPrintable(text->plainText()));
      */

      if (text->plainText() == "") {
            // sometimes empty Texts are present, exporting would result
            // in invalid MusicXML (as an empty direction-type would be created)
            return;
            }

      directionTag(xml, attr, text);
      wordsMetrome(xml, _score, text);
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   rehearsal
//---------------------------------------------------------

void ExportMusicXml::rehearsal(RehearsalMark const* const rmk, int staff)
      {
      if (rmk->plainText() == "") {
            // sometimes empty Texts are present, exporting would result
            // in invalid MusicXML (as an empty direction-type would be created)
            return;
            }

      directionTag(xml, attr, rmk);
      xml.stag("direction-type");
      QString attr;
      if (!rmk->textStyle().hasFrame()) attr = " enclosure=\"none\"";
      // set the default words format
      const TextStyle tsStaff = _score->textStyle(TextStyleType::STAFF);
      const QString mtf = _score->styleSt(StyleIdx::MusicalTextFont);
      CharFormat defFmt;
      defFmt.setFontFamily(tsStaff.family());
      defFmt.setFontSize(tsStaff.size());
      // write formatted
      MScoreTextToMXML mttm("rehearsal", attr, defFmt, mtf);
      mttm.writeTextFragments(rmk->fragmentList(), xml);
      xml.etag();
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   findHairpin -- get index of hairpin in hairpin table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findHairpin(const Hairpin* hp) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (hairpins[i] == hp) return i;
      return -1;
      }

//---------------------------------------------------------
//   hairpin
//---------------------------------------------------------

void ExportMusicXml::hairpin(Hairpin const* const hp, int staff, int tick)
      {
      int n = findHairpin(hp);
      if (n >= 0)
            hairpins[n] = 0;
      else {
            n = findHairpin(0);
            if (n >= 0)
                  hairpins[n] = hp;
            else {
                  qDebug("too many overlapping hairpins (hp %p staff %d tick %d)", hp, staff, tick);
                  return;
                  }
            }

      directionTag(xml, attr, hp);
      xml.stag("direction-type");

      if (hp->tick() == tick) {
            if ( hp->hairpinType() == HairpinType::CRESC_HAIRPIN ) {
                  if ( hp->hairpinCircledTip() ) {
                        xml.tagE(QString("wedge type=\"crescendo\" niente=\"yes\" number=\"%1\"").arg(n + 1));
                        }
                  else {
                        xml.tagE(QString("wedge type=\"crescendo\" number=\"%1\"").arg(n + 1));
                        }
                  }
            else {
                  xml.tagE(QString("wedge type=\"diminuendo\" number=\"%1\"").arg(n + 1));
                  }
            }
      else {
            if ( hp->hairpinCircledTip() && hp->hairpinType() == HairpinType::DECRESC_HAIRPIN )
                  xml.tagE(QString("wedge type=\"stop\" niente=\"yes\" number=\"%1\"").arg(n + 1));
            else
                  xml.tagE(QString("wedge type=\"stop\" number=\"%1\"").arg(n + 1));

            }
      xml.etag();
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   findOttava -- get index of ottava in ottava table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findOttava(const Ottava* ot) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (ottavas[i] == ot) return i;
      return -1;
      }

//---------------------------------------------------------
//   ottava
// <octave-shift type="down" size="8" relative-y="14"/>
// <octave-shift type="stop" size="8"/>
//---------------------------------------------------------

void ExportMusicXml::ottava(Ottava const* const ot, int staff, int tick)
      {
      int n = findOttava(ot);
      if (n >= 0)
            ottavas[n] = 0;
      else {
            n = findOttava(0);
            if (n >= 0)
                  ottavas[n] = ot;
            else {
                  qDebug("too many overlapping ottavas (ot %p staff %d tick %d)", ot, staff, tick);
                  return;
                  }
            }

      directionTag(xml, attr, ot);
      xml.stag("direction-type");

      Ottava::Type st = ot->ottavaType();
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
                  xml.tagE(QString("octave-shift type=\"%1\" size=\"%2\" number=\"%3\"").arg(tp).arg(sz).arg(n + 1));
            }
      else {
            if (st == Ottava::Type::OTTAVA_8VA || st == Ottava::Type::OTTAVA_8VB)
                  xml.tagE(QString("octave-shift type=\"stop\" size=\"8\" number=\"%1\"").arg(n + 1));
            else if (st == Ottava::Type::OTTAVA_15MA || st == Ottava::Type::OTTAVA_15MB)
                  xml.tagE(QString("octave-shift type=\"stop\" size=\"15\" number=\"%1\"").arg(n + 1));
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
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (brackets[i] == tl) return i;
      return -1;
      }

//---------------------------------------------------------
//   textLine
//---------------------------------------------------------

void ExportMusicXml::textLine(TextLine const* const tl, int staff, int tick)
      {
      int n = findBracket(tl);
      if (n >= 0)
            brackets[n] = 0;
      else {
            n = findBracket(0);
            if (n >= 0)
                  brackets[n] = tl;
            else {
                  qDebug("too many overlapping textlines (tl %p staff %d tick %d)", tl, staff, tick);
                  return;
                  }
            }

      QString rest;
      QPointF p;

      // special case: a dashed line w/o hooks is written as dashes
      bool dashes = tl->lineStyle() == Qt::DashLine && !tl->beginHook() && !tl->endHook();

      QString lineEnd = "none";
      QString type;
      bool hook = false;
      double hookHeight = 0.0;
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
      QSet<QString> set; // the valid MusicXML dynamics
      set << "f" << "ff" << "fff" << "ffff" << "fffff" << "ffffff"
          << "fp" << "fz"
          << "mf" << "mp"
          << "p" << "pp" << "ppp" << "pppp" << "ppppp" << "pppppp"
          << "rf" << "rfz"
          << "sf" << "sffz" << "sfp" << "sfpp" << "sfz";

      directionTag(xml, attr, dyn);

      xml.stag("direction-type");

      xml.stag("dynamics");
      QString dynTypeName = dyn->dynamicTypeName();
      if (set.contains(dynTypeName)) {
            xml.tagE(dynTypeName);
            }
      else if (dynTypeName != "") {
            xml.tag("other-dynamics", dynTypeName);
            }
      xml.etag();

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

void ExportMusicXml::lyrics(const std::vector<Lyrics*>* ll, const int trk)
      {
      for (const Lyrics* l : *ll) {
            if (l && !l->xmlText().isEmpty()) {
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
                                    qDebug("unknown syllabic %d", int(syl));
                              }
                        xml.tag("syllabic", s);
                        QString attr; // TODO TBD
                        // set the default words format
                        const TextStyle tsStaff = _score->textStyle(TextStyleType::LYRIC1);
                        const QString mtf = _score->styleSt(StyleIdx::MusicalTextFont);
                        CharFormat defFmt;
                        defFmt.setFontFamily(tsStaff.family());
                        defFmt.setFontSize(tsStaff.size());
                        // write formatted
                        MScoreTextToMXML mttm("text", attr, defFmt, mtf);
                        mttm.writeTextFragments(l->fragmentList(), xml);
#if 0
                        /*
                         Temporarily disabled because it doesn't work yet (and thus breaks the regression test).
                         See MusicXml::xmlLyric: "// TODO-WS      l->setTick(tick);"
                        if((l)->endTick() > 0)
                              xml.tagE("extend");
                        */
#else
                        if (l->ticks())
                              xml.tagE("extend");
#endif
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
            if (jp->xmlText() == "")
                  words = "D.C.";
            else
                  words = jp->xmlText();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == Jump::Type::DC_AL_FINE) {
            if (jp->xmlText() == "")
                  words = "D.C. al Fine";
            else
                  words = jp->xmlText();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == Jump::Type::DC_AL_CODA) {
            if (jp->xmlText() == "")
                  words = "D.C. al Coda";
            else
                  words = jp->xmlText();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == Jump::Type::DS_AL_CODA) {
            if (jp->xmlText() == "")
                  words = "D.S. al Coda";
            else
                  words = jp->xmlText();
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else if (jtp == Jump::Type::DS_AL_FINE) {
            if (jp->xmlText() == "")
                  words = "D.S. al Fine";
            else
                  words = jp->xmlText();
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
            qDebug("jump type=%d not implemented", int(jtp));
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
            if (m->xmlText() == "")
                  words = "To Coda";
            else
                  words = m->xmlText();
            if (m->label() == "")
                  sound = "tocoda=\"1\"";
            else
                  sound = "tocoda=\"" + m->label() + "\"";
            }
      else
            qDebug("marker type=%d not implemented", int(mtp));
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
      for (Element* e : m->el()) {
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
                              qDebug("repeatAtMeasureStart: marker %d not implemented", int(mtp));
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
      for (Element* e : m->el()) {
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
                              qDebug("repeatAtMeasureStop: marker %d not implemented", int(mtp));
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
// note: for a normal measure, mmRest1 is the measure itself,
// for a multi-meaure rest, it is the replacing measure

static void measureStyle(Xml& xml, Attributes& attr, Measure* m)
      {
      const Measure* mmR1 = m->mmRest1();
      if (m != mmR1 && m == mmR1->mmRestFirst()) {
            attr.doAttr(xml, true);
            xml.stag("measure-style");
            xml.tag("multiple-rest", mmR1->mmRestCount());
            xml.etag();
            }
      }

//---------------------------------------------------------
//  findFretDiagram
//---------------------------------------------------------

static const FretDiagram* findFretDiagram(int strack, int etrack, int track, Segment* seg)
      {
      if (seg->segmentType() == Segment::Type::ChordRest) {
            for (const Element* e : seg->annotations()) {

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

            for (const Element* e : seg->annotations()) {

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
                              case Element::Type::INSTRUMENT_CHANGE:
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
                  } // for
            if (fd)
                  // found fd but no harmony, cannot write (MusicXML would be invalid)
                  qDebug("annotations seg %p found fretboard diagram %p w/o harmony: cannot write",
                         seg, fd);
            }
      }

//---------------------------------------------------------
//  figuredBass
//---------------------------------------------------------

static void figuredBass(Xml& xml, int strack, int etrack, int track, const ChordRest* cr, FigBassMap& fbMap, int divisions)
      {
      Segment* seg = cr->segment();
      if (seg->segmentType() == Segment::Type::ChordRest) {
            for (const Element* e : seg->annotations()) {

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
                              int crEndTick = cr->tick() + cr->actualTicks();
                              int fbEndTick = fb->segment()->tick() + fb->ticks();
                              bool writeDuration = fb->ticks() < cr->actualTicks();
                              fb->writeMusicXML(xml, true, crEndTick, fbEndTick, writeDuration, divisions);

                              // Check for changing figures under a single note (each figure stored in a separate segment)
                              for (Segment* segNext = seg->next(); segNext && segNext->element(track) == NULL; segNext = segNext->next()) {
                                    for (Element* annot : segNext->annotations()) {
                                          if (annot->type() == Element::Type::FIGURED_BASS && annot->track() == track) {
                                                const FiguredBass* fb = static_cast<const FiguredBass*>(annot);
                                                fb->writeMusicXML(xml, true, 0, 0, true, divisions);
                                                }
                                          }
                                    }
                              // no extend can be pending
                              return;
                              }
                        }
                  }
            // check for extend pending
            if (fbMap.contains(strack)) {
                  const FiguredBass* fb = fbMap.value(strack);
                  int crEndTick = cr->tick() + cr->actualTicks();
                  int fbEndTick = fb->segment()->tick() + fb->ticks();
                  bool writeDuration = fb->ticks() < cr->actualTicks();
                  if (cr->tick() < fbEndTick) {
                        //qDebug("figuredbass() at tick %d extend only", cr->tick());
                        fb->writeMusicXML(xml, false, crEndTick, fbEndTick, writeDuration, divisions);
                        }
                  if (fbEndTick <= crEndTick) {
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

void ExportMusicXml::keysigTimesig(const Measure* m, const Part* p)
      {
      int strack = p->startTrack();
      int etrack = p->endTrack();
      //qDebug("keysigTimesig m %p strack %d etrack %d", m, strack, etrack);

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
                        //qDebug(" found keysig %p track %d", el, el->track());
                        int st = (t - strack) / VOICES;
                        if (!el->generated())
                              keysigs[st] = static_cast<KeySig*>(el);
                        }
                  }
            }

      //ClefType ct = rest->staff()->clef(rest->tick());

      // write the key signatues
      if (!keysigs.isEmpty()) {
            // determine if all staves have a keysig and all keysigs are identical
            // in that case a single <key> is written, without number=... attribute
            int nstaves = p->nstaves();
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
            //qDebug(" singleKey %d", singleKey);
            if (singleKey) {
                  // keysig applies to all staves
                  keysig(keysigs.value(0), p->staff(0)->clef(m->tick()), 0, keysigs.value(0)->visible());
                  }
            else {
                  // staff-specific keysigs
                  for (int st : keysigs.keys())
                        keysig(keysigs.value(st), p->staff(st)->clef(m->tick()), st + 1, keysigs.value(st)->visible());
                  }
            }
      else {
            // always write a keysig at tick = 0
            if (m->tick() == 0) {
                  //KeySigEvent kse;
                  //kse.setKey(Key::C);
                  KeySig* ks = new KeySig(_score);
                  ks->setKey(Key::C);
                  keysig(ks, p->staff(0)->clef(m->tick()));
                  delete ks;
                  }
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
      for (QString type : creators) {
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

      // specify supported elements
      xml.tagE("supports element=\"accidental\" type=\"yes\"");
      xml.tagE("supports element=\"beam\" type=\"yes\"");
      // set support for print new-page and new-system to match user preference
      // for MusicxmlExportBreaks::MANUAL support is "no" because "yes" breaks Finale NotePad import
      if (preferences.musicxmlExportLayout
          && preferences.musicxmlExportBreaks == MusicxmlExportBreaks::ALL) {
            xml.tagE("supports element=\"print\" attribute=\"new-page\" type=\"yes\" value=\"yes\"");
            xml.tagE("supports element=\"print\" attribute=\"new-system\" type=\"yes\" value=\"yes\"");
            }
      else {
            xml.tagE("supports element=\"print\" attribute=\"new-page\" type=\"no\"");
            xml.tagE("supports element=\"print\" attribute=\"new-system\" type=\"no\"");
            }
      xml.tagE("supports element=\"stem\" type=\"yes\"");

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
//  scoreInstrument
//---------------------------------------------------------

static void scoreInstrument(Xml& xml, const int partNr, const int instrNr, const QString& instrName)
      {
      xml.stag(QString("score-instrument %1").arg(instrId(partNr, instrNr)));
      xml.tag("instrument-name", instrName);
      xml.etag();
      }

//---------------------------------------------------------
//  midiInstrument
//---------------------------------------------------------

static void midiInstrument(Xml& xml, const int partNr, const int instrNr,
                           const Instrument* instr, const Score* score, const int unpitched = 0)
      {
      xml.stag(QString("midi-instrument %1").arg(instrId(partNr, instrNr)));
      int midiChannel = score->masterScore()->midiChannel(instr->channel(0)->channel);
      if (midiChannel >= 0 && midiChannel < 16)
            xml.tag("midi-channel", midiChannel + 1);
      int midiProgram = instr->channel(0)->program;
      if (midiProgram >= 0 && midiProgram < 128)
            xml.tag("midi-program", midiProgram + 1);
      if (unpitched > 0)
            xml.tag("midi-unpitched", unpitched);
      xml.tag("volume", (instr->channel(0)->volume / 127.0) * 100);  //percent
      xml.tag("pan", int(((instr->channel(0)->pan - 63.5) / 63.5) * 90)); //-90 hard left, +90 hard right
      xml.etag();
      }

//---------------------------------------------------------
//  initInstrMap
//---------------------------------------------------------

/**
 Initialize the Instrument* to number map for a Part
 Used to generate instrument numbers for a multi-instrument part
 */

static void initInstrMap(MxmlInstrumentMap& im, const InstrumentList* il, const Score* /*score*/)
      {
      im.clear();
      for (auto i = il->begin(); i != il->end(); ++i) {
            const Instrument* pinstr = i->second;
            if (!im.contains(pinstr))
                  im.insert(pinstr, im.size());
            }
      }

//---------------------------------------------------------
//  initReverseInstrMap
//---------------------------------------------------------

typedef QMap<int, const Instrument*> MxmlReverseInstrumentMap;

/**
 Initialize the number t Instrument* map for a Part
 Used to iterate in sequence over instrument numbers for a multi-instrument part
 */

static void initReverseInstrMap(MxmlReverseInstrumentMap& rim, const MxmlInstrumentMap& im)
      {
      rim.clear();
      for (const Instrument* i : im.keys()) {
            int instNr = im.value(i);
            rim.insert(instNr, i);
            }
      }

//---------------------------------------------------------
//  print
//---------------------------------------------------------

/**
 Handle the <print> element.
 When exporting layout and all breaks, a <print> with layout informations
 is generated for the measure types TopSystem, NewSystem and newPage.
 When exporting layout but only manual or no breaks, a <print> with
 layout informations is generated only for the measure type TopSystem,
 as it is assumed the system layout is broken by the importing application
 anyway and is thus useless.
 */

void ExportMusicXml::print(Measure* m, int idx, int staffCount, int staves)
      {
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
            QString newThing;       // new-[system|page]="yes" or empty
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
                  const PageFormat* pf = score()->pageFormat();
                  const double pageWidth  = getTenthsFromInches(pf->size().width());
                  const double lm = getTenthsFromInches(pf->oddLeftMargin());
                  const double rm = getTenthsFromInches(pf->oddRightMargin());
                  const double tm = getTenthsFromInches(pf->oddTopMargin());

                  // System Layout

                  // For a multi-meaure rest positioning is valid only
                  // in the replacing measure
                  // note: for a normal measure, mmRest1 is the measure itself,
                  // for a multi-meaure rest, it is the replacing measure
                  const Measure* mmR1 = m->mmRest1();
                  const System* system = mmR1->system();

                  // Put the system print suggestions only for the first part in a score...
                  if (idx == 0) {

                        // Find the right margin of the system.
                        double systemLM = getTenthsFromDots(mmR1->pagePos().x() - system->page()->pagePos().x()) - lm;
                        double systemRM = pageWidth - rm - (getTenthsFromDots(system->bbox().width()) + lm);

                        xml.stag("system-layout");
                        xml.stag("system-margins");
                        xml.tag("left-margin", QString("%1").arg(QString::number(systemLM,'f',2)));
                        xml.tag("right-margin", QString("%1").arg(QString::number(systemRM,'f',2)) );
                        xml.etag();

                        if (currentSystem == NewPage || currentSystem == TopSystem) {
                              const double topSysDist = getTenthsFromDots(mmR1->pagePos().y()) - tm;
                              xml.tag("top-system-distance", QString("%1").arg(QString::number(topSysDist,'f',2)) );
                              }
                        if (currentSystem == NewSystem) {
                              // see System::layout2() for the factor 2 * score()->spatium()
                              const double sysDist = getTenthsFromDots(mmR1->pagePos().y()
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
                        const double staffDist = 0.0;
//TODO-ws                              getTenthsFromDots(system->staff(staffCount + staffIdx - 1)->distanceDown());
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

      }

//---------------------------------------------------------
//  findAndExportClef
//---------------------------------------------------------

/**
 Make sure clefs at end of measure get exported at start of next measure.
 */

void ExportMusicXml::findAndExportClef(Measure* m, const int staves, const int strack, const int etrack)
      {
      Measure* prevMeasure = m->prevMeasure();
      Measure* mmR         = m->mmRest();       // the replacing measure in a multi-measure rest
      int tick             = m->tick();
      Segment* cs1;
      Segment* cs2         = m->findSegment(Segment::Type::Clef, tick);
      Segment* cs3;
      Segment* seg         = 0;

      if (prevMeasure)
            cs1 = prevMeasure->findSegment(Segment::Type::Clef,  tick);
      else
            cs1 = 0;

      if (mmR)
            cs3 = mmR->findSegment(Segment::Type::Clef,  tick);
      else
            cs3 = 0;

      if (cs1 && cs2) {
            // should only happen at begin of new system
            // when previous system ends with a non-generated clef
            seg = cs1;
            }
      else if (cs1)
            seg = cs1;
      else if (cs3) {
            // happens when the first measure is a multi-measure rest
            // containing a generated clef
            seg = cs3;
            }
      else
            seg = cs2;
      clefDebug("exportxml: clef segments cs1=%p cs2=%p cs3=%p seg=%p", cs1, cs2, cs3, seg);

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
                        clefDebug("exportxml: clef at start measure ti=%d ct=%d gen=%d", tick, int(cle->clefType()), cle->generated());
                        // output only clef changes, not generated clefs at line beginning
                        // exception: at tick=0, export clef anyway
                        if (tick == 0 || !cle->generated()) {
                              clefDebug("exportxml: clef exported");
                              clef(sstaff, cle);
                              }
                        else {
                              clefDebug("exportxml: clef not exported");
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//  findPitchesUsed
//---------------------------------------------------------

/**
 Find the set of pitches actually used in a part.
 */

typedef QSet<int> pitchSet;       // the set of pitches used

static void addChordPitchesToSet(const Chord* c, pitchSet& set)
      {
      for (const Note* note : c->notes()) {
            qDebug("chord %p note %p pitch %d", c, note, note->pitch() + 1);
            set.insert(note->pitch());
            }
      }

static void findPitchesUsed(const Part* part, pitchSet& set)
      {
      int strack = part->startTrack();
      int etrack = part->endTrack();

      // loop over all chords in the part
      for (const MeasureBase* mb = part->score()->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != Element::Type::MEASURE)
                  continue;
            const Measure* m = static_cast<const Measure*>(mb);
            for (int st = strack; st < etrack; ++st) {
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
                        const Element* el = seg->element(st);
                        if (!el)
                              continue;
                        if (el->type() == Element::Type::CHORD)
                              {
                              // add grace and non-grace note pitches to the result set
                              const Chord* c = static_cast<const Chord*>(el);
                              if (c) {
                                    for (const Chord* g : c->graceNotesBefore()) {
                                          addChordPitchesToSet(g, set);
                                          }
                                    addChordPitchesToSet(c, set);
                                    for (const Chord* g : c->graceNotesAfter()) {
                                          addChordPitchesToSet(g, set);
                                          }
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//  partList
//---------------------------------------------------------

/**
 Write the part list to \a xml.
 */

static void partList(Xml& xml, Score* score, const QList<Part*>& il, MxmlInstrumentMap& instrMap)
      {
      xml.stag("part-list");
      int staffCount = 0;                             // count sum of # staves in parts
      int partGroupEnd[MAX_PART_GROUPS];              // staff where part group ends (bracketSpan is in staves, not parts)
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
            initInstrMap(instrMap, part->instruments(), score);
            // by default export the parts long name as part-name
            if (part->longName() != "")
                  xml.tag("part-name", MScoreTextToMXML::toPlainText(part->longName()));
            else {
                  if (part->partName() != "") {
                        // use the track name if no part long name
                        // to prevent an empty track name on import
                        xml.tag("part-name print-object=\"no\"", MScoreTextToMXML::toPlainText(part->partName()));
                        }
                  else
                        // part-name is required
                        xml.tag("part-name", "");
                  }
            if (!part->shortName().isEmpty())
                  xml.tag("part-abbreviation", MScoreTextToMXML::toPlainText(part->shortName()));

            if (part->instrument()->useDrumset()) {
                  const Drumset* drumset = part->instrument()->drumset();
                  pitchSet pitches;
                  findPitchesUsed(part, pitches);
                  for (int i = 0; i < 128; ++i) {
                        DrumInstrument di = drumset->drum(i);
                        if (di.notehead != NoteHead::Group::HEAD_INVALID)
                              scoreInstrument(xml, idx + 1, i + 1, di.name);
                        else if (pitches.contains(i))
                              scoreInstrument(xml, idx + 1, i + 1, QString("Instrument %1").arg(i + 1));
                        }
                  int midiPort = part->midiPort() + 1;
                  if (midiPort >= 1 && midiPort <= 16)
                        xml.tag(QString("midi-device port=\"%1\"").arg(midiPort), "");

                  for (int i = 0; i < 128; ++i) {
                        DrumInstrument di = drumset->drum(i);
                        if (di.notehead != NoteHead::Group::HEAD_INVALID || pitches.contains(i))
                              midiInstrument(xml, idx + 1, i + 1, part->instrument(), score, i + 1);
                        }
                  }
            else {
                  MxmlReverseInstrumentMap rim;
                  initReverseInstrMap(rim, instrMap);
                  for (int instNr : rim.keys()) {
                        scoreInstrument(xml, idx + 1, instNr + 1, MScoreTextToMXML::toPlainText(rim.value(instNr)->trackName()));
                        }
                  for (auto ii = rim.constBegin(); ii != rim.constEnd(); ii++) {
                        int instNr = ii.key();
                        int midiPort = part->midiPort() + 1;
                        if (ii.value()->channel().size() > 0)
                              midiPort = score->masterScore()->midiMapping(ii.value()->channel(0)->channel)->port + 1;
                        if (midiPort >= 1 && midiPort <= 16)
                              xml.tag(QString("midi-device %1 port=\"%2\"").arg(instrId(idx+1, instNr + 1)).arg(midiPort), "");
                        else
                              xml.tag(QString("midi-device %1").arg(instrId(idx+1, instNr + 1)), "");
                        midiInstrument(xml, idx + 1, instNr + 1, rim.value(instNr), score);
                        }
                  }

            xml.etag();
            staffCount += part->nstaves();
            for (int i = MAX_PART_GROUPS - 1; i >= 0; i--) {
                  int end = partGroupEnd[i];
                  if (end >= 0) {
                        if (staffCount >= end) {
                              xml.tagE(QString("part-group type=\"stop\" number=\"%1\"").arg(i + 1));
                              partGroupEnd[i] = -1;
                              }
                        }
                  }
            }
      xml.etag();

      }

//---------------------------------------------------------
//  writeElement
//---------------------------------------------------------

/**
 Write \a el.
 */

void ExportMusicXml::writeElement(Element* el, const Measure* m, int sstaff, bool useDrumset)
      {
      switch (el->type()) {

            case Element::Type::CLEF:
                  {
                  // output only clef changes, not generated clefs
                  // at line beginning
                  // also ignore clefs at the start of a measure,
                  // these have already been output
                  // also ignore clefs at the end of a measure
                  // these will be output at the start of the next measure
                  const Clef* cle = static_cast<const Clef*>(el);
                  int ti = cle->segment()->tick();
                  clefDebug("exportxml: clef in measure ti=%d ct=%d gen=%d", ti, int(cle->clefType()), el->generated());
                  if (el->generated()) {
                        clefDebug("exportxml: generated clef not exported");
                        break;
                        }
                  if (!el->generated() && ti != m->tick() && ti != m->endTick())
                        clef(sstaff, cle);
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
                  Chord* c = static_cast<Chord*>(el);
                  const auto ll = &c->lyrics();
                  // ise grace after
                  if (c) {
                        for (Chord* g : c->graceNotesBefore()) {
                              chord(g, sstaff, ll, useDrumset);
                              }
                        chord(c, sstaff, ll, useDrumset);
                        for (Chord* g : c->graceNotesAfter()) {
                              chord(g, sstaff, ll, useDrumset);
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

      }

//---------------------------------------------------------
//  writeStaffDetails
//---------------------------------------------------------

/**
 Write the staff details for \a part to \a xml.
 */

static void writeStaffDetails(Xml& xml, const Part* part)
      {
      const Instrument* instrument = part->instrument();
      int staves = part->nstaves();

      // staff details
      // TODO: decide how to handle linked regular / TAB staff
      //       currently exported as a two staff part ...
      for (int i = 0; i < staves; i++) {
            Staff* st = part->staff(i);
            if (st->lines() != 5 || st->isTabStaff()) {
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
      }

//---------------------------------------------------------
//  writeInstrumentDetails
//---------------------------------------------------------

/**
 Write the instrument details for \a part to \a xml.
 */

static void writeInstrumentDetails(Xml& xml, const Part* part)
      {
      const Instrument* instrument = part->instrument();

      // instrument details
      if (instrument->transpose().chromatic) {        // TODO: tick
            xml.stag("transpose");
            xml.tag("diatonic",  instrument->transpose().diatonic % 7);
            xml.tag("chromatic", instrument->transpose().chromatic % 12);
            int octaveChange = instrument->transpose().chromatic / 12;
            if (octaveChange != 0)
                  xml.tag("octave-change", octaveChange);
            xml.etag();
            }
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
            score()->startCmd();
            score()->undo(new ChangeStyleVal(score(), StyleIdx::concertPitch, false));
            score()->doLayout();    // this is only allowed in a cmd context to not corrupt the undo/redo stack
            }

      calcDivisions();

      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            brackets[i] = 0;
            hairpins[i] = 0;
            ottavas[i] = 0;
            trills[i] = 0;
            }

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

      const QList<Part*>& il = _score->parts();
      partList(xml, _score, il, instrMap);

      int staffCount = 0;

      for (int idx = 0; idx < il.size(); ++idx) {
            Part* part = il.at(idx);
            tick = 0;
            xml.stag(QString("part id=\"P%1\"").arg(idx+1));

            int staves = part->nstaves();
            int strack = part->startTrack();
            int etrack = part->endTrack();

            trillStart.clear();
            trillStop.clear();
            initInstrMap(instrMap, part->instruments(), _score);

            int measureNo = 1;          // number of next regular measure
            int irregularMeasureNo = 1; // number of next irregular measure
            int pickupMeasureNo = 1;    // number of next pickup measure

            FigBassMap fbMap;           // pending figured bass extends

            for (MeasureBase* mb = _score->measures()->first(); mb; mb = mb->next()) {
                  if (mb->type() != Element::Type::MEASURE)
                        continue;
                  Measure* m = static_cast<Measure*>(mb);


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
                  const bool isFirstActualMeasure = (irregularMeasureNo + measureNo + pickupMeasureNo) == 4;

                  if (preferences.musicxmlExportLayout)
                        measureTag += QString(" width=\"%1\"").arg(QString::number(m->bbox().width() / DPMM / millimeters * tenths,'f',2));
#if 0 // MERGE
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

                              // For a multi-meaure rest positioning is valid only
                              // in the replacing measure
                              // note: for a normal measure, mmRest1 is the measure itself,
                              // for a multi-meaure rest, it is the replacing measure
                              const Measure* mmR1 = m->mmRest1();
                              const System* system = mmR1->system();

                              // Put the system print suggestions only for the first part in a score...
                              if (idx == 0) {

                                    // Find the right margin of the system.
                                    double systemLM = getTenthsFromDots(mmR1->pagePos().x() - system->page()->pagePos().x()) - lm;
                                    double systemRM = pageWidth - rm - (getTenthsFromDots(system->bbox().width()) + lm);

                                    xml.stag("system-layout");
                                    xml.stag("system-margins");
                                    xml.tag("left-margin", QString("%1").arg(QString::number(systemLM,'f',2)));
                                    xml.tag("right-margin", QString("%1").arg(QString::number(systemRM,'f',2)) );
                                    xml.etag();

                                    if (currentSystem == NewPage || currentSystem == TopSystem) {
                                          const double topSysDist = getTenthsFromDots(mmR1->pagePos().y()) - tm;
                                          xml.tag("top-system-distance", QString("%1").arg(QString::number(topSysDist,'f',2)) );
                                          }
                                    if (currentSystem == NewSystem) {
                                          // see System::layout2() for the factor 2 * score()->spatium()
                                          const double sysDist = getTenthsFromDots(mmR1->pagePos().y()
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
                                          // getTenthsFromDots(system->staff(staffCount + staffIdx - 1)->distanceDown());
                                          0.0;
                                    xml.tag("staff-distance", QString("%1").arg(QString::number(staffDist,'f',2)));
                                    xml.etag();
                                    }
                              }
                        }
#endif //MERGE

                  xml.stag(measureTag);

                  print(m, idx, staffCount, staves);

                  attr.start();

                  findTrills(m, strack, etrack, trillStart, trillStop);

                  // barline left must be the first element in a measure
                  barlineLeft(m);

                  // output attributes with the first actual measure (pickup or regular)
                  if (isFirstActualMeasure) {
                        attr.doAttr(xml, true);
                        xml.tag("divisions", MScore::division / div);
                        }

                  // output attributes at start of measure: key, time
                  keysigTimesig(m, part);

                  // output attributes with the first actual measure (pickup or regular) only
                  if (isFirstActualMeasure) {
                        if (staves > 1)
                              xml.tag("staves", staves);
                        if (instrMap.size() > 1)
                              xml.tag("instruments", instrMap.size());
                        }

                  // make sure clefs at end of measure get exported at start of next measure
                  findAndExportClef(m, staves, strack, etrack);

                  // output attributes with the first actual measure (pickup or regular) only
                  if (isFirstActualMeasure) {
                        writeStaffDetails(xml, part);
                        writeInstrumentDetails(xml, part);
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
                              if (tick != seg->tick()) {
                                    attr.doAttr(xml, false);
                                    moveToTick(seg->tick());
                                    }

                              // handle annotations and spanners (directions attached to this note or rest)
                              if (el->isChordRest()) {
                                    attr.doAttr(xml, false);
                                    annotations(this, xml, strack, etrack, st, sstaff, seg);
                                    // look for more harmony
                                    for (Segment* seg1 = seg->next(); seg1; seg1 = seg1->next()) {
                                          if (seg1->isChordRestType()) {
                                                Element* el1 = seg1->element(st);
                                                if (el1) // found a ChordRest, next harmony will be attach to this one
                                                      break;
                                                for (Element* annot : seg1->annotations()) {
                                                      if (annot->type() == Element::Type::HARMONY && annot->track() == st)
                                                            harmony(static_cast<Harmony*>(annot), 0, (seg1->tick() - seg->tick()) / div);
                                                      }
                                                }
                                          }
                                    figuredBass(xml, strack, etrack, st, static_cast<const ChordRest*>(el), fbMap, div);
                                    spannerStart(this, strack, etrack, st, sstaff, seg);
                                    }

#if 0  // MERGE
                              // write element el if necessary
                              writeElement(el, m, sstaff, part->instrument()->useDrumset());
#else
                              switch (el->type()) {

                                    case Element::Type::CLEF:
                                          {
                                          // output only clef changes, not generated clefs
                                          // at line beginning
                                          // also ignore clefs at the start of a measure,
                                          // these have already been output
                                          // also ignore clefs at the end of a measure
                                          // these will be output at the start of the next measure
                                          Clef* cle = static_cast<Clef*>(el);
                                          int ti = seg->tick();
                                          clefDebug("exportxml: clef in measure ti=%d ct=%d gen=%d", ti, int(cle->clefType()), el->generated());
                                          if (el->generated()) {
                                                clefDebug("exportxml: generated clef not exported");
                                                break;
                                                }
                                          if (!el->generated() && ti != m->tick() && ti != m->endTick())
                                                clef(sstaff, cle);
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
                                          Chord* c      = toChord(el);
                                          const auto ll = &c->lyrics();
                                          // ise grace after
                                          if (c) {
                                                for (Chord* g : c->graceNotesBefore()) {
                                                      chord(g, sstaff, ll, part->instrument()->useDrumset());
                                                      }
                                                chord(c, sstaff, ll, part->instrument()->useDrumset());
                                                for (Chord* g : c->graceNotesAfter()) {
                                                      chord(g, sstaff, ll, part->instrument()->useDrumset());
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
#endif // MERGE

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
            score()->endCmd(true);        // rollback
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
      return dots / DPMM / millimeters * tenths;
      }

//---------------------------------------------------------
//   harmony
//---------------------------------------------------------

void ExportMusicXml::harmony(Harmony const* const h, FretDiagram const* const fd, int offset)
      {
      // this code was probably in place to allow chord symbols shifted *right* to export with offset
      // since this was at once time the only way to get a chord to appear over beat 3 in an empty 4/4 measure
      // but the value was calculated incorrectly (should be divided by spatium) and would be better off using offset anyhow
      // since we now support placement of chord symbols over "empty" beats directly,
      // and wedon't generally export position info for other elements
      // it's just as well to not bother doing so here
      //double rx = h->userOff().x()*10;
      //QString relative;
      //if (rx > 0) {
      //      relative = QString(" relative-x=\"%1\"").arg(QString::number(rx,'f',2));
      //      }
      int rootTpc = h->rootTpc();
      if (rootTpc != Tpc::TPC_INVALID) {
            QString tagName = "harmony";
            bool frame = h->textStyle().hasFrame();
            tagName += QString(" print-frame=\"%1\"").arg(frame ? "yes" : "no"); // .append(relative));
            tagName += color2xml(h);
            xml.stag(tagName);
            xml.stag("root");
            xml.tag("root-step", tpc2stepName(rootTpc));
            int alter = int(tpc2alter(rootTpc));
            if (alter)
                  xml.tag("root-alter", alter);
            xml.etag();

            if (!h->xmlKind().isEmpty()) {
                  QString s = "kind";
                  QString kindText = h->xmlText();
                  if (h->xmlText() != "")
                        s += " text=\"" + kindText + "\"";
                  if (h->xmlSymbols() == "yes")
                        s += " use-symbols=\"yes\"";
                  if (h->xmlParens() == "yes")
                        s += " parentheses-degrees=\"yes\"";
                  xml.tag(s, h->xmlKind());
                  QStringList l = h->xmlDegrees();
                  if (!l.isEmpty()) {
                        for (QString tag : l) {
                              QString degreeText;
                              if (h->xmlKind().startsWith("suspended")
                                  && tag.startsWith("add") && tag[3].isDigit()
                                  && !kindText.isEmpty() && kindText[0].isDigit()) {
                                    // hack to correct text for suspended chords whose kind text has degree information baked in
                                    // (required by some other applications)
                                    int tagDegree = tag.mid(3).toInt();
                                    QString kindTextExtension;
                                    for (int i = 0; i < kindText.length() && kindText[i].isDigit(); ++i)
                                          kindTextExtension[i] = kindText[i];
                                    int kindExtension = kindTextExtension.toInt();
                                    if (tagDegree <= kindExtension && (tagDegree & 1) && (kindExtension & 1))
                                          degreeText = "\"\"";
                                    }
                              if (degreeText.isEmpty())
                                    xml.stag("degree");
                              else
                                    xml.stag("degree text=" + degreeText);
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
            if (offset > 0)
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
                  xml.stag(QString("harmony print-frame=\"yes\""));     // .append(relative));
            else
                  xml.stag(QString("harmony print-frame=\"no\""));      // .append(relative));
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

