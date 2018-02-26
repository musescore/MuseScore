//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "config.h"
#include "score.h"
#include "page.h"
#include "segment.h"
#include "clef.h"
#include "utils.h"
#include "system.h"
#include "measure.h"
#include "pitchspelling.h"
#include "chordrest.h"
#include "part.h"
#include "staff.h"
#include "note.h"
#include "chord.h"
#include "key.h"
#include "sig.h"
#include "tuplet.h"
#include "sym.h"

namespace Ms {

//---------------------------------------------------------
//   handleRect
//---------------------------------------------------------

QRectF handleRect(const QPointF& pos)
      {
      return QRectF(pos.x()-4, pos.y()-4, 8, 8);
      }

//---------------------------------------------------------
//   tick2measure
//---------------------------------------------------------

Measure* Score::tick2measure(int tick) const
      {
      if (tick == -1)
            return lastMeasure();

//      Q_ASSERT(firstMeasure());
      Measure* lm = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            if (tick < m->tick())
                  return lm;
            lm = m;
            }
      // check last measure
      if (lm && (tick >= lm->tick()) && (tick <= lm->endTick()))
            return lm;
      qDebug("tick2measure %d (max %d) not found", tick, lm ? lm->tick() : -1);
      return 0;
      }

//---------------------------------------------------------
//   tick2measureMM
//---------------------------------------------------------

Measure* Score::tick2measureMM(int tick) const
      {
      if (tick == -1)
            return lastMeasureMM();
      Measure* lm = 0;

      for (Measure* m = firstMeasureMM(); m; m = m->nextMeasureMM()) {
            if (tick < m->tick())
                  return lm;
            lm = m;
            }
      // check last measure
      if (lm && (tick >= lm->tick()) && (tick <= lm->endTick()))
            return lm;
      qDebug("tick2measureMM %d (max %d) not found", tick, lm ? lm->tick() : -1);
      return 0;
      }

//---------------------------------------------------------
//   tick2measureBase
//---------------------------------------------------------

MeasureBase* Score::tick2measureBase(int tick) const
      {
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            int st = mb->tick();
            int l  = mb->ticks();
            if (tick >= st && tick < (st+l))
                  return mb;
            }
//      qDebug("tick2measureBase %d not found", tick);
      return 0;
      }

//---------------------------------------------------------
//   tick2segment
//---------------------------------------------------------

Segment* Score::tick2segmentMM(int tick, bool first, SegmentType st) const
      {
      return tick2segment(tick,first,st,true);
      }

Segment* Score::tick2segmentMM(int tick) const
      {
      return tick2segment(tick, false, SegmentType::All, true);
      }
Segment* Score::tick2segmentMM(int tick, bool first) const
      {
      return tick2segment(tick, first, SegmentType::All, true);
      }

Segment* Score::tick2segment(int tick, bool first, SegmentType st, bool useMMrest ) const
      {
      Measure* m;
      if (useMMrest) {
            m = tick2measureMM(tick);
            // When mmRest force tick to the first segment of mmRest.
            if (m && m->isMMRest())
                  tick = m->tick();
            }
      else
            m = tick2measure(tick);

      if (m == 0) {
            qDebug("   no segment for tick %d", tick);
            return 0;
            }
      for (Segment* segment = m->first(st); segment;) {
            int t1 = segment->tick();
            Segment* nsegment = segment->next(st);
            int t2 = nsegment ? nsegment->tick() : INT_MAX;
            if ((tick == t1) && (first || (tick < t2)))
                  return segment;
            segment = nsegment;
            }
      return 0;
      }

Segment* Score::tick2segment(int tick) const
      {
      return tick2segment(tick, false, SegmentType::All, false);
      }

Segment* Score::tick2segment(int tick, bool first) const
      {
      return tick2segment(tick, first, SegmentType::All, false);
      }

//---------------------------------------------------------
//   tick2segmentEnd
//---------------------------------------------------------

/**
 Find a segment containing a note or rest in \a track ending at \a tick
 Return the segment or null
 */

