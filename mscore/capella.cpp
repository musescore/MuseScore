//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: capella.cpp 5637 2012-05-16 14:23:09Z wschweer $
//
//  Copyright (C) 2009-2013 Werner Schweer and others
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

//
//    Capella 2000 import filter
//
#include <assert.h>
#include "libmscore/mscore.h"
#include "capella.h"
#include "libmscore/score.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/rest.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/utils.h"
#include "libmscore/lyrics.h"
#include "libmscore/timesig.h"
#include "libmscore/clef.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/keysig.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/box.h"
#include "libmscore/measure.h"
#include "libmscore/sig.h"
#include "libmscore/tuplet.h"
#include "libmscore/segment.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/dynamic.h"
#include "libmscore/barline.h"
#include "libmscore/volta.h"

extern QString rtf2html(const QString &);

namespace Ms {

//---------------------------------------------------------
//   errmsg
//---------------------------------------------------------

const char* Capella::errmsg[] = {
      "no error",
      "bad file signature, no Capella file or not from version 2000 (3.0) or later?",
      "unexpected end of file",
      "bad voice signature",
      "bad staff signature",
      "bad system signature",
      };

//---------------------------------------------------------
//   addDynamic
//---------------------------------------------------------

static void addDynamic(Score* score, Segment* s, int track, const char* name)
      {
      Dynamic* d = new Dynamic(score);
      d->setDynamicType(name);
      d->setTrack(track);
      s->add(d);
      }

#if 0 // TODO-S
//---------------------------------------------------------
//   levelofGraceSeg
//---------------------------------------------------------

static int levelofGraceSeg(Measure* m,int tick)
      {
      int nGraces = 1;
      Segment* seglist = m->findSegment(Segment::Type::Grace,tick);
      // count SegGrace segments
      for (Segment* ss = seglist; ss && ss->tick() == tick; ss = ss->prev()) {
            if ((ss->segmentType() == Segment::Type::Grace) && (ss->tick() == tick))
                  nGraces++;
            }
      return nGraces;
      }
#endif

//---------------------------------------------------------
//   SetCapGraceDuration
//---------------------------------------------------------

static void SetCapGraceDuration(Chord* chord,ChordObj* o)
      {
      NoteType nt = NoteType::APPOGGIATURA;
      ((Chord*)chord)->setNoteType(nt);
      if (o->t == TIMESTEP::D4) {
            ((Chord*)chord)->setNoteType(NoteType::GRACE4);
            chord->setDurationType(TDuration::DurationType::V_QUARTER);
            }
      else if (o->t == TIMESTEP::D_BREVE)
            ((Chord*)chord)->setDurationType(TDuration::DurationType::V_BREVE);
      else if (o->t == TIMESTEP::D1)
            ((Chord*)chord)->setDurationType(TDuration::DurationType::V_WHOLE);
      else if (o->t == TIMESTEP::D2)
            ((Chord*)chord)->setDurationType(TDuration::DurationType::V_HALF);
      else if (o->t == TIMESTEP::D16) {
            ((Chord*)chord)->setNoteType(NoteType::GRACE16);
            chord->setDurationType(TDuration::DurationType::V_16TH);
            }
      else if (o->t == TIMESTEP::D32) {
            ((Chord*)chord)->setNoteType(NoteType::GRACE32);
            chord->setDurationType(TDuration::DurationType::V_32ND);
            }
      else if (o->t == TIMESTEP::D64)
            ((Chord*)chord)->setDurationType(TDuration::DurationType::V_64TH);
      else if (o->t == TIMESTEP::D128)
            ((Chord*)chord)->setDurationType(TDuration::DurationType::V_128TH);
      else if (o->t == TIMESTEP::D256)
            ((Chord*)chord)->setDurationType(TDuration::DurationType::V_256TH);
      else
            ((Chord*)chord)->setDurationType(TDuration::DurationType::V_EIGHT);
      }

//---------------------------------------------------------
//   processBasicDrawObj
//---------------------------------------------------------

static void processBasicDrawObj(QList<BasicDrawObj*> objects, Segment* s, int track)
      {
      Score* score = s->score();
      foreach(BasicDrawObj* oo, objects) {
            switch (oo->type) {
                  case CapellaType::SIMPLE_TEXT:
                        {
                        SimpleTextObj* st = static_cast<SimpleTextObj*>(oo);
                        if (st->font().family() == "capella3") {
                              QString text(st->text());
                              if (text.size() == 1) {
                                    QChar c(text[0]);
                                    ushort code = c.unicode();
                                    switch (code) {
                                          case 'p':
                                                addDynamic(score, s, track, "p");
                                                break;
                                          case 'q':
                                                addDynamic(score, s, track, "pp");
                                                break;
                                          case 'r':
                                                addDynamic(score, s, track, "ppp");
                                                break;
                                          case 's':
                                                addDynamic(score, s, track, "sf");
                                                break;
                                          case 'f':
                                                addDynamic(score, s, track, "f");
                                                break;
                                          case 'g':
                                                addDynamic(score, s, track, "ff");
                                                break;
                                          case 'h':
                                                addDynamic(score, s, track, "fff");
                                                break;
                                          case 'i':
                                                addDynamic(score, s, track, "mp");
                                                break;
                                          case 'j':
                                                addDynamic(score, s, track, "mf");
                                                break;
                                          case 'l':         // ?
                                                break;
                                          case 't':   //    TRILL
                                                addDynamic(score, s, track, "xxx");
                                                break;
                                          case 'u':   // fermata up
                                          case 'v':   // 8va
                                          case 'w':   // turn
                                          case 'x':   // prall
                                          case 'y':   // segno
                                                break;
                                          case 'z':   // sfz
                                                addDynamic(score, s, track, "sfz");
                                                break;
                                          case '{':
                                                addDynamic(score, s, track, "fz");
                                                break;
                                          case '|':
                                                addDynamic(score, s, track, "fp");
                                                break;
                                          default:
                                                qDebug("====unsupported capella code %x(%c)", code, code);
                                                break;
                                          }
                                    break;
                                    }
                              }
                        Text* text = new Text(score);
                        QFont f(st->font());
                        text->textStyle().setItalic(f.italic());
                        // text->setUnderline(f.underline());
                        text->textStyle().setBold(f.bold());
                        text->textStyle().setSize(f.pointSizeF());

                        text->setText(st->text());
                        QPointF p(st->pos());
                        p = p / 32.0 * MScore::DPMM;
                        // text->setUserOff(st->pos());
                        text->setUserOff(p);
                        // qDebug("setText %s (%f %f)(%f %f) <%s>",
                        //            qPrintable(st->font().family()),
                        //            st->pos().x(), st->pos().y(), p.x(), p.y(), qPrintable(st->text()));
                        text->textStyle().setAlign(AlignmentFlags::LEFT | AlignmentFlags::BASELINE);
                        text->setTrack(track);
                        s->add(text);
                        }
                        break;
                  case CapellaType::TEXT:
                        qDebug("======================Text:");
                        break;
                  default:
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   findChordRests -- find begin and end ChordRest for BasicDrawObj o
//   return true on success (both begin and end found)
//---------------------------------------------------------

static bool findChordRests(BasicDrawObj const* const o, Score const* const score, const int track, const int tick,
                           ChordRest*& cr1, ChordRest*& cr2)
      {
      cr1 = 0;                         // ChordRest where BasicDrawObj o begins
      cr2 = 0;                         // ChordRest where BasicDrawObj o ends

      // find the ChordRests where o begins and ends
      int n = o->nNotes + 1;                                // # notes in BasicDrawObj (nNotes is # notes following the first note)
      for (Segment* seg = score->tick2segment(tick); seg; seg = seg->next1()) {
            if (seg->segmentType() != Segment::Type::ChordRest)
                  continue;
            ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
            if (cr) {
                  --n;                                              // found a ChordRest, count down
                  if (!cr1) cr1 = cr;                               // found first ChordRest
                  }
            else
                  qDebug("  %d empty seg", n);
            if (n == 0) {
                  cr2 = cr;                               // cr should be the second ChordRest
                  break;
                  }
            }
      qDebug("findChordRests o %p nNotes %d score %p track %d tick %d cr1 %p cr2 %p", o, o->nNotes, score, track, tick, cr1, cr2);

      if (!(cr1 && cr2)) {
            qDebug("first or second anchor for BasicDrawObj not found (tick %d type %d track %d first %p second %p)",
                   tick, o->type, track, cr1, cr2);
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   readCapVoice
//---------------------------------------------------------

static int readCapVoice(Score* score, CapVoice* cvoice, int staffIdx, int tick, bool capxMode)
      {
      int voice = cvoice->voiceNo;
      int track = staffIdx * VOICES + voice;

      qDebug("readCapVoice 1");
      //
      // pass I
      //
      int startTick = tick;

      Tuplet* tuplet = 0;
      int tupletCount = 0;
      int nTuplet = 0;
      int tupletTick = 0;

      qDebug("    read voice: tick %d track: %d)", tick, track);
      foreach(NoteObj* no, cvoice->objects) {
            switch (no->type()) {
                  case CapellaNoteObjectType::REST:
                        {
                        qDebug("     <Rest>");
                        Measure* m = score->getCreateMeasure(tick);
                        RestObj* o = static_cast<RestObj*>(no);
                        int ticks  = o->ticks();
                        if (o->invisible && ticks == 0) {  // get rid of placeholders
                              break;
                              }
                        TDuration d;
                        d.setVal(ticks);
                        if (o->count) {
                              if (tuplet == 0) {
                                    tupletCount = o->count;
                                    nTuplet     = 0;
                                    tupletTick  = tick;
                                    tuplet      = new Tuplet(score);
                                    Fraction f(3,2);
                                    if (tupletCount == 3)
                                          f = Fraction(3,2);
                                    else
                                          qDebug("Capella: unknown tuplet");
                                    tuplet->setRatio(f);
                                    tuplet->setBaseLen(d); // TODO check if necessary (the MusicXML importer doesn't do this)
                                    tuplet->setTrack(track);
                                    tuplet->setTick(tick);
                                    tuplet->setParent(m);
                                    int nn = ((tupletCount * ticks) * f.denominator()) / f.numerator();
                                    tuplet->setDuration(Fraction::fromTicks(nn)); // TODO check if necessary (the MusicXML importer doesn't do this)
                                    }
                              }

                        int ft     = m->ticks();
                        if (o->fullMeasures) {
                              ticks = ft * o->fullMeasures;
                              if (!o->invisible) {
                                    for (unsigned i = 0; i < o->fullMeasures; ++i) {
                                          Measure* m = score->getCreateMeasure(tick + i * ft);
                                          Segment* s = m->getSegment(Segment::Type::ChordRest, tick + i * ft);
                                          Rest* rest = new Rest(score);
                                          rest->setDurationType(TDuration(TDuration::DurationType::V_MEASURE));
                                          rest->setDuration(m->len());
                                          rest->setTrack(staffIdx * VOICES + voice);
                                          s->add(rest);
                                          }
                                    }
                              }
                        if (!o->invisible || voice == 0) {
                              Segment* s = m->getSegment(Segment::Type::ChordRest, tick);
                              Rest* rest = new Rest(score);
                              if (tuplet) {
                                    rest->setTuplet(tuplet);
                                    tuplet->add(rest);
                                    }
                              TDuration d;
                              if (o->fullMeasures) {
                                    d.setType(TDuration::DurationType::V_MEASURE);
                                    rest->setDuration(m->len());
                                    }
                              else {
                                    d.setVal(ticks);
                                    rest->setDuration(d.fraction());
                                    }
                              rest->setDurationType(d);
                              rest->setTrack(track);
                              rest->setVisible(!o->invisible);
                              s->add(rest);
                              processBasicDrawObj(o->objects, s, track);
                              }

                        if (tuplet) {
                              if (++nTuplet >= tupletCount) {
                                    tick = tupletTick + tuplet->actualTicks();
                                    tuplet = 0;
                                    }
                              else {
                                    tick += (ticks * tuplet->ratio().denominator()) / tuplet->ratio().numerator();
                                    }
                              }
                        else
                              tick += ticks;
                        }
                        break;
                  case CapellaNoteObjectType::CHORD:
                        {
                        qDebug("     <Chord>");
                        ChordObj* o = static_cast<ChordObj*>(no);
                        int ticks = o->ticks();
                        if (o->invisible && ticks == 0) {  // get rid of placeholders
                              break;
                              }
                        TDuration d;
                        d.setVal(ticks);
                        Measure* m = score->getCreateMeasure(tick);

//TODO-S                        int gl = levelofGraceSeg(m,tick);
                        bool isgracenote = (!(o->invisible) && (ticks==0));
//                        Segment* s = (isgracenote) ? m->getGraceSegment(tick, gl) : m->getSegment(Segment::Type::ChordRest, tick);
                        Segment* s = m->getSegment(Segment::Type::ChordRest, tick);
//                        if (isgracenote)
//                              s = m->getGraceSegment(tick,1);
                        if (o->count) {
                              if (tuplet == 0) {
                                    tupletCount = o->count;
                                    nTuplet     = 0;
                                    tupletTick  = tick;
                                    tuplet      = new Tuplet(score);
                                    Fraction f(3,2);
                                    if (tupletCount == 3)
                                          f = Fraction(3,2);
                                    else
                                          qDebug("Capella: unknown tuplet");
                                    tuplet->setRatio(f);
                                    tuplet->setBaseLen(d); // TODO check if necessary (the MusicXML importer doesn't do this)
                                    tuplet->setTrack(track);
                                    tuplet->setTick(tick);
                                    tuplet->setParent(m);
                                    int nn = ((tupletCount * ticks) * f.denominator()) / f.numerator();
                                    tuplet->setDuration(Fraction::fromTicks(nn)); // TODO check if necessary (the MusicXML importer doesn't do this)
                                    }
                              qDebug("Tuplet at %d: count: %d  tri: %d  prolonging: %d  ticks %d objects %d",
                                     tick, o->count, o->tripartite, o->isProlonging, ticks,
                                     o->objects.size());
                              }

                        Chord* chord = new Chord(score);
                        if (tuplet) {
                              chord->setTuplet(tuplet);
                              tuplet->add(chord);
                              }
                        if (isgracenote) { // grace notes
                              SetCapGraceDuration(chord,o);
                              chord->setDuration(chord->durationType().fraction());
                              }
                        else { // normal notes
                              chord->setDurationType(d);
                              chord->setDuration(d.fraction());
                              }
                        chord->setTrack(track);
                        switch (o->stemDir) {
                              case ChordObj::StemDir::DOWN:
                                    chord->setStemDirection(MScore::Direction::DOWN);
                                    break;
                              case ChordObj::StemDir::UP:
                                    chord->setStemDirection(MScore::Direction::UP);
                                    break;
                              case ChordObj::StemDir::NONE:
                                    chord->setNoStem(true);
                                    break;
                              case ChordObj::StemDir::AUTO:
                              default:
                                    break;
                              }
                        s->add(chord);
                        ClefType clef = score->staff(staffIdx)->clef(tick);
                        Key key  = score->staff(staffIdx)->key(tick);
                        int off;
                        switch (clef) {
                              case ClefType::G:      off = 0; break;
                              case ClefType::G1:     off = 7; break;
                              case ClefType::G2:     off = 14; break;
                              case ClefType::G3:     off = -7; break;
                              case ClefType::F:      off = -14; break;
                              case ClefType::F8:     off = -21; break;
                              case ClefType::F15:    off = -28; break;
                              case ClefType::F_B:    off = -14; break;
                              case ClefType::F_C:    off = -14; break;
                              case ClefType::C1:     off = -7; break;
                              case ClefType::C2:     off = -7; break;
                              case ClefType::C3:     off = -7; break;
                              case ClefType::C4:     off = -7; break;
                              case ClefType::C5:     off = -7; break;
                              case ClefType::G4:     off = 0; break;
                              case ClefType::F_8VA:  off = -7; break;
                              case ClefType::F_15MA: off = 0; break;
                              default:          off = 0; qDebug("clefType %hhd not implemented", clef);
                              }
                        // qDebug("clef %hhd off %d", clef, off);

                        static int keyOffsets[15] = {
                              /*   -7 -6 -5 -4 -3 -2 -1  0  1  2  3  4  5  6  7 */
                              /* */ 7, 4, 1, 5, 2, 6, 3, 0, 4, 1, 5, 2, 6, 3, 0
                              };
                        off += keyOffsets[int(key) + 7];

                        foreach(CNote n, o->notes) {
                              Note* note = new Note(score);
                              int pitch = 0;
                              // .cap import: pitch contains the diatonic note number relative to clef and key
                              // .capx  import: pitch the MIDI note number instead
                              if (capxMode) {
                                    pitch = n.pitch;
                                    }
                              else {
                                    int l = n.pitch + off + 7 * 6;
                                    int octave = 0;
                                    while (l < 0) {
                                          l += 7;
                                          octave--;
                                          }
                                    octave += l / 7;
                                    l       = l % 7;

                                    pitch = pitchKeyAdjust(l, key) + octave * 12;
                                    }
                              pitch    += n.alteration;

                              if (pitch > 127)
                                    pitch = 127;
                              else if (pitch < 0)
                                    pitch = 0;

                              note->setPitch(pitch);
                              // TODO: compute tpc from pitch & line
                              note->setTpcFromPitch();

                              chord->add(note);
                              if (o->rightTie) {
                                    Tie* tie = new Tie(score);
                                    tie->setStartNote(note);
                                    tie->setTrack(track);
                                    note->setTieFor(tie);
                                    }
                              }
                        foreach(Verse v, o->verse) {
                              Lyrics* l = new Lyrics(score);
                              l->setTrack(track);
                              l->setText(v.text);
                              if (v.hyphen)
                                    l->setSyllabic(Lyrics::Syllabic::BEGIN);
                              l->setNo(v.num);
                              chord->add(l);
                              }

                        processBasicDrawObj(o->objects, s, track);

                        if (tuplet) {
                              if (++nTuplet >= tupletCount) {
                                    tick = tupletTick + tuplet->actualTicks();
                                    tuplet = 0;
                                    }
                              else {
                                    tick += (ticks * tuplet->ratio().denominator()) / tuplet->ratio().numerator();
                                    }
                              }
                        else
                              tick += ticks;
                        }
                        break;
                  case CapellaNoteObjectType::CLEF:
                        {
                        qDebug("     <Clef>");
                        CapClef* o = static_cast<CapClef*>(no);
                        ClefType nclef = o->clef();
                        qDebug("%d:%d <Clef> %s line %d oct %d clef %hhd", tick, staffIdx, o->name(), o->line, o->oct, o->clef());
                        if (nclef == ClefType::INVALID)
                              break;
                        // staff(staffIdx)->setClef(tick, nclef);
                        Clef* clef = new Clef(score);
                        clef->setClefType(nclef);
                        clef->setTrack(staffIdx * VOICES);
                        Measure* m = score->getCreateMeasure(tick);
                        Segment* s = m->getSegment(Segment::Type::Clef, tick);
                        s->add(clef);
                        }
                        break;
                  case CapellaNoteObjectType::KEY:
                        {
                        qDebug("   <Key>");
                        CapKey* o = static_cast<CapKey*>(no);
                        Key key = score->staff(staffIdx)->key(tick);
                        if (key != Key(o->signature)) {
                              score->staff(staffIdx)->setKey(tick, Key(o->signature));
                              KeySig* ks = new KeySig(score);
                              ks->setTrack(staffIdx * VOICES);
                              Measure* m = score->getCreateMeasure(tick);
                              Segment* s = m->getSegment(Segment::Type::KeySig, tick);
                              s->add(ks);
                              ks->setKey(Key(o->signature));
                              }
                        }
                        break;
                  case CapellaNoteObjectType::METER:
                        {
                        CapMeter* o = static_cast<CapMeter*>(no);
                        qDebug("     <Meter> tick %d %d/%d", tick, o->numerator, 1 << o->log2Denom);
                        if (o->log2Denom > 7 || o->log2Denom < 0)
                              qFatal("illegal fraction");
                        SigEvent se = score->sigmap()->timesig(tick);
                        Fraction f(o->numerator, 1 << o->log2Denom);
                        SigEvent ne(f);
                        if (!(se == ne))
                              score->sigmap()->add(tick, ne);
                        TimeSig* ts = new TimeSig(score);
                        ts->setSig(f);
                        ts->setTrack(track);
                        Measure* m = score->getCreateMeasure(tick);
                        Segment* s = m->getSegment(Segment::Type::TimeSig, tick);
                        s->add(ts);
                        }
                        break;
                  case CapellaNoteObjectType::EXPL_BARLINE:
                  case CapellaNoteObjectType::IMPL_BARLINE:    // does not exist?
                        {
                        CapExplicitBarline* o = static_cast<CapExplicitBarline*>(no);
                        qDebug("     <Barline>");
                        Measure* pm = 0; // the previous measure (the one terminated by this barline)
                        if (tick > 0)
                              pm = score->getCreateMeasure(tick-1);
                        if (pm) {
                              int ticks = tick - pm->tick();
                              if (ticks > 0 && ticks != pm->ticks()) {
                                    // this is a measure with different actual duration
                                    Fraction f = Fraction::fromTicks(ticks);
                                    pm->setLen(f);
#if 0
                                    AL::SigEvent ne(f);
                                    ne.setNominal(m->timesig());
                                    score->sigmap()->add(m->tick(), ne);
                                    AL::SigEvent ne2(m->timesig());
                                    score->sigmap()->add(m->tick() + m->ticks(), ne2);
#endif
                                    }
                              }
                        // qDebug("pm %p", pm);

                        BarLineType st = o->type();
                        if (st == BarLineType::NORMAL)
                              break;

                        if (pm && (st == BarLineType::DOUBLE || st == BarLineType::END || st == BarLineType::BROKEN))
                              pm->setEndBarLineType(st, false, true);

                        if (st == BarLineType::START_REPEAT || st == BarLineType::END_START_REPEAT) {
                              Measure* nm = 0; // the next measure (the one started by this barline)
                              nm = score->getCreateMeasure(tick);
                              // qDebug("nm %p", nm);
                              if (nm)
                                    nm->setRepeatFlags(nm->repeatFlags() | Repeat::START);
                              }

                        if (st == BarLineType::END_REPEAT || st == BarLineType::END_START_REPEAT) {
                              if (pm)
                                    pm->setRepeatFlags(pm->repeatFlags() | Repeat::END);
                              }
                        }
                        break;
                  case CapellaNoteObjectType::PAGE_BKGR:
                        qDebug("     <PageBreak>");
                        break;
                  }
            }
      int endTick = tick;

      qDebug("readCapVoice 2");
      //
      // pass II
      //
      tick = startTick;
      foreach(NoteObj* no, cvoice->objects) {
            BasicDurationalObj* d = 0;
            if (no->type() == CapellaNoteObjectType::REST)
                  d = static_cast<BasicDurationalObj*>(static_cast<RestObj*>(no));
            else if (no->type() == CapellaNoteObjectType::CHORD)
                  d = static_cast<BasicDurationalObj*>(static_cast<ChordObj*>(no));
            if (!d)
                  continue;
            foreach(BasicDrawObj* o, d->objects) {
                  switch (o->type) {
                        case CapellaType::SIMPLE_TEXT:
                              // qDebug("simple text at %d", tick);
                              break;
                        case CapellaType::WAVY_LINE:
                              break;
                        case CapellaType::SLUR:
                              {
                              // SlurObj* so = static_cast<SlurObj*>(o);
                              // qDebug("slur tick %d  %d-%d-%d-%d   %d-%d", tick, so->nEnd, so->nMid,
                              //        so->nDotDist, so->nDotWidth, so->nRefNote, so->nNotes);
                              ChordRest* cr1 = 0; // ChordRest where slur begins
                              ChordRest* cr2 = 0; // ChordRest where slur ends
                              bool res = findChordRests(o, score, track, tick, cr1, cr2);

                              if (res) {
                                    if (cr1 == cr2)
                                          qDebug("first and second anchor for slur identical (tick %d track %d first %p second %p)",
                                                 tick, track, cr1, cr2);
                                    else {
                                          Slur* slur = new Slur(score);
                                          qDebug("tick %d track %d cr1 %p cr2 %p -> slur %p", tick, track, cr1, cr2, slur);
                                          slur->setTick(cr1->tick());
                                          slur->setTick2(cr2->tick());
                                          slur->setStartElement(cr1);
                                          slur->setEndElement(cr2);
                                          slur->setTrack(cr1->track());
                                          slur->setTrack2(cr2->track());
                                          score->addElement(slur);
                                          }
                                    }
                              }
                              break;
                        case CapellaType::TEXT: {

                              TextObj* to = static_cast<TextObj*>(o);
                              Text* s = new Text(score);
                              QString ss = ::rtf2html(QString(to->text));

                              // qDebug("string %f:%f w %d ratio %d <%s>",
                              //    to->relPos.x(), to->relPos.y(), to->width, to->yxRatio, qPrintable(ss));
                              s->setText(ss);
                              MeasureBase* measure = score->measures()->first();
                              if (measure->type() != Element::Type::VBOX) {
                                    MeasureBase* mb = new VBox(score);
                                    mb->setTick(0);
                                    score->addMeasure(mb, measure);
                                    }
                              s->setTextStyleType(TextStyleType::TITLE);
                              measure->add(s);
                              }
                              break;
                        case CapellaType::VOLTA:
                              {
                              VoltaObj* vo = static_cast<VoltaObj*>(o);
                              ChordRest* cr1 = 0; // ChordRest where volta begins
                              ChordRest* cr2 = 0; // ChordRest where volta ends
                              bool res = findChordRests(o, score, track, tick, cr1, cr2);

                              if (res) {
                                    Volta* volta = new Volta(score);
                                    volta->setTrack(track);
                                    volta->setTrack2(track);
                                    // TODO also support endings such as "1 - 3"
                                    volta->setText(QString("%1.").arg(vo->to));
                                    volta->endings().append(vo->to);
                                    if (vo->bRight)
                                          volta->setVoltaType(Volta::Type::CLOSED);
                                    else
                                          volta->setVoltaType(Volta::Type::OPEN);
                                    volta->setTick(cr1->measure()->tick());
                                    volta->setTick2(cr2->measure()->tick() + cr2->measure()->ticks());
                                    score->addElement(volta);
                                    }
                              }
                              break;
                        default:
                              break;
                        }
                  }
            // TODO: tick is wrong wg. tuplets
            int ticks = d->ticks();
            if (no->type() == CapellaNoteObjectType::REST) {
                  RestObj* o = static_cast<RestObj*>(no);
                  if (o->fullMeasures) {
                        Measure* m = score->getCreateMeasure(tick);
                        int ft     = m->ticks();
                        ticks = ft * o->fullMeasures;
                        }
                  }
            tick += ticks;
            }
      qDebug("   readCapVoice");
      return endTick;
      }

//---------------------------------------------------------
//   needPart -- determine if a staff needs its own part
//---------------------------------------------------------

// As Capella does not define parts (it only knows about staves,
// MIDI instruments numbers, brackets and braces), the following
// algorithm is used:
// - every staff is a separate part
// - unless:
//   - it is in a brace
//   - it is not the first staff in the brace
//   - it has the same MIDI instrument as the previous staff
// Common cases:
// - Keyboards: two or three staves with the same MIDI instrument and a brace
//   -> create one part
// - SATB: two or four staves with the same MIDI instrument and a bracket
//   -> create two or four parts

static bool needPart(const int prevInst, const int currInst, const int staffIdx, QList<CapBracket> const& bracketList)
      {
      // qDebug("needPart(prevInst %d, currInst %d, staffIdx %d)", prevInst, currInst, staffIdx);
      foreach(CapBracket cb, bracketList) {
            // qDebug("needPart bracket %d-%d curly %d", cb.from, cb.to, cb.curly);
            if (prevInst == currInst && cb.from < staffIdx && staffIdx <= cb.to && cb.curly) {
                  // qDebug("needPart found brace, continue part");
                  return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   convertCapella
//---------------------------------------------------------

void convertCapella(Score* score, Capella* cap, bool capxMode)
      {
      if (cap->systems.isEmpty())
            return;

      score->style()->set(StyleIdx::measureSpacing, 1.0);
      score->setSpatium(cap->normalLineDist * MScore::DPMM);
      score->style()->set(StyleIdx::smallStaffMag, cap->smallLineDist / cap->normalLineDist);
      score->style()->set(StyleIdx::minSystemDistance, Spatium(8));
      score->style()->set(StyleIdx::maxSystemDistance, Spatium(12));
      // score->style()->set(StyleIdx::hideEmptyStaves, true);

#if 1
      foreach(CapSystem* csys, cap->systems) {
            qDebug("System:");
            foreach(CapStaff* cstaff, csys->staves) {
                  CapStaffLayout* cl = cap->staffLayout(cstaff->iLayout);
                  qDebug("  Staff layout <%s><%s><%s><%s><%s> %d  barline %d-%d mode %d",
                         qPrintable(cl->descr), qPrintable(cl->name), qPrintable(cl->abbrev),
                         qPrintable(cl->intermediateName), qPrintable(cl->intermediateAbbrev),
                         cstaff->iLayout, cl->barlineFrom, cl->barlineTo, cl->barlineMode);
                  }
            }
#endif

      //
      // find out the maximum number of staves
      //
      int staves = 0;
      foreach(CapSystem* csys, cap->systems) {
            staves = qMax(staves, csys->staves.size());
            }
      //
      // check the assumption that every stave should be
      // associated with a CapStaffLayout
      //
      if (staves != cap->staffLayouts().size()) {
            qDebug("Capella: max number of staves != number of staff layouts (%d, %d)",
                   staves, cap->staffLayouts().size());
            staves = qMax(staves, cap->staffLayouts().size());
            }

      // set the initial time signature
      CapStaff* cs = cap->systems[0]->staves[0];
      if (cs->log2Denom <= 7)
            score->sigmap()->add(0, Fraction(cs->numerator, 1 << cs->log2Denom));

      // create parts and staves
      Staff* bstaff = 0;
      int span = 1;
      int midiPatch = -1; // the previous MIDI patch (instrument)
      Part* part = 0;
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            CapStaffLayout* cl = cap->staffLayout(staffIdx);
            // qDebug("Midi staff %d program %d", staffIdx, cl->sound);

            // create a new part if necessary
            if (needPart(midiPatch, cl->sound, staffIdx, cap->brackets)) {
                  part = new Part(score);
                  score->appendPart(part);
                  }
            midiPatch = cl->sound;

            Staff* s = new Staff(score, part, staffIdx);
            if (cl->bPercussion)
                  part->setMidiProgram(0, 128);
            else
                  part->setMidiProgram(cl->sound, 0);
            part->setPartName(cl->descr);
            part->setLongName(cl->name);
            part->setShortName(cl->abbrev);

            // ClefType clefType = CapClef::clefType(cl->form, cl->line, cl->oct);
            // s->setClef(0, clefType);
            s->setBarLineSpan(0);
            if (bstaff == 0) {
                  bstaff = s;
                  span = 0;
                  }
            ++span;
            if (cl->barlineMode == 1) {
                  bstaff->setBarLineSpan(span);
                  bstaff = 0;
                  }
            s->setSmall(cl->bSmall);
            part->insertStaff(s);
            score->staves().push_back(s);
            // _parts.push_back(part);
            }
      if (bstaff)
            bstaff->setBarLineSpan(span);

      foreach(CapBracket cb, cap->brackets) {
            qDebug("Bracket %d-%d curly %d", cb.from, cb.to, cb.curly);
            Staff* staff = score->staves().value(cb.from);
            if (staff == 0) {
                  qDebug("bad bracket 'from' value");
                  continue;
                  }
            staff->setBracket(0, cb.curly ? BracketType::BRACE : BracketType::NORMAL);
            staff->setBracketSpan(0, cb.to - cb.from + 1);
            }

      foreach(BasicDrawObj* o, cap->backgroundChord->objects) {
            switch (o->type) {
                  case CapellaType::SIMPLE_TEXT:
                        {
                        SimpleTextObj* to = static_cast<SimpleTextObj*>(o);
                        Text* s = new Text(score);
                        s->setTextStyleType(TextStyleType::TITLE);
                        QFont f(to->font());
                        s->textStyle().setItalic(f.italic());
                        // s->setUnderline(f.underline());
                        s->textStyle().setBold(f.bold());
                        s->textStyle().setSize(f.pointSizeF());

                        QString ss = to->text();
                        s->setText(ss);
                        MeasureBase* measure = new VBox(score);
                        measure->setTick(0);
                        score->addMeasure(measure, score->measures()->first());
                        measure->add(s);
                        // qDebug("page background object type %d (CapellaType::SIMPLE_TEXT) text %s", o->type, qPrintable(ss));
                        }
                        break;
                  default:
                        qDebug("page background object type %d", o->type);
                        break;
                  }
            }

      if (cap->topDist) {
            VBox* mb = 0;
            MeasureBaseList* mbl = score->measures();
            if (mbl->size() && mbl->first()->type() == Element::Type::VBOX)
                  mb = static_cast<VBox*>(mbl->first());
            else {
                  VBox* vb = new VBox(score);
                  vb->setTick(0);
                  score->addMeasure(vb, mb);
                  mb = vb;
                  }
            mb->setBoxHeight(Spatium(cap->topDist));
            }

      int systemTick = 0;
      foreach(CapSystem* csys, cap->systems) {
            qDebug("readCapSystem");
            /*
            if (csys->explLeftIndent > 0) {
                  HBox* mb = new HBox(score);
                  mb->setTick(systemTick);
                  mb->setBoxWidth(Spatium(csys->explLeftIndent));
                  score->addMeasure(mb);
                  }
            */
            int mtick = 0;
            foreach(CapStaff* cstaff, csys->staves) {
                  //
                  // assumption: layout index is mscore staffIdx
                  //    which means that there is a 1:1 relation between layout/staff
                  //

                  qDebug("  ReadCapStaff %d/%d", cstaff->numerator, 1 << cstaff->log2Denom);
                  int staffIdx = cstaff->iLayout;
                  int voice = 0;
                  foreach(CapVoice* cvoice, cstaff->voices) {
                        int tick = readCapVoice(score, cvoice, staffIdx, systemTick, capxMode);
                        ++voice;
                        if (tick > mtick)
                              mtick = tick;
                        }
                  }
            Measure* m = score->tick2measure(mtick-1);
            if (m && !m->lineBreak()) {
                  LayoutBreak* lb = new LayoutBreak(score);
                  lb->setLayoutBreakType(LayoutBreak::Type::LINE);
                  lb->setTrack(-1);       // this are system elements
                  m->add(lb);
                  }
            systemTick = mtick;
            }

      //
      // fill empty measures with rests
      //
      Segment::Type st = Segment::Type::ChordRest;
      for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            for (int staffIdx = 0; staffIdx < score->staves().size(); ++staffIdx) {
                  bool empty = true;
                  for (Segment* s = m->first(st); s; s = s->next(st)) {
                        if (s->element(staffIdx * VOICES)) {
                              empty = false;
                              break;
                              }
                        }
                  if (empty) {
                        Segment* s = m->getSegment(Segment::Type::ChordRest, m->tick());
                        Rest* rest = new Rest(score);
                        TDuration d(m->len());
                        if ((m->len() == m->timesig()) || !d.isValid())
                              rest->setDurationType(TDuration::DurationType::V_MEASURE);
                        else
                              rest->setDurationType(d.type());
                        rest->setDuration(m->len());
                        rest->setTrack(staffIdx * VOICES);
                        s->add(rest);
                        }
                  }
            }
      // score->connectSlurs();
      score->connectTies();
      score->fixTicks();
      score->updateNotes();
      score->setPlaylistDirty(true);
      score->setLayoutAll(true);
      score->addLayoutFlags(LayoutFlag::FIX_TICKS | LayoutFlag::FIX_PITCH_VELO);
      }

//---------------------------------------------------------
//   Capella
//---------------------------------------------------------

Capella::Capella()
      {
      author   = 0;
      keywords = 0;
      comment  = 0;
      }

Capella::~Capella()
      {
      delete[] author;
      delete[] keywords;
      delete[] comment;
      }

//---------------------------------------------------------
//   SlurObj::read
//---------------------------------------------------------

void SlurObj::read()
      {
      BasicDrawObj::read();
      for (int i = 0; i < 4; ++i) {
            bezierPoint[i].setX(cap->readInt());
            bezierPoint[i].setY(cap->readInt());
            }
      color     = cap->readColor();
      nEnd      = cap->readByte();
      nMid      = cap->readByte();
      nDotDist  = cap->readByte();
      nDotWidth = cap->readByte();
      // qDebug("SlurObj nEnd %d nMid %d nDotDist %d nDotWidth %d",
      //        nEnd, nMid, nDotDist, nDotWidth);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextObj::read()
      {
      BasicRectObj::read();
      unsigned size = cap->readUnsigned();
      char txt[size+1];
      cap->read(txt, size);
      txt[size] = 0;
      text = QString(txt);
      // qDebug("read textObj len %d <%s>", size, txt);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SimpleTextObj::read()
      {
      BasicDrawObj::read();
      relPos = cap->readPoint();
      align  = cap->readByte();
      _font  = cap->readFont();
      _text  = cap->readQString();
      // qDebug("read SimpletextObj(%f,%f) len %zd <%s>",
      //        relPos.x(), relPos.y(), _text.length(), qPrintable(_text));
      }

//---------------------------------------------------------
//   LineObj::read
//---------------------------------------------------------

void LineObj::read()
      {
      BasicDrawObj::read();
      pt1       = cap->readPoint();
      pt2       = cap->readPoint();
      color     = cap->readColor();
      lineWidth = cap->readByte();
      // qDebug("LineObj: %f:%f  %f:%f  width %d", pt1.x(), pt1.y(), pt2.x(), pt2.y(), lineWidth);
      }

//---------------------------------------------------------
//   BracketObj::read
//---------------------------------------------------------

void BracketObj::read()
      {
      LineObj::read();
      orientation = cap->readByte();
      number      = cap->readByte();
      }

//---------------------------------------------------------
//   GroupObj::read
//---------------------------------------------------------

void GroupObj::read()
      {
      BasicDrawObj::read();
      relPos = cap->readPoint();
      objects = cap->readDrawObjectArray();
      }

//---------------------------------------------------------
//   TransposableObj::read
//---------------------------------------------------------

void TransposableObj::read()
      {
      BasicDrawObj::read();
      relPos = cap->readPoint();
      b = cap->readByte();
      if (b != 12 && b != 21)
            qDebug("TransposableObj::read: warning: unknown drawObjectArray size of %d", b);
      variants = cap->readDrawObjectArray();
      if (variants.size() != b)
            qDebug("variants.size %d, expected %d", variants.size(), b);
      Q_ASSERT(variants.size() == b);
      /*int nRefNote =*/ cap->readInt();
      }

//---------------------------------------------------------
//   MetafileObj::read
//---------------------------------------------------------

void MetafileObj::read()
      {
      BasicRectObj::read();
      unsigned size = cap->readUnsigned();
      char enhMetaFileBits[size];
      cap->read(enhMetaFileBits, size);
      // qDebug("MetaFileObj::read %d bytes", size);
      }

//---------------------------------------------------------
//   RectEllipseObj::read
//---------------------------------------------------------

void RectEllipseObj::read()
      {
      LineObj::read();
      radius = cap->readInt();
      bFilled = cap->readByte();
      clrFill = cap->readColor();
      }

//---------------------------------------------------------
//   PolygonObj::read
//---------------------------------------------------------

void PolygonObj::read()
      {
      BasicDrawObj::read();

      unsigned nPoints = cap->readUnsigned();
      for (unsigned i = 0; i < nPoints; ++i)
            cap->readPoint();

      bFilled = cap->readByte();
      lineWidth = cap->readByte();
      clrFill = cap->readColor();
      clrLine = cap->readColor();
      }

//---------------------------------------------------------
//   WavyLineObj::read
//---------------------------------------------------------

void WavyLineObj::read()
      {
      LineObj::read();
      waveLen = cap->readByte();
      adapt = cap->readByte();
      }

//---------------------------------------------------------
//   NotelinesObj::read
//---------------------------------------------------------

void NotelinesObj::read()
      {
      BasicDrawObj::read();

      x0 = cap->readInt();
      x1 = cap->readInt();
      y  = cap->readInt();
      color = cap->readColor();

      unsigned char b = cap->readByte();
      switch (b) {
            case 1: break; // Einlinienzeile
            case 2: break; // Standard (5 Linien)
            default: {
                  Q_ASSERT(b == 0);
                  char lines[11];
                  cap->read(lines, 11);
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   VoltaObj::read
//---------------------------------------------------------

void VoltaObj::read()
      {
      BasicDrawObj::read();

      x0 = cap->readInt();
      x1 = cap->readInt();
      y  = cap->readInt();
      color = cap->readColor();

      unsigned char flags = cap->readByte();
      bLeft      = (flags & 1) != 0; // links abgeknickt
      bRight     = (flags & 2) != 0; // rechts abgeknickt
      bDotted    = (flags & 4) != 0;
      allNumbers = (flags & 8) != 0;

      unsigned char numbers = cap->readByte();
      from = numbers & 0x0F;
      to = (numbers >> 4) & 0x0F;
      qDebug("VoltaObj::read x0 %d x1 %d y %d bLeft %d bRight %d bDotted %d allNumbers %d from %d to %d",
             x0, x1, y, bLeft, bRight, bDotted, allNumbers, from, to);
      }

//---------------------------------------------------------
//   GuitarObj::read
//---------------------------------------------------------

void GuitarObj::read()
      {
      BasicDrawObj::read();
      relPos  = cap->readPoint();
      color   = cap->readColor();
      flags   = cap->readWord();
      strings = cap->readDWord();   // 8 Saiten in 8 Halbbytes
      }

//---------------------------------------------------------
//   TrillObj::read
//---------------------------------------------------------

void TrillObj::read()
      {
      BasicDrawObj::read();
      x0 = cap->readInt();
      x1 = cap->readInt();
      y  = cap->readInt();
      color = cap->readColor();
      trillSign = cap->readByte();
      }

//---------------------------------------------------------
//   readDrawObjectArray
//---------------------------------------------------------

QList<BasicDrawObj*> Capella::readDrawObjectArray()
      {
      QList<BasicDrawObj*> ol;
      int n = readUnsigned();       // draw obj array

      // qDebug("readDrawObjectArray %d elements", n);
      for (int i = 0; i < n; ++i) {
            CapellaType type = CapellaType(readByte());

            // qDebug("   readDrawObject %d of %d, type %d", i, n, type);
            switch (type) {
                  case  CapellaType::GROUP: {
                        GroupObj* o = new GroupObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  CapellaType::TRANSPOSABLE: {
                        TransposableObj* o = new TransposableObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  CapellaType::METAFILE: {
                        MetafileObj* o = new MetafileObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  CapellaType::SIMPLE_TEXT: {
                        SimpleTextObj* o = new SimpleTextObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  CapellaType::TEXT: {
                        TextObj* o = new TextObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  CapellaType::RECT_ELLIPSE: {
                        RectEllipseObj* o = new RectEllipseObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case CapellaType::LINE: {
                        LineObj* o = new LineObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  CapellaType::POLYGON: {
                        PolygonObj* o = new PolygonObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  CapellaType::WAVY_LINE: {
                        WavyLineObj* o = new WavyLineObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case CapellaType::SLUR: {
                        SlurObj* o = new SlurObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  CapellaType::NOTE_LINES: {
                        NotelinesObj* o = new NotelinesObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case CapellaType::WEDGE: {
                        WedgeObj* o = new WedgeObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  CapellaType::VOLTA: {
                        VoltaObj* o = new VoltaObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case CapellaType::BRACKET: {
                        BracketObj* o = new BracketObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  CapellaType::GUITAR: {
                        GuitarObj* o = new GuitarObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  case  CapellaType::TRILL: {
                        TrillObj* o = new TrillObj(this);
                        o->read();
                        ol.append(o);
                        }
                        break;
                  default:
                        qFatal("readDrawObjectArray unsupported type %d", type);
                        break;
                  }
            }
      return ol;
      }

//---------------------------------------------------------
//   BasicDrawObj
//---------------------------------------------------------

void BasicDrawObj::read()
      {
      modeX       = cap->readByte();      // anchor mode
      modeY       = cap->readByte();
      distY       = cap->readByte();
      flags       = cap->readByte();
      nRefNote    = cap->readInt();
      short range = cap->readWord();
      nNotes      = range & 0x0fff;
      background  = range & 0x1000;
      pageRange   = (range >> 13) & 0x7;
      qDebug("BasicDrawObj::read modeX %d modeY %d distY %d flags %d nRefNote %d nNotes %d background %d pageRange %d",
             modeX, modeY, distY, flags, nRefNote, nNotes, background, pageRange);
      }

//---------------------------------------------------------
//   BasicRectObj
//---------------------------------------------------------

void BasicRectObj::read()
      {
      BasicDrawObj::read();
      relPos  = cap->readPoint();
      width   = cap->readInt();
      yxRatio = cap->readInt();
      height  = (width * yxRatio) / 0x10000;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BasicDurationalObj::read()
      {
      unsigned char b = cap->readByte();
      nDots      = b & 0x03;
      noDuration = b & 0x04;
      postGrace  = b & 0x08;
      bSmall     = b & 0x10;
      invisible  = b & 0x20;
      notBlack   = b & 0x40;
      Q_ASSERT(!(b & 0x80));

      color = notBlack ? cap->readColor() : Qt::black;

      unsigned char c = cap->readByte();
      t = TIMESTEP(c & 0x0f);
      horizontalShift = (c & 0x10) ? cap->readInt() : 0;
      count = 0;
      tripartite = 0;
      isProlonging = 0;
      if (c & 0x20) {
            unsigned char tuplet = cap->readByte();
            count        = tuplet & 0x0f;
            tripartite   = (tuplet & 0x10) != 0;
            isProlonging = (tuplet & 0x20) != 0;
            if (tuplet & 0xc0)
                  qDebug("bad tuplet value 0x%02x", tuplet);
            }
      if (c & 0x40) {
            objects = cap->readDrawObjectArray();
            }
      Q_ASSERT(!(c & 0x80));
      qDebug("DurationObj ndots %d nodur %d postgr %d bsm %d inv %d notbl %d t %d hsh %d cnt %d trp %d ispro %d",
             nDots, noDuration, postGrace, bSmall, invisible, notBlack, t, horizontalShift, count, tripartite, isProlonging
             );
      }

//---------------------------------------------------------
//   RestObj
//---------------------------------------------------------

RestObj::RestObj(Capella* c)
      : BasicDurationalObj(c), NoteObj(CapellaNoteObjectType::REST)
      {
      cap          = c;
      fullMeasures = 0;
      vertShift    = 0;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void RestObj::read()
      {
      BasicDurationalObj::read();
      unsigned char b        = cap->readByte();
      bool bMultiMeasures    = b & 1;
      bVerticalCentered      = b & 2;
      bool bAddVerticalShift = b & 4;
      if (b & 0xf8)
            qFatal("RestObj: res. bits 0x%02x", b);
      fullMeasures = bMultiMeasures ? cap->readUnsigned() : 0;
      vertShift    = bAddVerticalShift ? cap->readInt() : 0;
      }

//---------------------------------------------------------
//   ChordObj
//---------------------------------------------------------

ChordObj::ChordObj(Capella* c)
      : BasicDurationalObj(c), NoteObj(CapellaNoteObjectType::CHORD)
      {
      cap      = c;
      beamMode = BeamMode::AUTO;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordObj::read()
      {
      stemDir      = StemDir::AUTO;
      dStemLength  = 0;
      nTremoloBars = 0;
      articulation = 0;
      leftTie      = false;
      rightTie     = false;
      beamShift    = 0;
      beamSlope    = 0;

      BasicDurationalObj::read();

      unsigned char flags = cap->readByte();
      beamMode      = (flags & 0x01) ? BeamMode(cap->readByte()) : BeamMode::AUTO;
      notationStave = (flags & 0x02) ? cap->readChar() : 0;
      Q_ASSERT(notationStave >= -1 && notationStave <= 1);

      if (flags & 0x04) {
            stemDir     = StemDir(cap->readChar());
            dStemLength = cap->readChar();
            }
      if (flags & 0x08) {
            nTremoloBars = cap->readByte();
            articulation = cap->readByte();
            }
      if (flags & 0x10) {
            unsigned char b = cap->readByte();
            leftTie  = b & 1;
            rightTie = b & 2;
            }
      if (flags & 0x20) {
            beamShift = cap->readChar();
            beamSlope = cap->readChar();
            }
      if (flags & 0x40) {
            unsigned nVerses = cap->readUnsigned();
            for (unsigned int i = 0; i < nVerses; ++i) {
                  bool bVerse = cap->readByte();
                  if (bVerse) {
                        Verse v;
                        unsigned char b = cap->readByte();
                        v.leftAlign = b & 1;
                        v.extender  = b & 2;
                        v.hyphen    = b & 4;
                        v.num       = i;
                        if (b & 8)
                              v.verseNumber = cap->readQString();
                        if (b & 16)
                              v.text = cap->readQString();
                        verse.append(v);
                        }
                  }
            }
      unsigned nNotes = cap->readUnsigned();
      for (unsigned int i = 0; i < nNotes; ++i) {
            CNote n;
            n.explAlteration = 0;
            char c           = cap->readChar();
            bool bit7        = c & 0x80;
            bool bit6        = c & 0x40;
            n.pitch          = c;
            if (bit7 != bit6) {
                  n.explAlteration = 2;
                  n.pitch ^= 0x80;
                  }
            unsigned char b = cap->readByte();
            n.headType      = b & 7;
            n.alteration    = ((b >> 3) & 7) - 2;  // -2 -- +2
            if (b & 0x40)
                  n.explAlteration = 1;
            n.silent = b & 0x80;
            qDebug("ChordObj::read() note pitch %d explAlt %d head %d alt %d silent %d",
                   n.pitch, n.explAlteration, n.headType, n.alteration, n.silent);
            notes.append(n);
            }
      }

//---------------------------------------------------------
//    read
//    return false on error
//---------------------------------------------------------

void Capella::read(void* p, qint64 len)
      {
      if (len == 0)
            return;
      qint64 rv = f->read((char*)p, len);
      if (rv != len)
            throw Capella::Error::CAP_EOF;
      curPos += len;
      }

//---------------------------------------------------------
//   readByte
//---------------------------------------------------------

unsigned char Capella::readByte()
      {
      unsigned char c;
      read(&c, 1);
      return c;
      }

//---------------------------------------------------------
//   readChar
//---------------------------------------------------------

char Capella::readChar()
      {
      char c;
      read(&c, 1);
      return c;
      }

//---------------------------------------------------------
//   readWord
//---------------------------------------------------------

short Capella::readWord()
      {
      short c;
      read(&c, 2);
      return c;
      }

//---------------------------------------------------------
//   readDWord
//---------------------------------------------------------

int Capella::readDWord()
      {
      int c;
      read(&c, 4);
      return c;
      }

//---------------------------------------------------------
//   readLong
//---------------------------------------------------------

int Capella::readLong()
      {
      int c;
      read(&c, 4);
      return c;
      }

//---------------------------------------------------------
//   readUnsigned
//---------------------------------------------------------

unsigned Capella::readUnsigned()
      {
      unsigned char c;
      read(&c, 1);
      if (c == 254) {
            unsigned short s;
            read(&s, 2);
            return s;
            }
      else if (c == 255) {
            unsigned s;
            read(&s, 4);
            return s;
            }
      else
            return c;
      }

//---------------------------------------------------------
//   readInt
//---------------------------------------------------------

int Capella::readInt()
      {
      char c;
      read(&c, 1);
      if (c == -128) {
            short s;
            read(&s, 2);
            return s;
            }
      else if (c == 127) {
            int s;
            read(&s, 4);
            return s;
            }
      else
            return c;
      }

//---------------------------------------------------------
//   readString -- read Capella string into newly allocated char buffer
//   note that no carriage return / newline interpretation is done
//---------------------------------------------------------

char* Capella::readString()
      {
      unsigned len = readUnsigned();
      char* buffer = new char[len + 1];
      read(buffer, len);
      buffer[len] = 0;
      return buffer;
      }

//---------------------------------------------------------
//   readQString -- read Capella string into QString
//   strings in Capella files may contain \r\n, must remove the \r
//---------------------------------------------------------

QString Capella::readQString()
      {
      char* buffer = readString();   // read Capella string
      QString res(buffer);           // and copy into QString
      res = res.remove(QChar('\r')); // remove the \r
      delete [] buffer;              // delete memory allocated by readString
      return res;
      }

//---------------------------------------------------------
//   readColor
//---------------------------------------------------------

QColor Capella::readColor()
      {
      static const int colors[] = {
            0x000000, // schwarz
            0x000080, // dunkelrot
            0x008000, // dunkelgrün
            0x008080, // ocker
            0x800000, // dunkelblau
            0x800080, // purpurrot
            0x808000, // blaugün
            0x808080, // grau
            0xC0C0C0, // hellgrau
            0x0000FF, // rot
            0x00FF00, // grün
            0x00FFFF, // gelb
            0xFF0000, // blau
            0xFF00FF, // lila
            0xFFFF00, // aquamarin
            0xFFFFFF  // weiß
            };

      QColor c;
      unsigned char b = readByte();
      if (b >= 16) {
            Q_ASSERT(b == 255);
            int r = readByte();
            int g = readByte();
            int b = readByte();
            c = QColor(r, g, b);
            }
      else {
            c = QColor(colors[b]);
            }
      return c;
      }

//---------------------------------------------------------
//   readFont
//---------------------------------------------------------

QFont Capella::readFont()
      {
      int index = readUnsigned();
      if (index == 0) {
            int lfHeight           = readLong();
            /*int lfWidth            =*/ readLong();
            /*int lfEscapement       =*/ readLong();
            /*int lfOrientation      =*/ readLong();
            int lfWeight           = readLong();
            uchar lfItalic         = readByte();
            uchar lfUnderline      = readByte();
            uchar lfStrikeOut      = readByte();
            /*uchar lfCharSet        =*/ readByte();
            /*uchar lfOutPrecision   =*/ readByte();
            /*uchar lfClipPrecision  =*/ readByte();
            /*uchar lfQuality        =*/ readByte();
            /*uchar lfPitchAndFamily =*/ readByte();
            /*QColor color           =*/ readColor();
            char* face             = readString();

            qDebug("Font <%s> size %d, weight %d", face, lfHeight, lfWeight);
            QFont font(face);
            font.setPointSizeF(lfHeight / 1000.0);
            font.setItalic(lfItalic);
            font.setStrikeOut(lfStrikeOut);
            font.setUnderline(lfUnderline);

            switch (lfWeight) {
                  case 700:  font.setWeight(QFont::Bold); break;
                  case 400:  font.setWeight(QFont::Normal); break;
                  case 0:    font.setWeight(QFont::Light); break;
                  }
            fonts.append(font);
            return font;
            }
      index -= 1;
      if (index >= fonts.size()) {
            qDebug("illegal font index %d (max %d)", index, fonts.size()-1);
            }
      return fonts[index];
      }

//---------------------------------------------------------
//   readStaveLayout
//---------------------------------------------------------

void Capella::readStaveLayout(CapStaffLayout* sl, int idx)
      {
      sl->barlineMode = readByte();
      sl->noteLines   = readByte();
      switch (sl->noteLines) {
            case 1:     break;      // one line
            case 2:     break;      // five lines
            default:
                  {
                  char lines[11];
                  f->read(lines, 11);
                  curPos += 11;
                  }
                  break;
            }
      qDebug("StaffLayout %d: barlineMode %d noteLines %d", idx, sl->barlineMode, sl->noteLines);

      sl->bSmall      = readByte();
      qDebug("staff size small %d", sl->bSmall);

      sl->topDist      = readInt();
      sl->btmDist      = readInt();
      sl->groupDist    = readInt();
      sl->barlineFrom = readByte();
      sl->barlineTo   = readByte();
      // qDebug("topDist %d btmDist %d groupDist %d barlineFrom %d barlineTo %d",
      //        sl->topDist, sl->btmDist, sl->groupDist, sl->barlineFrom, sl->barlineTo);

      unsigned char clef = readByte();
      sl->form = Form(clef & 7);
      sl->line = ClefLine((clef >> 3) & 7);
      sl->oct  = Oct((clef >> 6));
      qDebug("   clef %x  form %d, line %d, oct %d", clef, sl->form, sl->line, sl->oct);

      // Schlagzeuginformation
      unsigned char b   = readByte();
      sl->bPercussion  = b & 1;    // Schlagzeugkanal verwenden
      sl->bSoundMapIn  = b & 2;
      sl->bSoundMapOut = b & 4;
      if (sl->bSoundMapIn) {      // Umleitungstabelle für Eingabe vom Keyboard
            uchar iMin = readByte();
            Q_UNUSED(iMin);
            uchar n    = readByte();
            Q_ASSERT (n > 0 and iMin + n <= 128);
            f->read(sl->soundMapIn, n);
            curPos += n;
            }
      if (sl->bSoundMapOut) {     // Umleitungstabelle für das Vorspielen
            unsigned char iMin = readByte();
            Q_UNUSED(iMin);
            unsigned char n    = readByte();
            Q_ASSERT (n > 0 and iMin + n <= 128);
            f->read(sl->soundMapOut, n);
            curPos += n;
            }
      sl->sound  = readInt();
      sl->volume = readInt();
      sl->transp = readInt();
      qDebug("   sound %d vol %d transp %d", sl->sound, sl->volume, sl->transp);

      sl->descr              = readQString();
      sl->name               = readQString();
      sl->abbrev             = readQString();
      sl->intermediateName   = readQString();
      sl->intermediateAbbrev = readQString();
      qDebug("   descr <%s> name <%s>  abbrev <%s> iname <%s> iabrev <%s>",
             qPrintable(sl->descr), qPrintable(sl->name), qPrintable(sl->abbrev),
             qPrintable(sl->intermediateName), qPrintable(sl->intermediateAbbrev));
      }

//---------------------------------------------------------
//   readLayout
//---------------------------------------------------------

void Capella::readLayout()
      {
      smallLineDist  = double(readInt()) / 100;
      normalLineDist = double(readInt()) / 100;
      qDebug("Capella::readLayout(): smallLineDist %g normalLineDist %g", smallLineDist, normalLineDist);

      topDist        = readInt();
      interDist      = readInt();
      qDebug("Capella::readLayout(): topDist %d", topDist);

      txtAlign   = readByte();    // Stimmenbezeichnungen 0=links, 1=zentriert, 2=rechts
      adjustVert = readByte();    // 0=nein, 1=au�er letzte Seite, 3=alle Seiten

      unsigned char b          = readByte();
      redundantKeys    = b & 1;
      modernDoubleNote = b & 2;
      Q_ASSERT((b & 0xFC) == 0); // bits 2...7 reserviert

      bSystemSeparators = readByte();
      nUnnamed           = readInt();

      namesFont = readFont();

      // Musterzeilen
      unsigned nStaveLayouts = readUnsigned();

      // qDebug("%d staves", nStaveLayouts);

      for (unsigned iStave = 0; iStave < nStaveLayouts; iStave++) {
            CapStaffLayout* sl = new CapStaffLayout;
            readStaveLayout(sl, iStave);
            _staffLayouts.append(sl);
            }

      // system brackets:
      unsigned n = readUnsigned();  // number of brackets
      for (unsigned int i = 0; i < n; i++) {
            CapBracket b;
            b.from   = readInt();
            b.to     = readInt();
            b.curly = readByte();
            // qDebug("Bracket%d %d-%d curly %d", i, b.from, b.to, b.curly);
            brackets.append(b);
            }
      // qDebug("Capella::readLayout(): done");
      }

//---------------------------------------------------------
//   readExtra
//---------------------------------------------------------

void Capella::readExtra()
      {
      uchar n = readByte();
      if (n) {
            qDebug("Capella::readExtra(%d)", n);
            for (int i = 0; i < n; ++i)
                  readByte();
            }
      }

//---------------------------------------------------------
//   CapClef::read
//---------------------------------------------------------

void CapClef::read()
      {
      unsigned char b = cap->readByte();
      form            = Form(b & 7);
      line            = ClefLine((b >> 3) & 7);
      oct             = Oct(b >> 6);
      qDebug("Clef::read form %d line %d oct %d", form, line, oct);
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

ClefType CapClef::clef() const
      {
      return clefType(form, line, oct);
      }

ClefType CapClef::clefType(Form form, ClefLine line, Oct oct)
      {
      int idx = int(form) + (int(line) << 3) + (int(oct) << 5);
      switch (idx) {
            case int(Form::G) + (int(ClefLine::L2) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::G;
            case int(Form::G) + (int(ClefLine::L2) << 3) + (int(Oct::OCT_ALTA) << 5):  return ClefType::G1;
            case int(Form::G) + (int(ClefLine::L2) << 3) + (int(Oct::OCT_BASSA) << 5): return ClefType::G3;

            case int(Form::C) + (int(ClefLine::L1) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::C1;
            case int(Form::C) + (int(ClefLine::L2) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::C2;
            case int(Form::C) + (int(ClefLine::L3) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::C3;
            case int(Form::C) + (int(ClefLine::L4) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::C4;
            case int(Form::C) + (int(ClefLine::L5) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::C5;

            case int(Form::F) + (int(ClefLine::L4) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::F;
            case int(Form::F) + (int(ClefLine::L4) << 3) + (int(Oct::OCT_BASSA) << 5): return ClefType::F8;
            case int(Form::F) + (int(ClefLine::L3) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::F_B;
            case int(Form::F) + (int(ClefLine::L5) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::F_C;

            default:
                  if (form == Form::FORM_NULL)
                        return ClefType::INVALID;
                  qDebug("unknown clef %d %d %d", form, line, oct);
                  break;
            }
      return ClefType::INVALID;
      }

//---------------------------------------------------------
//   CapKey::read
//---------------------------------------------------------

void CapKey::read()
      {
      unsigned char b = cap->readByte();
      signature = int(b) - 7;
      // qDebug("         Key %d", signature);
      }

//---------------------------------------------------------
//   CapMeter::read
//---------------------------------------------------------

void CapMeter::read()
      {
      numerator = cap->readByte();
      uchar d   = cap->readByte();
      log2Denom = (d & 0x7f) - 1;
      allaBreve = d & 0x80;
      qDebug("   Meter %d/%d allaBreve %d", numerator, log2Denom, allaBreve);
      if (log2Denom > 7 || log2Denom < 0) {
            qDebug("   illegal fraction");
            // abort();
            log2Denom = 2;
            numerator = 4;
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void WedgeObj::read()
      {
      LineObj::read();
      char b = cap->readByte();
      height = b & 0x7f;
      decresc = b & 0x80;
      }

//---------------------------------------------------------
//   CapExplicitBarline::read
//---------------------------------------------------------

void CapExplicitBarline::read()
      {
      unsigned char b = cap->readByte();
      _type = BarLineType(b & 0x0f);
      _barMode = b >> 4;         // 0 = auto, 1 = nur Zeilen, 2 = durchgezogen
      Q_ASSERT(_type <= BarLineType::END_START_REPEAT);
      Q_ASSERT(_barMode <= 2);

      qDebug("         Expl.Barline type %d mode %d", _type, _barMode);
      }

//---------------------------------------------------------
//   readVoice
//---------------------------------------------------------

void Capella::readVoice(CapStaff* cs, int idx)
      {
      qDebug("      readVoice %d", idx);

      if (readChar() != 'C')
            throw Capella::Error::BAD_VOICE_SIG;

      CapVoice* v   = new CapVoice;
      v->voiceNo    = idx;
      v->y0Lyrics   = readByte();
      v->dyLyrics   = readByte();
      v->lyricsFont = readFont();
      v->stemDir    = readByte();
      readExtra();

      unsigned nNoteObjs = readUnsigned();          // Notenobjekte
      for (unsigned i = 0; i < nNoteObjs; i++) {
            QColor color       = Qt::black;
            uchar type = readByte();
            // qDebug("         Voice %d read object idx %d(%d) type %d", idx,  i, nNoteObjs, type);
            readExtra();
            if ((type != uchar(CapellaNoteObjectType::REST)) && (type != uchar(CapellaNoteObjectType::CHORD)) && (type != uchar(CapellaNoteObjectType::PAGE_BKGR)))
                  color = readColor();

            // Die anderen Objekttypen haben eine komprimierte Farbcodierung
            switch (CapellaNoteObjectType(type)) {
                  case CapellaNoteObjectType::REST:
                        {
                        RestObj* rest = new RestObj(this);
                        rest->read();
                        v->objects.append(rest);
                        }
                        break;
                  case CapellaNoteObjectType::CHORD:
                  case CapellaNoteObjectType::PAGE_BKGR:
                        {
                        ChordObj* chord = new ChordObj(this);
                        chord->read();
                        v->objects.append(chord);
                        }
                        break;
                  case CapellaNoteObjectType::CLEF:
                        {
                        CapClef* clef = new CapClef(this);
                        clef->read();
                        v->objects.append(clef);
                        }
                        break;
                  case CapellaNoteObjectType::KEY:
                        {
                        CapKey* key = new CapKey(this);
                        key->read();
                        v->objects.append(key);
                        }
                        break;
                  case CapellaNoteObjectType::METER:
                        {
                        CapMeter* meter = new CapMeter(this);
                        meter->read();
                        v->objects.append(meter);
                        }
                        break;
                  case CapellaNoteObjectType::EXPL_BARLINE:
                        {
                        CapExplicitBarline* bl = new CapExplicitBarline(this);
                        bl->read();
                        qDebug("append Expl Barline==========");
                        v->objects.append(bl);
                        }
                        break;
                  default:
                        qFatal("bad voice type %d", type);
                  }
            }
      cs->voices.append(v);
      }

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

void Capella::readStaff(CapSystem* system)
      {
      if (readChar() != 'V')
            throw Capella::Error::BAD_STAFF_SIG;

      CapStaff* staff = new CapStaff;
      //Meter
      staff->numerator = readByte();
      uchar d          = readByte();
      staff->log2Denom = (d & 0x7f) - 1;
      staff->allaBreve = d & 0x80;

      staff->iLayout   = readByte();
      staff->topDistX  = readInt();
      staff->btmDistX  = readInt();
      staff->color     = readColor();
      readExtra();

      qDebug("      Staff iLayout %d", staff->iLayout);
      // Stimmen
      unsigned nVoices = readUnsigned();
      for (unsigned i = 0; i < nVoices; i++)
            readVoice(staff, i);
      system->staves.append(staff);
      }

//---------------------------------------------------------
//   readSystem
//---------------------------------------------------------

void Capella::readSystem()
      {
      if (readChar() != 'S')
            throw Capella::Error::BAD_SYSTEM_SIG;

      CapSystem* s = new CapSystem;
      s->nAddBarCount   = readInt();
      s->bBarCountReset = readByte();
      s->explLeftIndent = readByte();

      s->beamMode = BeamMode(readByte());
      s->tempo    = readUnsigned();
      s->color    = readColor();
      readExtra();

      unsigned char b  = readByte();
      s->bJustified    = b & 2;
      s->bPageBreak    = (b & 4) != 0;
      s->instrNotation = (b >> 3) & 7;

      unsigned nStaves = readUnsigned();
      for (unsigned i = 0; i < nStaves; i++)
            readStaff(s);
      systems.append(s);
      }

//---------------------------------------------------------
//   toTicks
//---------------------------------------------------------

int BasicDurationalObj::ticks() const
      {
      if (noDuration)
            return 0;
      int len = 0;
      switch (t) {
            case TIMESTEP::D1:          len = 4 * MScore::division; break;
            case TIMESTEP::D2:          len = 2 * MScore::division; break;
            case TIMESTEP::D4:          len = MScore::division; break;
            case TIMESTEP::D8:          len = MScore::division >> 1; break;
            case TIMESTEP::D16:         len = MScore::division >> 2; break;
            case TIMESTEP::D32:         len = MScore::division >> 3; break;
            case TIMESTEP::D64:         len = MScore::division >> 4; break;
            case TIMESTEP::D128:        len = MScore::division >> 5; break;
            case TIMESTEP::D256:        len = MScore::division >> 6; break;
            case TIMESTEP::D_BREVE:     len = MScore::division * 8; break;
            default:
                  qDebug("BasicDurationalObj::ticks: illegal duration value %d", t);
                  break;
            }
      int slen = len;
      int dots = nDots;
      while (dots--) {
            slen >>= 1;
            len += slen;
            }
      return len;
      }

//---------------------------------------------------------
//   readPoint
//---------------------------------------------------------

QPointF Capella::readPoint()
      {
      int x = readInt();
      int y = readInt();
      return QPointF(double(x), double(y));
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Capella::read(QFile* fp)
      {
      f      = fp;
      curPos = 0;

      char signature[9];
      read(signature, 8);
      signature[8] = 0;
      if (memcmp(signature, "cap3-v:", 7) != 0)
            throw Capella::Error::BAD_SIG;

      // qDebug("read Capella file signature <%s>", signature);

      // TODO: test for signature[7] = a-z

      author   = readString();
      keywords = readString();
      comment  = readString();

      // qDebug("author <%s> keywords <%s> comment <%s>", author, keywords, comment);

      nRel   = readUnsigned();            // 75
      nAbs   = readUnsigned();            // 16
      unsigned char b   = readByte();
      bUseRealSize      = b & 1;
      bAllowCompression = b & 2;
      bPrintLandscape   = b & 16;

      // qDebug("  nRel %d  nAbs %d useRealSize %d compresseion %d", nRel, nAbs, bUseRealSize, bAllowCompression);

      readLayout();

      beamRelMin0 = readByte();        // basic setup for beam slope
      beamRelMin1 = readByte();
      beamRelMax0 = readByte();
      beamRelMax1 = readByte();

      readExtra();

      readDrawObjectArray();                // Galerie (gesammelte Grafikobjekte)

      unsigned n = readUnsigned();
      if (n) {
            qDebug("Gallery objects");
            }
      for (unsigned int i = 0; i < n; ++i) {
            /*char* s =*/ readString();     // Namen der Galerie-Objekte
            // qDebug("Galerie: <%s>", s);
            }

      // qDebug("read backgroundChord");
      backgroundChord = new ChordObj(this);
      backgroundChord->read();              // contains graphic objects on the page background
      // qDebug("read backgroundChord done");
      bShowBarCount    = readByte();        // Taktnumerierung zeigen
      barNumberFrame   = readByte();        // 0=kein, 1=Rechteck, 2=Ellipse
      nBarDistX        = readByte();
      nBarDistY        = readByte();
      QFont barNumFont = readFont();
      nFirstPage       = readUnsigned();    // Versatz fuer Seitenzaehlung
      leftPageMargins  = readUnsigned();    // Seitenraender
      topPageMargins   = readUnsigned();
      rightPageMargins = readUnsigned();
      btmPageMargins   = readUnsigned();

      unsigned nSystems  = readUnsigned();
      for (unsigned i = 0; i < nSystems; i++)
            readSystem();
      char esig[4];
      read(esig, 4);
      if (memcmp (esig, "\0\0\0\0", 4) != 0)
            throw Capella::Error::BAD_SIG;
      }

//---------------------------------------------------------
//   importCapella
//---------------------------------------------------------

Score::FileError importCapella(Score* score, const QString& name)
      {
      QFile fp(name);
      if(!fp.exists())
            return Score::FileError::FILE_NOT_FOUND;
      if (!fp.open(QIODevice::ReadOnly))
            return Score::FileError::FILE_OPEN_ERROR;

      Capella cf;
      try {
            cf.read(&fp);
            }
      catch (Capella::Error errNo) {
            if (!MScore::noGui) {
                  QMessageBox::warning(0,
                     QWidget::tr("MuseScore: Import Capella"),
                     QWidget::tr("Load failed: ") + cf.error(errNo),
                     QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
                  }
            fp.close();
            // avoid another error message box
            return Score::FileError::FILE_NO_ERROR;
            }
      fp.close();
      convertCapella(score, &cf, false);
      return Score::FileError::FILE_NO_ERROR;
      }
}