Segment* Score::tick2segmentEnd(int track, int tick) const
      {
      Measure* m = tick2measure(tick);
      if (m == 0) {
            qDebug("tick2segment(): not found tick %d", tick);
            return 0;
            }
      // loop over all segments
      for (Segment* segment = m->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
            ChordRest* cr = toChordRest(segment->element(track));
            if (!cr)
                  continue;
            // TODO LVI: check if following is correct, see exceptions in
            // ExportMusicXmlchord() and ExportMusicXmlrest()
            int endTick = cr->tick() + cr->actualTicks();
            if (endTick < tick)
                  continue; // not found yet
            else if (endTick == tick) {
                  return segment; // found it
                  }
            else {
                  // endTick > tick (beyond the tick we are looking for)
                  return 0;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   tick2leftSegment
/// return the segment at this tick position if any or
/// the first segment *before* this tick position
//---------------------------------------------------------

Segment* Score::tick2leftSegment(int tick) const
      {
      Measure* m = tick2measure(tick);
      if (m == 0) {
            qDebug("tick2leftSegment(): not found tick %d", tick);
            return 0;
            }
      // loop over all segments
      Segment* ps = 0;
      for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            if (tick < s->tick())
                  return ps;
            else if (tick == s->tick())
                  return s;
            ps = s;
            }
      return ps;
      }

//---------------------------------------------------------
//   tick2rightSegment
/// return the segment at this tick position if any or
/// the first segment *after* this tick position
//---------------------------------------------------------

Segment* Score::tick2rightSegment(int tick) const
      {
      Measure* m = tick2measure(tick);
      if (m == 0) {
            qDebug("tick2nearestSegment(): not found tick %d", tick);
            return 0;
            }
      // loop over all segments
      for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            if (tick <= s->tick())
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   tick2beatType
//---------------------------------------------------------

BeatType Score::tick2beatType(int tick)
      {
      Measure* m = tick2measure(tick);
      const int msrTick = m->tick();
      TimeSigFrac timeSig = sigmap()->timesig(msrTick).nominal();

      int rtick = tick - msrTick;

      if (m->isAnacrusis()) // measure is incomplete (anacrusis)
            rtick += timeSig.ticksPerMeasure() - m->ticks();

      return timeSig.rtick2beatType(rtick);
      }

//---------------------------------------------------------
//   getStaff
//---------------------------------------------------------

int getStaff(System* system, const QPointF& p)
      {
      QPointF pp = p - system->page()->pos() - system->pos();
      for (int i = 0; i < system->page()->score()->nstaves(); ++i) {
            qreal sp = system->spatium();
            QRectF r = system->bboxStaff(i).adjusted(0.0, -sp, 0.0, sp);
            if (r.contains(pp))
                  return i;
            }
      return -1;
      }

//---------------------------------------------------------
//   nextSeg
//---------------------------------------------------------

int Score::nextSeg(int tick, int track)
      {
      Segment* seg = tick2segment(tick);
      while (seg) {
            seg = seg->next1(SegmentType::ChordRest);
            if (seg == 0)
                  break;
            if (seg->element(track))
                  break;
            }
      return seg ? seg->tick() : -1;
      }

//---------------------------------------------------------
//   nextSeg1
//---------------------------------------------------------

Segment* nextSeg1(Segment* seg, int& track)
      {
      int staffIdx   = track / VOICES;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      while ((seg = seg->next1(SegmentType::ChordRest))) {
            for (int t = startTrack; t < endTrack; ++t) {
                  if (seg->element(t)) {
                        track = t;
                        return seg;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   prevSeg1
//---------------------------------------------------------

Segment* prevSeg1(Segment* seg, int& track)
      {
      int staffIdx   = track / VOICES;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      while ((seg = seg->prev1(SegmentType::ChordRest))) {
            for (int t = startTrack; t < endTrack; ++t) {
                  if (seg->element(t)) {
                        track = t;
                        return seg;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
// next/prevChordNote
//
//    returns the top note of the next/previous chord. If a chord exists in the same track as note,
//    it is used. If not, the topmost existing chord is used.
//    May return nullptr if there is no next/prev note
//---------------------------------------------------------

Note* nextChordNote(Note* note)
      {
      int         track       = note->track();
      int         fromTrack   = (track / VOICES) * VOICES;
      int         toTrack     = fromTrack + VOICES;
      // TODO : limit to same instrument, not simply to same staff!
      Segment*    seg   = note->chord()->segment()->nextCR(track, true);
      while (seg) {
            Element*    targetElement = seg->elementAt(track);
            // if a chord exists in the same track, return its top note
            if (targetElement && targetElement->isChord())
                  return toChord(targetElement)->upNote();
            // if not, return topmost chord in track range
            for (int i = fromTrack ; i < toTrack; i++) {
                  targetElement = seg->elementAt(i);
                  if (targetElement && targetElement->isChord())
                        return toChord(targetElement)->upNote();
                  }
            seg = seg->nextCR(track, true);
            }
      return nullptr;
      }

Note* prevChordNote(Note* note)
      {
      int         track       = note->track();
      int         fromTrack   = (track / VOICES) * VOICES;
      int         toTrack     = fromTrack + VOICES;
      // TODO : limit to same instrument, not simply to same staff!
      Segment*    seg   = note->chord()->segment()->prev1();
      while (seg) {
            if (seg->segmentType() == SegmentType::ChordRest) {
                  Element*    targetElement = seg->elementAt(track);
                  // if a chord exists in the same track, return its top note
                  if (targetElement && targetElement->isChord())
                        return toChord(targetElement)->upNote();
                  // if not, return topmost chord in track range
                  for (int i = fromTrack ; i < toTrack; i++) {
                        targetElement = seg->elementAt(i);
                        if (targetElement && targetElement->isChord())
                              return toChord(targetElement)->upNote();
                        }
                  }
            seg = seg->prev1();
            }
      return nullptr;
      }

//---------------------------------------------------------
//   pitchKeyAdjust
//    change entered note to sounding pitch dependent
//    on key.
//    Example: if F is entered in G-major, a Fis is played
//    key -7 ... +7
//---------------------------------------------------------

int pitchKeyAdjust(int step, Key key)
      {
      static int ptab[15][7] = {
//             c  d  e  f  g   a  b
            { -1, 1, 3, 4, 6,  8, 10 },     // Bes
            { -1, 1, 3, 5, 6,  8, 10 },     // Ges
            {  0, 1, 3, 5, 6,  8, 10 },     // Des
            {  0, 1, 3, 5, 7,  8, 10 },     // As
            {  0, 2, 3, 5, 7,  8, 10 },     // Es
            {  0, 2, 3, 5, 7,  9, 10 },     // B
            {  0, 2, 4, 5, 7,  9, 10 },     // F
            {  0, 2, 4, 5, 7,  9, 11 },     // C
            {  0, 2, 4, 6, 7,  9, 11 },     // G
            {  1, 2, 4, 6, 7,  9, 11 },     // D
            {  1, 2, 4, 6, 8,  9, 11 },     // A
            {  1, 3, 4, 6, 8,  9, 11 },     // E
            {  1, 3, 4, 6, 8, 10, 11 },     // H
            {  1, 3, 5, 6, 8, 10, 11 },     // Fis
            {  1, 3, 5, 6, 8, 10, 12 },     // Cis
            };
      return ptab[int(key)+7][step];
      }

//---------------------------------------------------------
//   y2pitch
//---------------------------------------------------------

int y2pitch(qreal y, ClefType clef, qreal _spatium)
      {
      int l = lrint(y / _spatium * 2.0);
      return line2pitch(l, clef, Key::C);
      }

//---------------------------------------------------------
//   line2pitch
//    key  -7 ... +7
//---------------------------------------------------------

int line2pitch(int line, ClefType clef, Key key)
      {
      int l      = ClefInfo::pitchOffset(clef) - line;
      int octave = 0;
      while (l < 0) {
            l += 7;
            octave++;
            }
      octave += l / 7;
      l       = l % 7;

      int pitch = pitchKeyAdjust(l, key) + octave * 12;

      if (pitch > 127)
            pitch = 127;
      else if (pitch < 0)
            pitch = 0;
      return pitch;
      }

//---------------------------------------------------------
//   quantizeLen
//---------------------------------------------------------

int quantizeLen(int len, int raster)
      {
      if (raster == 0)
            return len;
      return int( ((float)len/raster) + 0.5 ) * raster; //round to the closest multiple of raster
      }

static const char* vall[] = {
      QT_TRANSLATE_NOOP("utils", "c"),
      QT_TRANSLATE_NOOP("utils", "c#"),
      QT_TRANSLATE_NOOP("utils", "d"),
      QT_TRANSLATE_NOOP("utils", "d#"),
      QT_TRANSLATE_NOOP("utils", "e"),
      QT_TRANSLATE_NOOP("utils", "f"),
      QT_TRANSLATE_NOOP("utils", "f#"),
      QT_TRANSLATE_NOOP("utils", "g"),
      QT_TRANSLATE_NOOP("utils", "g#"),
      QT_TRANSLATE_NOOP("utils", "a"),
      QT_TRANSLATE_NOOP("utils", "a#"),
      QT_TRANSLATE_NOOP("utils", "b")
      };
static const char* valu[] = {
      QT_TRANSLATE_NOOP("utils", "C"),
      QT_TRANSLATE_NOOP("utils", "C#"),
      QT_TRANSLATE_NOOP("utils", "D"),
      QT_TRANSLATE_NOOP("utils", "D#"),
      QT_TRANSLATE_NOOP("utils", "E"),
      QT_TRANSLATE_NOOP("utils", "F"),
      QT_TRANSLATE_NOOP("utils", "F#"),
      QT_TRANSLATE_NOOP("utils", "G"),
      QT_TRANSLATE_NOOP("utils", "G#"),
      QT_TRANSLATE_NOOP("utils", "A"),
      QT_TRANSLATE_NOOP("utils", "A#"),
      QT_TRANSLATE_NOOP("utils", "B")
      };

/*!
 * Returns the string representation of the given pitch.
 *
 * Returns the latin letter name, accidental, and octave numeral.
 * Uses upper case only for pitches 0-24.
 *
 * @param v
 *  The pitch number of the note.
 *
 * @return
 *  The string representation of the note.
 */
QString pitch2string(int v)
      {
      if (v < 0 || v > 127)
            return QString("----");
      int octave = (v / 12) - 1;
      QString o;
      o.sprintf("%d", octave);
      int i = v % 12;
      return qApp->translate("utils", octave < 0 ? valu[i] : vall[i]) + o;
      }

/*!
 * An array of all supported interval sorted by size.
 *
 * Because intervals can be spelled differently, this array
 * tracks all the different valid intervals. They are arranged
 * in diatonic then chromatic order.
 */
Interval intervalList[26] = {
      // diatonic - chromatic
      Interval(0, 0),         //  0 Perfect Unison
      Interval(0, 1),         //  1 Augmented Unison

      Interval(1, 0),         //  2 Diminished Second
      Interval(1, 1),         //  3 Minor Second
      Interval(1, 2),         //  4 Major Second
      Interval(1, 3),         //  5 Augmented Second

      Interval(2, 2),         //  6 Diminished Third
      Interval(2, 3),         //  7 Minor Third
      Interval(2, 4),         //  8 Major Third
      Interval(2, 5),         //  9 Augmented Third

      Interval(3, 4),         // 10 Diminished Fourth
      Interval(3, 5),         // 11 Perfect Fourth
      Interval(3, 6),         // 12 Augmented Fourth

      Interval(4, 6),         // 13 Diminished Fifth
      Interval(4, 7),         // 14 Perfect Fifth
      Interval(4, 8),         // 15 Augmented Fifth

      Interval(5, 7),         // 16 Diminished Sixth
      Interval(5, 8),         // 17 Minor Sixth
      Interval(5, 9),         // 18 Major Sixth
      Interval(5, 10),        // 19 Augmented Sixth

      Interval(6, 9),         // 20 Diminished Seventh
      Interval(6, 10),        // 21 Minor Seventh
      Interval(6, 11),        // 22 Major Seventh
      Interval(6, 12),        // 23 Augmented Seventh

      Interval(7, 11),        // 24 Diminshed Octave
      Interval(7, 12)         // 25 Perfect Octave
      };

/*!
 * Finds the most likely diatonic interval for a semitone distance.
 *
 * Uses the most common diatonic intervals.
 *
 * @param
 *  The number of semitones in the chromatic interval.
 *  Negative semitones will simply be made positive.
 *
 * @return
 *  The number of diatonic steps in the interval.
 */

int chromatic2diatonic(int semitones)
      {
      static int il[12] = {
            0,    // Perfect Unison
            3,    // Minor Second
            4,    // Major Second
            7,    // Minor Third
            8,    // Major Third
            11,   // Perfect Fourth
            12,   // Augmented Fourth
            14,   // Perfect Fifth
            17,   // Minor Sixth
            18,   // Major Sixth
            21,   // Minor Seventh
            22,   // Major Seventh
            // 25    Perfect Octave
            };
      bool down = semitones < 0;
      if (down)
            semitones = -semitones;
      int val = semitones % 12;
      int octave = semitones / 12;
      int intervalIndex = il[val];
      int steps = intervalList[intervalIndex].diatonic;
      steps = steps + octave * 7;
      return down ? -steps : steps;
      }

//---------------------------------------------------------
//   searchInterval
//---------------------------------------------------------

int searchInterval(int steps, int semitones)
      {
      unsigned n = sizeof(intervalList)/sizeof(*intervalList);
      for (unsigned i = 0; i < n; ++i) {
            if ((intervalList[i].diatonic == steps) && (intervalList[i].chromatic == semitones))
                  return i;
            }
      return -1;
      }

static int _majorVersion, _minorVersion, _updateVersion;

/*!
 * Returns the program version
 *
 * @return
 *  Version in the format: MMmmuu
 *  Where M=Major, m=minor, and u=update
 */

int version()
      {
      QRegExp re("(\\d+)\\.(\\d+)\\.(\\d+)");
      if (re.indexIn(VERSION) != -1) {
            QStringList sl = re.capturedTexts();
            if (sl.size() == 4) {
                  _majorVersion = sl[1].toInt();
                  _minorVersion = sl[2].toInt();
                  _updateVersion = sl[3].toInt();
                  return _majorVersion * 10000 + _minorVersion * 100 + _updateVersion;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   majorVersion
//---------------------------------------------------------

int majorVersion()
      {
      version();
      return _majorVersion;
      }

//---------------------------------------------------------
//   minorVersion
//---------------------------------------------------------

int minorVersion()
      {
      version();
      return _minorVersion;
      }

//---------------------------------------------------------
//   updateVersion
//---------------------------------------------------------

int updateVersion()
      {
      version();
      return _updateVersion;
      }

//---------------------------------------------------------
//   diatonicUpDown
//    used to find the second note of a trill, mordent etc.
//    key  -7 ... +7
//---------------------------------------------------------

int diatonicUpDown(Key k, int pitch, int steps)
      {
      static int ptab[15][7] = {
//             c  c#   d  d#    e   f  f#   g  g#  a  a#   b
            { -1,      1,       3,  4,      6,     8,      10 },     // Cb Ces
            { -1,      1,       3,  5,      6,     8,      10 },     // Gb Ges
            {  0,      1,       3,  5,      6,     8,      10 },     // Db Des
            {  0,      1,       3,  5,      7,     8,      10 },     // Ab As
            {  0,      2,       3,  5,      7,     8,      10 },     // Eb Es
            {  0,      2,       3,  5,      7,     9,      10 },     // Bb B
            {  0,      2,       4,  5,      7,     9,      10 },     // F  F

            {  0,      2,       4,  5,      7,     9,      11 },     // C  C

            {  0,      2,       4,  6,      7,     9,      11 },     // G  G
            {  1,      2,       4,  6,      7,     9,      11 },     // D  D
            {  1,      2,       4,  6,      8,     9,      11 },     // A  A
            {  1,      3,       4,  6,      8,     9,      11 },     // E  E
            {  1,      3,       4,  6,      8,    10,      11 },     // B  H
            {  1,      3,       5,  6,      8,    10,      11 },     // F# Fis
            {  1,      3,       5,  6,      8,    10,      12 },     // C# Cis
            };

      int key    = int(k) + 7;
      int step   = pitch % 12;
      int octave = pitch / 12;

      // loop through the diatonic steps of the key looking for the given note
      // or the gap where it would fit
      int i = 0;
      while (i < 7) {
            if (ptab[key][i] >= step)
                  break;
            ++i;
            }

      // neither step nor gap found
      // reset to beginning
      if (i == 7) {
            ++octave;
            i = 0;
            }
      // if given step not found (gap found instead), and we are stepping up
      // then we've already accounted for one step
      if (ptab[key][i] > step && steps > 0)
            --steps;

      // now start counting diatonic steps up or down
      if (steps > 0) {
            // count up
            while (steps--) {
                  ++i;
                  if (i == 7) {
                        // hit last step; reset to beginning
                        ++octave;
                        i = 0;
                        }
                  }
            }
      else if (steps < 0) {
            // count down
            while (steps++) {
                  --i;
                  if (i < 0) {
                        // hit first step; reset to end
                        --octave;
                        i = 6;
                        }
                  }
            }

      // convert step to pitch
      step = ptab[key][i];
      pitch = octave * 12 + step;
      if (pitch < 0)
            pitch = 0;
      if (pitch > 127)
            pitch = 128;
      return pitch;
      }

//---------------------------------------------------------
//   searchTieNote
//    search Note to tie to "note"
//---------------------------------------------------------

Note* searchTieNote(Note* note)
      {
      Note* note2  = 0;
      Chord* chord = note->chord();
      Segment* seg = chord->segment();
      Part* part   = chord->part();
      int strack   = part->staves()->front()->idx() * VOICES;
      int etrack   = strack + part->staves()->size() * VOICES;

      if (chord->isGraceBefore()) {
            chord = toChord(chord->parent());

            // try to tie to next grace note

            int index = chord->graceIndex();
            for (Chord* c : chord->graceNotes()) {
                  if (c->graceIndex() == index + 1) {
                        note2 = c->findNote(note->pitch());
                        if (note2) {
                              printf("found grace-grace tie\n");
                              return note2;
                              }
                        }
                  }

            // try to tie to note in parent chord
            note2 = chord->findNote(note->pitch());
            if (note2)
                  return note2;
            }
      else if (chord->isGraceAfter()) {
            // grace after
            // we will try to tie to note in next normal chord, below
            // meanwhile, set chord to parent chord so the endTick calculation will make sense
            chord = toChord(chord->parent());
            }
      else {
            // normal chord
            // try to tie to grace note after if present
            QVector<Chord*> gna = chord->graceNotesAfter();
            if (!gna.empty()) {
                  Chord* gc = gna[0];
                  note2 = gc->findNote(note->pitch());
                  if (note2)
                        return note2;
                  }
            }
      // at this point, chord is a regular chord, not a grace chord
      // and we are looking for a note in the *next* chord (grace or regular)

      // calculate end of current note duration
      // but err on the safe side in case there is roundoff in tick count
      int endTick = chord->tick() + chord->actualTicks() - 1;

      while ((seg = seg->next1(SegmentType::ChordRest))) {
            // skip ahead to end of current note duration as calculated above
            // but just in case, stop if we find element in current track
            if (seg->tick() < endTick  && !seg->element(chord->track()))
                  continue;
            for (int track = strack; track < etrack; ++track) {
                  Element* e = seg->element(track);
                  if (e == 0 || !e->isChord())
                        continue;
                  Chord* c = toChord(e);
                  // if there are grace notes before, try to tie to first one
                  QVector<Chord*> gnb = c->graceNotesBefore();
                  if (!gnb.empty()) {
                        Chord* gc = gnb[0];
                        Note* gn2 = gc->findNote(note->pitch());
                        if (gn2)
                              return gn2;
                        }
                  int staffIdx = c->staffIdx() + c->staffMove();
                  if (staffIdx != chord->staffIdx() + chord->staffMove())  // cannot happen?
                        continue;
                  for (Note* n : c->notes()) {
                        if (n->pitch() == note->pitch()) {
                              if (note2 == 0 || c->track() == chord->track())
                                    note2 = n;
                              }
                        }
                  }
            if (note2)
                  break;
            }
      return note2;
      }

//---------------------------------------------------------
//   searchTieNote114
//    search Note to tie to "note", tie to next note in
//    same voice
//---------------------------------------------------------

Note* searchTieNote114(Note* note)
      {
      Note* note2  = 0;
      Chord* chord = note->chord();
      Segment* seg = chord->segment();
      Part* part   = chord->part();
      int strack   = part->staves()->front()->idx() * VOICES;
      int etrack   = strack + part->staves()->size() * VOICES;

      while ((seg = seg->next1(SegmentType::ChordRest))) {
            for (int track = strack; track < etrack; ++track) {
                  Element* e = seg->element(track);
                  if (e == 0 || (!e->isChord()) || (e->track() != chord->track()))
                        continue;
                  Chord* c = toChord(e);
                  int staffIdx = c->staffIdx() + c->staffMove();
                  if (staffIdx != chord->staffIdx() + chord->staffMove())  // cannot happen?
                        continue;
                  for (Note* n : c->notes()) {
                        if (n->pitch() == note->pitch()) {
                              if (note2 == 0 || c->track() == chord->track())
                                    note2 = n;
                              }
                        }
                  }
            if (note2)
                  break;
            }
      return note2;
      }

//---------------------------------------------------------
//   absStep
///   Compute absolute step.
///   C D E F G A B ....
//---------------------------------------------------------

int absStep(int tpc, int pitch)
      {
      int line     = tpc2step(tpc) + (pitch / 12) * 7;
      int tpcPitch = tpc2pitch(tpc);

      if (tpcPitch < 0)
            line += 7;
      else
            line -= (tpcPitch / 12) * 7;
      return line;
      }

int absStep(int pitch)
      {
      // TODO - does this need to be key-aware?
      int tpc = pitch2tpc(pitch, Key::C, Prefer::NEAREST);
      return absStep(tpc, pitch);
      }

int absStep(int line, ClefType clef)
      {
      return ClefInfo::pitchOffset(clef) - line;
      }

//---------------------------------------------------------
//   relStep
///   Compute relative step from absolute step
///   which depends on actual clef. Step 0 starts on the
///   first (top) staff line.
//---------------------------------------------------------

int relStep(int line, ClefType clef)
      {
      return ClefInfo::pitchOffset(clef) - line;
      }

int relStep(int pitch, int tpc, ClefType clef)
      {
      return relStep(absStep(tpc, pitch), clef);
      }

//---------------------------------------------------------
//   pitch2step
//   returns one of { 0, 1, 2, 3, 4, 5, 6 }
//---------------------------------------------------------

int pitch2step(int pitch)
      {
      //                            C  C# D  D# E  F  F# G  G# A  A# B
      static const char tab[12] = { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6 };
      return tab[pitch%12];
      }

//---------------------------------------------------------
//   step2pitch
//   returns one of { 0, 2, 4, 5, 7, 9, 11 }
//---------------------------------------------------------

int step2pitch(int step)
      {
      static const char tab[7] = { 0, 2, 4, 5, 7, 9, 11 };
      return tab[step % 7];
      }

//---------------------------------------------------------
//   skipTuplet
//    return segment of rightmost chord/rest in a
//    (possible nested) tuplet
//---------------------------------------------------------

Segment* skipTuplet(Tuplet* tuplet)
      {
      DurationElement* nde = tuplet->elements().back();
      while (nde->isTuplet()) {
            tuplet = toTuplet(nde);
            nde = tuplet->elements().back();
            }
      return toChordRest(nde)->segment();
      }

//---------------------------------------------------------
//   toTimeSigString
//    replace ascii with bravura symbols
//---------------------------------------------------------

std::vector<SymId> toTimeSigString(const QString& s)
      {
      struct Dict {
            QChar code;
            SymId id;
            };
      static const std::vector<Dict> dict = {
            { 43,    SymId::timeSigPlusSmall        },  // '+'
            { 48,    SymId::timeSig0                },  // '0'
            { 49,    SymId::timeSig1                },  // '1'
            { 50,    SymId::timeSig2                },  // '2'
            { 51,    SymId::timeSig3                },  // '3'
            { 52,    SymId::timeSig4                },  // '4'
            { 53,    SymId::timeSig5                },  // '5'
            { 54,    SymId::timeSig6                },  // '6'
            { 55,    SymId::timeSig7                },  // '7'
            { 56,    SymId::timeSig8                },  // '8'
            { 57,    SymId::timeSig9                },  // '9'
            { 67,    SymId::timeSigCommon           },  // 'C'
            { 40,    SymId::timeSigParensLeftSmall  },  // '('
            { 41,    SymId::timeSigParensRightSmall },  // ')'
            { 162,   SymId::timeSigCutCommon        },  // '¢'
            { 59664, SymId::mensuralProlation1      },
            { 79,    SymId::mensuralProlation2      },  // 'O'
            { 59665, SymId::mensuralProlation2      },
            { 216,   SymId::mensuralProlation3      },  // 'Ø'
            { 59666, SymId::mensuralProlation3      },
            { 59667, SymId::mensuralProlation4      },
            { 59668, SymId::mensuralProlation5      },
            { 59670, SymId::mensuralProlation7      },
            { 59671, SymId::mensuralProlation8      },
            { 59673, SymId::mensuralProlation10     },
            { 59674, SymId::mensuralProlation11     },
            };

      std::vector<SymId> d;
      for (auto c : s) {
            for (const Dict& e : dict) {
                  if (c == e.code) {
                        d.push_back(e.id);
                        break;
                        }
                  }
            }
      return d;
      }

}

