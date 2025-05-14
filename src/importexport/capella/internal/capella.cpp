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

//
//    Capella 2000 import filter
//
#include "capella.h"

#include <QFile>
#include <QtMath>

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/box.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/trill.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/utils.h"
#include "engraving/dom/volta.h"

#include "engraving/engravingerrors.h"
#include "engraving/infrastructure/messagebox.h"

#include "log.h"

#include "translation.h"

extern QString rtf2html(const QString&);

using namespace mu::engraving;

namespace mu::iex::capella {
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
    "bad file content",
};

//---------------------------------------------------------
//   addDynamic
//---------------------------------------------------------

static void addDynamic(Score*, Segment* s, int track, const char* name)
{
    Dynamic* d = Factory::createDynamic(s);
    d->setDynamicType(String::fromUtf8(name));
    d->setTrack(track);
    s->add(d);
}

//---------------------------------------------------------
//   addArticulationText
//---------------------------------------------------------

static void addArticulationText(Score*, ChordRest* cr, int track, SymId symId)
{
    Articulation* na = Factory::createArticulation(cr);
    na->setTrack(track);
    na->setSymId(symId);
    cr->add(na);
}

//---------------------------------------------------------
//   SetCapGraceDuration
//---------------------------------------------------------

static void SetCapGraceDuration(Chord* chord, ChordObj* o)
{
    NoteType nt = NoteType::APPOGGIATURA;
    if (o->nTremoloBars > 0) {
        nt = NoteType::ACCIACCATURA;
    }
    ((Chord*)chord)->setNoteType(nt);
    if (o->t == TIMESTEP::D4) {
        ((Chord*)chord)->setNoteType(NoteType::GRACE4);
        chord->setDurationType(DurationType::V_QUARTER);
    } else if (o->t == TIMESTEP::D_BREVE) {
        ((Chord*)chord)->setDurationType(DurationType::V_BREVE);
    } else if (o->t == TIMESTEP::D1) {
        ((Chord*)chord)->setDurationType(DurationType::V_WHOLE);
    } else if (o->t == TIMESTEP::D2) {
        ((Chord*)chord)->setDurationType(DurationType::V_HALF);
    } else if (o->t == TIMESTEP::D16) {
        ((Chord*)chord)->setNoteType(NoteType::GRACE16);
        chord->setDurationType(DurationType::V_16TH);
    } else if (o->t == TIMESTEP::D32) {
        ((Chord*)chord)->setNoteType(NoteType::GRACE32);
        chord->setDurationType(DurationType::V_32ND);
    } else if (o->t == TIMESTEP::D64) {
        ((Chord*)chord)->setDurationType(DurationType::V_64TH);
    } else if (o->t == TIMESTEP::D128) {
        ((Chord*)chord)->setDurationType(DurationType::V_128TH);
    } else if (o->t == TIMESTEP::D256) {
        ((Chord*)chord)->setDurationType(DurationType::V_256TH);
    } else {
        ((Chord*)chord)->setDurationType(DurationType::V_EIGHTH);
    }
}

//---------------------------------------------------------
//   processBasicDrawObj
//---------------------------------------------------------

static void processBasicDrawObj(QList<BasicDrawObj*> objects, Segment* s, int track, ChordRest* cr)
{
    Score* score = s->score();
    for (BasicDrawObj* oo : objects) {
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
                    case 'z':                         // sfz
                        addDynamic(score, s, track, "sfz");
                        break;
                    case '{':
                        addDynamic(score, s, track, "fz");
                        break;
                    case '|':
                        addDynamic(score, s, track, "fp");
                        break;
                    case 212:                         // dynamic m
                        addDynamic(score, s, track, "m");
                        break;
                    case 213:                         // dynamic r
                        addDynamic(score, s, track, "r");
                        break;
                    case 214:                         // dynamic s
                        addDynamic(score, s, track, "s");
                        break;
                    case 215:                         // dynamic z
                        addDynamic(score, s, track, "z");
                        break;
                    case 'k':                         // fermata down
                        addArticulationText(score, cr, track, SymId::fermataBelow);
                        break;
                    case 'u':                         // fermata up
                        addArticulationText(score, cr, track, SymId::fermataAbove);
                        break;
                    case 'd':                         // da capo D.C.
                    case 'e':                         // dal segno D.S.
                    case 'n':                         // segno coda
                    case 'o':                         // segno coda (smaller)
                    case 'y':                         // segno
                    case '$':                         // segno variation
                    case 'a':                         // pedal Ped.
                    case 'b':                         // pedal asterisk *
                    case 'v':                         // 8va
                    case 186:                         // 15ma
                        LOGD("Import of Capella text articulation %x(%c) not yet implemented", code, code);
                        break;
                    case 181:                         // caesura
                    {
                        Segment* seg = s->measure()->getSegment(SegmentType::Breath, s->tick() + (cr ? cr->actualTicks() : Fraction(0, 1)));
                        Breath* b = Factory::createBreath(seg);
                        b->setTrack(track);
                        b->setSymId(SymId::caesura);
                        seg->add(b);
                    }
                    break;
                    default:
                        break;
                    }
                    if (cr && cr->type() == ElementType::CHORD) {
                        switch (code) {
                        case 172:                           // arpeggio (short)
                        case 173:                           // arpeggio (long)
                        {
                            Arpeggio* a = Factory::createArpeggio(toChord(cr));
                            a->setArpeggioType(ArpeggioType::NORMAL);
                            if (mu::engraving::toChord(cr)->arpeggio()) {                           // there can be only one
                                delete a;
                                a = 0;
                            } else {
                                cr->add(a);
                            }
                        }
                        break;
                        case 187:                           // arpeggio (wiggle line, arrow up)
                        {
                            Arpeggio* a = Factory::createArpeggio(toChord(cr));
                            a->setArpeggioType(ArpeggioType::UP);
                            if ((static_cast<Chord*>(cr))->arpeggio()) {                           // there can be only one
                                delete a;
                                a = 0;
                            } else {
                                cr->add(a);
                            }
                        }
                        break;
                        case 188:                           // arpeggio (wiggle line, arrow down)
                        {
                            Arpeggio* a = Factory::createArpeggio(toChord(cr));
                            a->setArpeggioType(ArpeggioType::DOWN);
                            if ((static_cast<Chord*>(cr))->arpeggio()) {                           // there can be only one
                                delete a;
                                a = 0;
                            } else {
                                cr->add(a);
                            }
                        }
                        break;
                        default:
                            break;
                        }
                    }
                    break;
                }
            }
            StaffText* text = Factory::createStaffText(s);
            QFont f(st->font());
            text->setFamily(f.family());
            text->setItalic(f.italic());
            // text->setUnderline(f.underline());
            // text->setStrike(f.strikeOut());
            text->setBold(f.bold());
            text->setSize(f.pointSizeF());

            text->setPlainText(st->text());
            QPointF p(st->pos());
            p = p / 32.0 * score->style().spatium();
            // text->setUserOff(st->pos());
            text->setOffset(muse::PointF::fromQPointF(p));
            // LOGD("setText %s (%f %f)(%f %f) <%s>",
            //            qPrintable(st->font().family()),
            //            st->pos().x(), st->pos().y(), p.x(), p.y(), qPrintable(st->text()));
            AlignH textalign;
            switch (st->textalign()) {
            default:
            case 0:
                textalign = AlignH::LEFT;
                break;
            case 1:
                textalign = AlignH::HCENTER;
                break;
            case 2:
                textalign = AlignH::RIGHT;
                break;
            }
            text->setAlign(Align(textalign, AlignV::BASELINE));
            text->setOffset(muse::PointF(0.0, 2.0));
            text->setTrack(track);
            s->add(text);
        }
        break;
        case CapellaType::TRANSPOSABLE:
        {
            TransposableObj* to = static_cast<TransposableObj*>(oo);
            QString str = "";
            for (BasicDrawObj* bdo : to->variants) {
                SimpleTextObj* st = static_cast<SimpleTextObj*>(bdo);
                if (st->font().family() == "capella3") {
                    for (const QChar& ch : st->text()) {
                        if (ch == QChar('Q')) {
                            str += "b";
                        } else if (ch == QChar('S')) {
                            str += "#";
                        } else if (ch == QChar('R')) {
                            str += "natural";
                        } else {
                            str += ch;
                        }
                    }
                } else {
                    str += st->text();
                }
            }
            Harmony* harmony = Factory::createHarmony(s);
            harmony->setHarmony(str);
            harmony->setTrack(track);
            s->add(harmony);
            break;
        }
        case CapellaType::TEXT:
            LOGD("======================Text:");
            break;
        default:
            break;
        }
    }
}

//---------------------------------------------------------
//   TupletFractionCap
//---------------------------------------------------------

Fraction TupletFractionCap(int tupletCount, bool tuplettrp, bool tupletprol)
{
    int dd         = 0;
    int nn         = 0;
    qreal exponent = 0;
    qreal count    = tupletCount;
    Fraction f(3, 2);

    if ((count > 0) && (count <= 15)) {
        if (tuplettrp) {
            exponent = qFloor(qLn(count / 3.0) / qLn(2.0));
        } else {
            exponent = qFloor(qLn(count) / qLn(2.0));
        }
    } else {
        LOGD("Unknown tuplet, count = %d", tupletCount);
        return f;
    }
    if (tupletprol) {
        exponent += 1.0;
    }
    if (exponent < 0.0) {
        exponent = 0.0;
    }
    nn = tupletCount;
    dd = static_cast<int>(qPow(2.0, exponent));
    if (tuplettrp) {
        dd = dd * 3;
    }
    LOGD("Tuplet Fraction: %d / %d", nn, dd);
    return Fraction(nn, dd);
}

//---------------------------------------------------------
//   findChordRests -- find begin and end ChordRest for BasicDrawObj o
//   return true on success (both begin and end found)
//---------------------------------------------------------

static bool findChordRests(BasicDrawObj const* const o, Score* score, const int track, const Fraction& tick,
                           ChordRest*& cr1, ChordRest*& cr2, NoteObj* no, QList<NoteObj*> objects)
{
    cr1 = 0;                           // ChordRest where BasicDrawObj o begins
    cr2 = 0;                           // ChordRest where BasicDrawObj o ends

    // find the ChordRests where o begins and ends
    int n = o->nNotes + 1;                                  // # notes in BasicDrawObj (nNotes is # notes following the first note)
    int graceNumber = 0;
    int graceNumber1 = 0;
    bool foundcr1 = false;
    Fraction tick2 = tick;
    for (NoteObj* nobj : objects) {
        BasicDurationalObj* d = 0;
        if (nobj->type() == CapellaNoteObjectType::REST) {
            d = static_cast<BasicDurationalObj*>(static_cast<RestObj*>(nobj));
            graceNumber = 0;
        } else if (nobj->type() == CapellaNoteObjectType::CHORD) {
            ChordObj* cho = static_cast<ChordObj*>(nobj);
            d = static_cast<BasicDurationalObj*>(cho);
            if (!(cho->invisible) && (cho->ticks().isZero())) {     // grace note
                ++graceNumber;
            } else {
                graceNumber = 0;
            }
        }
        if (!d) {
            continue;
        }
        if (nobj == no) {
            foundcr1 = true;
            graceNumber1 = graceNumber;
        }
        Fraction ticks = Fraction(0, 1);
        if (foundcr1) {
            --n;         // found the object corresponding to cr1, count down to find the second one
            ticks = d->ticks();
            if (d->count) {
                Fraction f = TupletFractionCap(d->count, d->tripartite, d->isProlonging);
                ticks = ticks / f;
            }
            if (nobj->type() == CapellaNoteObjectType::REST) {
                RestObj* ro = static_cast<RestObj*>(nobj);
                if (ro->fullMeasures) {
                    Measure* m  = score->getCreateMeasure(tick2);
                    Fraction ft = m->ticks();
                    ticks       = ft * ro->fullMeasures;
                }
            }
            if (n == 0) {
                break;
            }
            tick2 += ticks;
        }
    }
    // Now we have the tick (tick) and the level of grace note (graceNumber1, if "no" is a grace note) for the first ChordRest
    // and the tick (tick2) and the level of grace note (graceNumber, if the target is a grace note) for the 2nd ChordRest
    for (Segment* seg = score->tick2segment(tick); seg; seg = seg->next1()) {
        if (seg->segmentType() != SegmentType::ChordRest) {
            continue;
        }
        ChordRest* cr = toChordRest(seg->element(track));
        if (cr) {
            if ((graceNumber1 > 0) && cr->isChord()) {       // the spanner is starting from a grace note
                Chord* chord = toChord(cr);
                for (Chord* cc : chord->graceNotes()) {
                    --graceNumber1;
                    if ((graceNumber1 == 0) && (!cr1)) {
                        cr1 = toChordRest(cc);             // found first ChordRest
                    }
                }
            }
            if (!cr1) {
                cr1 = cr;             // found first ChordRest
            }
            break;
        }
    }
    for (Segment* seg = score->tick2segment(tick2); seg; seg = seg->next1()) {
        if (seg->segmentType() != SegmentType::ChordRest) {
            continue;
        }
        ChordRest* cr = toChordRest(seg->element(track));
        if (cr) {
            if ((graceNumber > 0) && cr->isChord()) {       // the spanner is ending on a grace note
                Chord* chord = toChord(cr);
                for (Chord* cc : chord->graceNotes()) {
                    --graceNumber;
                    if ((graceNumber == 0) && (!cr2)) {
                        cr2 = toChordRest(cc);             // found 2nd ChordRest
                    }
                }
            }
            if (!cr2) {
                cr2 = cr;             // found 2nd ChordRest
            }
            break;
        }
    }
    LOGD("findChordRests o %p nNotes %d score %p track %d tick %d cr1 %p cr2 %p",
         o, o->nNotes, score, track, tick.ticks(), cr1, cr2);

    if (!(cr1 && cr2)) {
        LOGD("first or second anchor for BasicDrawObj not found (tick %d type %d track %d first %p second %p)",
             tick.ticks(), int(o->type), track, cr1, cr2);
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   createAndAddTimeSig
//---------------------------------------------------------

static Segment* createAndAddTimeSig(Score*, Measure* m, Fraction f, int track, Fraction tick)
{
    auto s = m->getSegment(SegmentType::TimeSig, tick);
    TimeSig* ts = Factory::createTimeSig(s);
    ts->setSig(f);
    ts->setTrack(track);
    s->add(ts);
    return s;
}

//---------------------------------------------------------
//   readCapVoice
//---------------------------------------------------------

static Fraction readCapVoice(Score* score, CapVoice* cvoice, int staffIdx, const Fraction& t, bool capxMode)
{
    Fraction tick(t);
    int voice = cvoice->voiceNo;
    int track = staffIdx * VOICES + voice;

    //
    // pass I
    //
    Fraction startTick = tick;

    Tuplet* tuplet  = nullptr;
    int tupletCount = 0;
    bool tuplettrp  = false;
    bool tupletprol = false;
    int nTuplet     = 0;
    Fraction tupletTick = Fraction(0, 1);
    ClefType pclef = score->staff(staffIdx)->defaultClefType().concertClef;

    QList<Chord*> graceNotes;
    for (NoteObj* no : cvoice->objects) {
        switch (no->type()) {
        case CapellaNoteObjectType::REST:
        {
            LOGD("     <Rest>");
            Measure* m = score->getCreateMeasure(tick);
            RestObj* o = static_cast<RestObj*>(no);
            Fraction ticks  = o->ticks();
            if (o->invisible && ticks.isZero()) {             // get rid of placeholders
                break;
            }
            TDuration d;
            d.setVal(ticks.ticks());
            if (o->count) {
                if (tuplet == nullptr) {
                    tupletCount = o->count;
                    tuplettrp   = o->tripartite;
                    tupletprol  = o->isProlonging;
                    nTuplet     = 0;
                    tupletTick  = tick;
                    tuplet      = Factory::createTuplet(m);
                    Fraction f  = TupletFractionCap(tupletCount, tuplettrp, tupletprol);
                    tuplet->setRatio(f);
                    tuplet->setBaseLen(d);
                    tuplet->setTrack(track);
                    tuplet->setTick(tick);
                    tuplet->setParent(m);
                    Fraction nn = (ticks * tupletCount) / f;
                    tuplet->setTicks(nn);
                }
            }

            Fraction ft = m->ticks();
            if (o->fullMeasures) {
                ticks = ft * o->fullMeasures;
                if (!o->invisible) {
                    for (unsigned i = 0; i < o->fullMeasures; ++i) {
                        Measure* m1 = score->getCreateMeasure(tick + (ft * i));
                        Segment* s = m1->getSegment(SegmentType::ChordRest, tick + (ft * i));
                        Rest* rest = Factory::createRest(s);
                        rest->setDurationType(TDuration(DurationType::V_MEASURE));
                        rest->setTicks(m1->ticks());
                        rest->setTrack(staffIdx * VOICES + voice);
                        s->add(rest);
                    }
                }
            }
            if (!o->invisible || voice == 0) {
                Segment* s = m->getSegment(SegmentType::ChordRest, tick);
                Rest* rest = Factory::createRest(s);
                TDuration d1;
                if (o->fullMeasures) {
                    d1.setType(DurationType::V_MEASURE);
                    rest->setTicks(m->ticks());
                } else {
                    d1.setVal(ticks.ticks());
                    rest->setTicks(d1.fraction());
                }
                rest->setDurationType(d1);
                rest->setTrack(track);
                rest->setVisible(!o->invisible);
                s->add(rest);
                if (tuplet) {
                    tuplet->add(rest);
                }
                processBasicDrawObj(o->objects, s, track, rest);
            }

            if (tuplet) {
                if (++nTuplet >= tupletCount) {
                    tick = tupletTick + tuplet->actualTicks();
                    //! NOTE If the tuplet is not added anywhere, then delete it
                    if (tuplet->elements().empty()) {
                        delete tuplet;
                    }

                    tuplet = nullptr;
                } else {
                    tick += ticks / tuplet->ratio();
                }
            } else {
                tick += ticks;
            }
        }
        break;
        case CapellaNoteObjectType::CHORD:
        {
            LOGD("     <Chord>");
            ChordObj* o = static_cast<ChordObj*>(no);
            Fraction ticks = o->ticks();
            if (o->invisible && ticks.isZero()) {              // get rid of placeholders
                break;
            }
            TDuration d;
            d.setVal(ticks.ticks());
            Measure* m = score->getCreateMeasure(tick);

            bool isgracenote = (!(o->invisible) && (ticks.isZero()));
            if (o->count) {
                if (tuplet == nullptr) {
                    tupletCount = o->count;
                    tuplettrp   = o->tripartite;
                    tupletprol  = o->isProlonging;
                    nTuplet     = 0;
                    tupletTick  = tick;
                    tuplet      = Factory::createTuplet(m);
                    Fraction f  = TupletFractionCap(tupletCount, tuplettrp, tupletprol);
                    tuplet->setRatio(f);
                    tuplet->setBaseLen(d);
                    tuplet->setTrack(track);
                    tuplet->setTick(tick);
                    tuplet->setParent(m);
                    Fraction nn = (ticks * tupletCount) / f;
                    tuplet->setTicks(nn);
                }
                LOGD("Tuplet at %d: count: %d  tri: %d  prolonging: %d  ticks %d objects %lld",
                     tick.ticks(), o->count, o->tripartite, o->isProlonging, ticks.ticks(),
                     o->objects.size());
            }

            Chord* chord = Factory::createChord(score->dummy()->segment());
            if (isgracenote) {             // grace notes
                SetCapGraceDuration(chord, o);
                chord->setTicks(chord->durationType().fraction());
            } else {           // normal notes
                chord->setDurationType(d);
                chord->setTicks(d.fraction());
            }
            chord->setTrack(track);
            switch (o->stemDir) {
            case ChordObj::StemDir::DOWN:
                chord->setStemDirection(DirectionV::DOWN);
                break;
            case ChordObj::StemDir::UP:
                chord->setStemDirection(DirectionV::UP);
                break;
            case ChordObj::StemDir::NONE:
                chord->setNoStem(true);
                break;
            case ChordObj::StemDir::AUTO:
            default:
                break;
            }
            Segment* s = m->getSegment(SegmentType::ChordRest, tick);
            if (isgracenote) {
                graceNotes.push_back(chord);
            } else {
                s->add(chord);
                // append grace notes before
                int ii = -1;
                for (ii = graceNotes.size() - 1; ii >= 0; ii--) {
                    Chord* gc = graceNotes[ii];
                    if (gc->voice() == chord->voice()) {
                        chord->add(gc);
                    }
                }
                graceNotes.clear();
            }
            if (tuplet) {
                tuplet->add(chord);
            }
            ClefType clef = score->staff(staffIdx)->clef(tick);
            Key key  = score->staff(staffIdx)->key(tick);
            int off;
            switch (clef) {
            case ClefType::G:      off = 0;
                break;
            case ClefType::G8_VA:  off = 7;
                break;
            case ClefType::G15_MA: off = 14;
                break;
            case ClefType::G8_VB:  off = -7;
                break;
            case ClefType::F:      off = -14;
                break;
            case ClefType::F8_VB:  off = -21;
                break;
            case ClefType::F15_MB: off = -28;
                break;
            case ClefType::F_B:    off = -14;
                break;
            case ClefType::F_C:    off = -14;
                break;
            case ClefType::C1:     off = -7;
                break;
            case ClefType::C2:     off = -7;
                break;
            case ClefType::C3:     off = -7;
                break;
            case ClefType::C4:     off = -7;
                break;
            case ClefType::C4_8VB: off = -14;
                break;
            case ClefType::C5:     off = -7;
                break;
            case ClefType::G_1:    off = 0;
                break;
            case ClefType::F_8VA:  off = -7;
                break;
            case ClefType::F_15MA: off = 0;
                break;
            default:          off = 0;
                LOGD("clefType %d not implemented", int(clef));
            }
            // LOGD("clef %hhd off %d", clef, off);

            static int keyOffsets[15] = {
                /*   -7 -6 -5 -4 -3 -2 -1  0  1  2  3  4  5  6  7 */
                /* */ 7, 4, 1, 5, 2, 6, 3, 0, 4, 1, 5, 2, 6, 3, 0
            };
            off += keyOffsets[int(key) + 7];

            for (const CNote& n : o->notes) {
                Note* note = Factory::createNote(chord);
                int pitch = 0;
                // .cap import: pitch contains the diatonic note number relative to clef and key
                // .capx  import: pitch the MIDI note number instead
                if (capxMode) {
                    pitch = n.pitch;
                } else {
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
                pitch += n.alteration;
                pitch += score->staff(staffIdx)->part()->instrument()->transpose().chromatic;               // assume not in concert pitch
                pitch = std::clamp(pitch, 0, 127);

                chord->add(note);
                note->setPitch(pitch);
                note->setHeadGroup(NoteHeadGroup(n.headGroup));
                // TODO: compute tpc from pitch & line
                note->setTpcFromPitch();
                if (o->rightTie) {
                    Tie* tie = Factory::createTie(score->dummy());
                    tie->setStartNote(note);
                    tie->setTrack(track);
                    note->setTieFor(tie);
                }
            }
            for (Verse v : o->verse) {
                Lyrics* l = Factory::createLyrics(chord);
                l->setTrack(track);
                l->setPlainText(v.text);
                if (v.hyphen) {
                    l->setSyllabic(LyricsSyllabic::BEGIN);
                }
                l->setNo(v.num);
                l->initTextStyleType(l->isEven() ? TextStyleType::LYRICS_EVEN : TextStyleType::LYRICS_ODD, /*preserveDifferent*/ true);
                chord->add(l);
            }

            processBasicDrawObj(o->objects, s, track, chord);
            switch (o->articulation) {
            case 7:                     // "weak beat"
            case 8:                     // "strong beat"
            default:  if (o->articulation) {
                    LOGD("Articulation # %d not implemented", o->articulation);
            }
                break;
            }

            if (tuplet) {
                if (++nTuplet >= tupletCount) {
                    tick = tupletTick + tuplet->actualTicks();
                    //! NOTE If the tuplet is not added anywhere, then delete it
                    if (tuplet->elements().empty()) {
                        delete tuplet;
                    }

                    tuplet = nullptr;
                } else {
                    tick += ticks / tuplet->ratio();
                }
            } else {
                tick += ticks;
            }
        }
        break;
        case CapellaNoteObjectType::CLEF:
        {
            LOGD("     <Clef>");
            CapClef* o = static_cast<CapClef*>(no);
            ClefType nclef = o->clef();
            LOGD("%d:%d <Clef> %s line %d oct %d clef %d",
                 tick.ticks(), staffIdx, o->name(), int(o->line), int(o->oct), int(o->clef()));
            if (nclef == ClefType::INVALID || nclef == pclef) {
                break;
            }
            pclef = nclef;
            // staff(staffIdx)->setClef(tick, nclef);
            Segment* s;
            Measure* m = score->getCreateMeasure(tick);
            if (tick == m->tick()) {
                s = m->getSegment(SegmentType::HeaderClef, tick);
            } else {
                s = m->getSegment(SegmentType::Clef, tick);
            }
            Clef* clef = Factory::createClef(s);
            clef->setClefType(nclef);
            clef->setTrack(staffIdx * VOICES);
            s->add(clef);
        }
        break;
        case CapellaNoteObjectType::KEY:
        {
            LOGD("   <Key>");
            CapKey* o = static_cast<CapKey*>(no);
            KeySigEvent key = score->staff(staffIdx)->keySigEvent(tick);
            KeySigEvent okey = key;
            Key tKey = Key(o->signature);
            Key cKey = tKey;
            Interval v = score->staff(staffIdx)->part()->instrument(tick)->transpose();
            if (!v.isZero() && !score->style().styleB(mu::engraving::Sid::concertPitch)) {
                cKey = transposeKey(tKey, v);
                // if there are more than 6 accidentals in transposing key, it cannot be PreferSharpFlat::AUTO
                Part* part = score->staff(staffIdx)->part();
                if ((tKey > 6 || tKey < -6) && part->preferSharpFlat() == PreferSharpFlat::AUTO) {
                    part->setPreferSharpFlat(PreferSharpFlat::NONE);
                }
            }
            okey.setConcertKey(cKey);
            okey.setKey(tKey);
            if (!(key == okey)) {
                score->staff(staffIdx)->setKey(tick, okey);
                Measure* m = score->getCreateMeasure(tick);
                Segment* s = m->getSegment(SegmentType::KeySig, tick);
                KeySig* ks = Factory::createKeySig(s);
                ks->setTrack(staffIdx * VOICES);
                ks->setKeySigEvent(okey);
                s->add(ks);
            }
        }
        break;
        case CapellaNoteObjectType::METER:
        {
            CapMeter* o = static_cast<CapMeter*>(no);
            LOGD("     <Meter> tick %d %d/%d", tick.ticks(), o->numerator, 1 << o->log2Denom);
            if (o->log2Denom > 7 || o->log2Denom < 0) {
                ASSERT_X("illegal fraction");
            }
            SigEvent se = score->sigmap()->timesig(tick);
            Fraction f(o->numerator, 1 << o->log2Denom);
            SigEvent ne(f);
            if (!(se == ne)) {
                score->sigmap()->add(tick.ticks(), ne);
            }

            // do not add timesig again
            Measure* m = score->getCreateMeasure(tick);
            Segment* s = m->findSegment(SegmentType::TimeSig, tick);
            if (s) {
                EngravingItem* e = s->element(trackZeroVoice(track));
                if (e && static_cast<TimeSig*>(e)->sig() == f) {
                    break;
                }
            }
            // create timesig in current track
            s = createAndAddTimeSig(score, m, f, track, tick);
            // if not present, also create timesig in track 0
            // to prevent corrupt .msc[xz] files when first staff
            // is empty and timesig is not 4/4
            if (!(s->element(0))) {
                createAndAddTimeSig(score, m, f, 0, tick);
            }
            m->setTicks(f);
        }
        break;
        case CapellaNoteObjectType::EXPL_BARLINE:
        case CapellaNoteObjectType::IMPL_BARLINE:              // does not exist?
        {
            CapExplicitBarline* o = static_cast<CapExplicitBarline*>(no);
            LOGD("     <Barline>");
            Measure* pm = 0;             // the previous measure (the one terminated by this barline)
            if (tick > Fraction(0, 1)) {
                pm = score->getCreateMeasure(tick - Fraction::fromTicks(1));
            }
            if (pm) {
                Fraction ticks = tick - pm->tick();
                if ((ticks > Fraction(0, 1)) && ticks != pm->ticks()) {
                    // this is a measure with different actual duration
                    Fraction f = ticks;
                    pm->setTicks(f);
                }
            }

            BarLineType st = o->type();
            if (st & BarLineType::NORMAL) {
                break;
            }

            if (st & BarLineType::START_REPEAT || st & BarLineType::END_START_REPEAT) {
                Measure* nm = 0;               // the next measure (the one started by this barline)
                nm = score->getCreateMeasure(tick);
                if (nm) {
                    nm->setRepeatStart(true);
                }
            }

            if (st & BarLineType::END_REPEAT || st & BarLineType::END_START_REPEAT) {
                if (pm) {
                    pm->setRepeatEnd(true);
                }
            }
        }
        break;
        case CapellaNoteObjectType::PAGE_BKGR:
            LOGD("     <PageBreak>");
            break;
        }
    }
    Fraction endTick = tick;

    //
    // pass II
    //
    tick = startTick;
    for (NoteObj* no : cvoice->objects) {
        BasicDurationalObj* d = 0;
        if (no->type() == CapellaNoteObjectType::REST) {
            d = static_cast<BasicDurationalObj*>(static_cast<RestObj*>(no));
        } else if (no->type() == CapellaNoteObjectType::CHORD) {
            d = static_cast<BasicDurationalObj*>(static_cast<ChordObj*>(no));
        }
        if (!d) {
            continue;
        }
        for (BasicDrawObj* o : d->objects) {
            switch (o->type) {
            case CapellaType::SIMPLE_TEXT:
                // LOGD("simple text at %d", tick);
                break;
            case CapellaType::WAVY_LINE:
                break;
            case CapellaType::SLUR:
            {
                // SlurObj* so = static_cast<SlurObj*>(o);
                // LOGD("slur tick %d  %d-%d-%d-%d   %d-%d", tick, so->nEnd, so->nMid,
                //        so->nDotDist, so->nDotWidth, so->nRefNote, so->nNotes);
                ChordRest* cr1 = 0;               // ChordRest where slur begins
                ChordRest* cr2 = 0;               // ChordRest where slur ends
                bool res = findChordRests(o, score, track, tick, cr1, cr2, no, cvoice->objects);

                if (res) {
                    if (cr1 == cr2) {
                        LOGD("first and second anchor for slur identical (tick %d track %d first %p second %p)",
                             tick.ticks(), track, cr1, cr2);
                    } else {
                        Slur* slur = Factory::createSlur(score->dummy());
                        LOGD("tick %d track %d cr1 %p cr2 %p -> slur %p",
                             tick.ticks(), track, cr1, cr2, slur);
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
                MeasureBase* measure = score->measures()->first();
                Text* s = Factory::createText(measure, TextStyleType::TITLE);
                QString ss = ::rtf2html(QString(to->text));

                // LOGD("string %f:%f w %d ratio %d <%s>",
                //    to->relPos.x(), to->relPos.y(), to->width, to->yxRatio, qPrintable(ss));
                s->setXmlText(ss);

                if (measure->type() != ElementType::VBOX) {
                    MeasureBase* mb = Factory::createVBox(score->dummy()->system());
                    mb->setTick(Fraction(0, 1));
                    score->addMeasure(mb, measure);
                    measure = mb;
                }
                s->setParent(measure);
                measure->add(s);
            }
            break;
            case CapellaType::VOLTA:
            {
                VoltaObj* vo = static_cast<VoltaObj*>(o);
                ChordRest* cr1 = 0;               // ChordRest where volta begins
                ChordRest* cr2 = 0;               // ChordRest where volta ends
                bool res = findChordRests(o, score, track, tick, cr1, cr2, no, cvoice->objects);

                if (res) {
                    Volta* volta = Factory::createVolta(score->dummy());
                    volta->setTrack(track);
                    volta->setTrack2(track);
                    // TODO also support endings such as "1 - 3"
                    volta->setText(QString("%1.").arg(vo->to));
                    volta->endings().push_back(vo->to);
                    if (vo->bRight) {
                        volta->setVoltaType(Volta::Type::CLOSED);
                    } else {
                        volta->setVoltaType(Volta::Type::OPEN);
                    }
                    volta->setTick(cr1->measure()->tick());
                    volta->setTick2(cr2->measure()->tick() + cr2->measure()->ticks());
                    score->addElement(volta);
                }
            }
            break;
            case CapellaType::TRILL:
            {
                TrillObj* tro = static_cast<TrillObj*>(o);
                ChordRest* cr1 = 0;               // ChordRest where trill line begins
                ChordRest* cr2 = 0;               // ChordRest where trill line ends
                bool res = findChordRests(o, score, track, tick, cr1, cr2, no, cvoice->objects);
                if (res) {
                    if (cr1 == cr2) {
                        LOGD("first and second anchor for trill line identical (tick %d track %d first %p second %p)",
                             tick.ticks(), track, cr1, cr2);
                    } else {
                        Trill* trill = Factory::createTrill(score->dummy());
                        trill->setTrack(track);
                        trill->setTrack2(track);
                        trill->setTick(cr1->tick());
                        trill->setTick2(cr2->tick());
                        if (!(tro->trillSign)) {
                            trill->setTrillType(TrillType::PRALLPRALL_LINE);
                        }
                        score->addElement(trill);
                    }
                }
            }
            break;
            case CapellaType::WEDGE:
            {
                WedgeObj* wdgo = static_cast<WedgeObj*>(o);
                ChordRest* cr1 = 0;               // ChordRest where hairpin begins
                ChordRest* cr2 = 0;               // ChordRest where hairpin ends
                bool res = findChordRests(o, score, track, tick, cr1, cr2, no, cvoice->objects);
                if (res) {
                    if (cr1 == cr2) {
                        LOGD("first and second anchor for hairpin identical (tick %d track %d first %p second %p)",
                             tick.ticks(), track, cr1, cr2);
                    } else {
                        Hairpin* hp = Factory::createHairpin(score->dummy()->segment());
                        if (wdgo->decresc) {
                            hp->setHairpinType(HairpinType::DECRESC_HAIRPIN);
                        } else {
                            hp->setHairpinType(HairpinType::CRESC_HAIRPIN);
                        }
                        hp->setTick(cr1->tick());
                        hp->setTick2(cr2->tick());
                        hp->setTrack(track);
                        hp->setTrack2(track);
                        hp->setAnchor(Spanner::Anchor::SEGMENT);
                        score->addSpanner(hp);
                    }
                }
            }
            break;
            default:
                break;
            }
        }
        Fraction ticks = d->ticks();
        if (d->count) {
            Fraction f = TupletFractionCap(d->count, d->tripartite, d->isProlonging);
            ticks = ticks / f;
        }
        if (no->type() == CapellaNoteObjectType::REST) {
            RestObj* o = static_cast<RestObj*>(no);
            if (o->fullMeasures) {
                Measure* m  = score->getCreateMeasure(tick);
                Fraction ft = m->ticks();
                ticks       = ft * o->fullMeasures;
            }
        }
        tick += ticks;
    }
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
    for (CapBracket cb : bracketList) {
        if (prevInst == currInst && cb.from < staffIdx && staffIdx <= cb.to && cb.curly) {
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
    if (cap->systems.isEmpty()) {
        return;
    }

    score->style().set(Sid::measureSpacing, 1.0);
    score->style().setSpatium(cap->normalLineDist * DPMM);
    score->style().set(Sid::smallStaffMag, cap->smallLineDist / cap->normalLineDist);
    score->style().set(Sid::minSystemDistance, Spatium(8));
    score->style().set(Sid::maxSystemDistance, Spatium(12));

    for (CapSystem* csys : cap->systems) {
        LOGD("System:");
        for (CapStaff* cstaff : csys->staves) {
            CapStaffLayout* cl = cap->staffLayout(cstaff->iLayout);
            LOGD("  Staff layout <%s><%s><%s><%s><%s> %d  barline %d-%d mode %d",
                 qPrintable(cl->descr), qPrintable(cl->name), qPrintable(cl->abbrev),
                 qPrintable(cl->intermediateName), qPrintable(cl->intermediateAbbrev),
                 cstaff->iLayout, cl->barlineFrom, cl->barlineTo, cl->barlineMode);
        }
    }

    //
    // find out the maximum number of staves
    //
    int staves = 0;
    for (CapSystem* csys : cap->systems) {
        staves = qMax(staves, csys->staves.size());
    }
    //
    // check the assumption that every stave should be
    // associated with a CapStaffLayout
    //
    if (staves != cap->staffLayouts().size()) {
        LOGD("Capella: max number of staves != number of staff layouts (%d, %lld)",
             staves, cap->staffLayouts().size());
        staves = qMax(staves, cap->staffLayouts().size());
    }

    // set the initial time signature
    CapStaff* cs = cap->systems[0]->staves[0];
    if (cs->log2Denom <= 7) {
        score->sigmap()->add(0, Fraction(cs->numerator, 1 << cs->log2Denom));
    }

    // create parts and staves
    Staff* bstaff = 0;
    int span = 1;
    int midiPatch = -1;   // the previous MIDI patch (instrument)
    Part* part = 0;
    for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
        CapStaffLayout* cl = cap->staffLayout(staffIdx);
        // LOGD("MIDI staff %d program %d", staffIdx, cl->sound);

        // create a new part if necessary
        if (needPart(midiPatch, cl->sound, staffIdx, cap->brackets)) {
            part = new Part(score);
            score->appendPart(part);
        }
        midiPatch = cl->sound;

        Staff* s = Factory::createStaff(part);
        s->initFromStaffType(0);

        if (cl->bPercussion) {
            part->setMidiProgram(0, 128);
        } else {
            part->setMidiProgram(cl->sound, 0);
        }
        part->setPartName(cl->descr);
        part->setPlainLongName(cl->name);
        part->setPlainShortName(cl->abbrev);

        // ClefType clefType = CapClef::clefType(cl->form, cl->line, cl->oct);
        // s->setClef(0, clefType);
        s->setBarLineSpan(0);
        if (bstaff == 0) {
            bstaff = s;
            span = 0;
        }
        ++span;
        if (cl->barlineMode == 1) {
            bstaff->setBarLineSpan(span != 0);
            bstaff = 0;
        }

        s->staffType(Fraction(0, 1))->setSmall(cl->bSmall);
        Interval interval;
        // guess diatonic transposition from chromatic transposition for the instrument
        int values[23] = { -6, -6, -5, -5, -4, -3, -3, -2, -2, -1, -1, 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 6 };
        interval.diatonic = values[(cl->transp % 12) + 11] + (cl->transp / 12) * 7;
        interval.chromatic = cl->transp;
        s->part()->instrument()->setTranspose(interval);
        score->appendStaff(s);
    }
    if (bstaff) {
        bstaff->setBarLineSpan(span != 0);
    }

    for (CapBracket cb : cap->brackets) {
        LOGD("Bracket %d-%d curly %d", cb.from, cb.to, cb.curly);
        Staff* staff = muse::value(score->staves(), cb.from);
        if (staff == 0) {
            LOGD("bad bracket 'from' value");
            continue;
        }
        staff->setBracketType(0, cb.curly ? BracketType::BRACE : BracketType::NORMAL);
        staff->setBracketSpan(0, cb.to - cb.from + 1);
    }
    MeasureBase* measure = nullptr;
    for (BasicDrawObj* o : cap->backgroundChord->objects) {
        switch (o->type) {
        case CapellaType::SIMPLE_TEXT:
        {
            SimpleTextObj* to = static_cast<SimpleTextObj*>(o);
            TextStyleType tid;
            switch (to->textalign()) {
            case 0:   tid = TextStyleType::LYRICIST;
                break;
            case 1:   tid = TextStyleType::TITLE;
                break;
            case 2:   tid = TextStyleType::COMPOSER;
                break;
            default:  tid = TextStyleType::DEFAULT;
                break;
            }

            if (!measure) {
                measure = Factory::createTitleVBox(score->dummy()->system());
                score->addMeasure(measure, score->measures()->first());
            }

            Text* s = Factory::createText(measure, tid);
            QFont f(to->font());
            s->setItalic(f.italic());
            // s->setUnderline(f.underline());
            // s->setStrike(f.strikeOut());
            s->setBold(f.bold());
            s->setSize(f.pointSizeF());

            QString ss = to->text();
            s->setPlainText(ss);

            measure->add(s);
            // LOGD("page background object type %d (CapellaType::SIMPLE_TEXT) text %s", o->type, qPrintable(ss));
        }
        break;
        default:
            LOGD("page background object type %d", int(o->type));
            break;
        }
    }

    if (cap->topDist) {
        VBox* mb = 0;
        MeasureBaseList* mbl = score->measures();
        if (mbl->size() && mbl->first()->type() == ElementType::VBOX) {
            mb = static_cast<VBox*>(mbl->first());
        } else {
            VBox* vb = Factory::createTitleVBox(score->dummy()->system());
            score->addMeasure(vb, mb);
            mb = vb;
        }
        mb->setBoxHeight(Spatium(cap->topDist));
    }

    Fraction systemTick = Fraction(0, 1);
    for (CapSystem* csys : cap->systems) {
        LOGD("readCapSystem");
        /*
        if (csys->explLeftIndent > 0) {
              HBox* mb = Factory::createHBox(score);
              mb->setTick(systemTick);
              mb->setBoxWidth(Spatium(csys->explLeftIndent));
              score->addMeasure(mb);
              }
        */
        Fraction mtick = Fraction(0, 1);
        for (CapStaff* cstaff : csys->staves) {
            //
            // assumption: layout index is mscore staffIdx
            //    which means that there is a 1:1 relation between layout/staff
            //

            LOGD("  ReadCapStaff %d/%d", cstaff->numerator, 1 << cstaff->log2Denom);
            int staffIdx = cstaff->iLayout;
            for (CapVoice* cvoice : cstaff->voices) {
                Fraction tick = readCapVoice(score, cvoice, staffIdx, systemTick, capxMode);
                if (tick > mtick) {
                    mtick = tick;
                }
            }
        }
        Measure* m = score->tick2measure(mtick - Fraction::fromTicks(1));
        if (m && !m->lineBreak()) {
            LayoutBreak* lb = Factory::createLayoutBreak(m);
            lb->setLayoutBreakType(LayoutBreakType::LINE);
            lb->setTrack(0);
            m->add(lb);
        }
        systemTick = mtick;
    }

    //
    // fill empty measures with rests
    //
    SegmentType st = SegmentType::ChordRest;
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        for (size_t staffIdx = 0; staffIdx < score->staves().size(); ++staffIdx) {
            bool empty = true;
            for (Segment* s = m->first(st); s; s = s->next(st)) {
                if (s->element(static_cast<int>(staffIdx) * VOICES)) {
                    empty = false;
                    break;
                }
            }
            if (empty) {
                if ((m->ticks() == m->timesig())) {
                    Segment* s = m->getSegment(SegmentType::ChordRest, m->tick());
                    Rest* rest = Factory::createRest(s);
                    rest->setDurationType(DurationType::V_MEASURE);
                    rest->setTicks(m->ticks());
                    rest->setTrack(static_cast<int>(staffIdx) * VOICES);
                    s->add(rest);
                } else {
                    auto durList = toDurationList(m->ticks(), true);
                    int tickOffset = 0;
                    for (auto d : durList) {
                        Segment* s = m->getSegment(SegmentType::ChordRest, m->tick() + Fraction::fromTicks(tickOffset));
                        Rest* rest = Factory::createRest(s);
                        rest->setDurationType(d);
                        rest->setTrack(static_cast<int>(staffIdx) * VOICES);
                        s->add(rest);
                        tickOffset += d.ticks().ticks();
                    }
                }
            }
        }
    }
    // score->connectSlurs();
    score->connectTies();
    score->setUpTempoMap();
    score->setPlaylistDirty();
    score->setLayoutAll();
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
    // LOGD("SlurObj nEnd %d nMid %d nDotDist %d nDotWidth %d",
    //        nEnd, nMid, nDotDist, nDotWidth);
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextObj::read()
{
    BasicRectObj::read();
    unsigned size = cap->readUnsigned();
    std::vector<char> vtxt(size + 1);
    char* txt = vtxt.data();
    cap->read(txt, size);
    txt[size] = 0;
    text = QString(txt);
    // LOGD("read textObj len %d <%s>", size, txt);
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
    // LOGD("read SimpletextObj(%f,%f) len %zd <%s>",
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
    // LOGD("LineObj: %f:%f  %f:%f  width %d", pt1.x(), pt1.y(), pt2.x(), pt2.y(), lineWidth);
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
    if (b != 12 && b != 21) {
        LOGD("TransposableObj::read: warning: unknown drawObjectArray size of %d", b);
    }
    variants = cap->readDrawObjectArray();
    IF_ASSERT_FAILED_X(variants.size() == b, QString::asprintf("variants.size %lld, expected %d", variants.size(), b)) {
        throw Capella::Error::BAD_FORMAT;
    }
    /*int nRefNote =*/ cap->readInt();
}

//---------------------------------------------------------
//   MetafileObj::read
//---------------------------------------------------------

void MetafileObj::read()
{
    BasicRectObj::read();
    unsigned size = cap->readUnsigned();
    std::vector<char> vEnhMetaFileBits(size);
    char* enhMetaFileBits = vEnhMetaFileBits.data();
    cap->read(enhMetaFileBits, size);
    // LOGD("MetaFileObj::read %d bytes", size);
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
    for (unsigned i = 0; i < nPoints; ++i) {
        cap->readPoint();
    }

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
    case 1: break;         // Einlinienzeile
    case 2: break;         // Standard (5 Linien)
    default: {
        IF_ASSERT_FAILED(b == 0) {
            throw Capella::Error::BAD_FORMAT;
        }
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

    unsigned char f = cap->readByte();
    bLeft      = (f & 1) != 0;   // links abgeknickt
    bRight     = (f & 2) != 0;   // rechts abgeknickt
    bDotted    = (f & 4) != 0;
    allNumbers = (f & 8) != 0;

    unsigned char numbers = cap->readByte();
    from = numbers & 0x0F;
    to = (numbers >> 4) & 0x0F;
    LOGD("VoltaObj::read x0 %d x1 %d y %d bLeft %d bRight %d bDotted %d allNumbers %d from %d to %d",
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
    strings = cap->readDWord();     // 8 Saiten in 8 Halbbytes
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
    int n = readUnsigned();         // draw obj array

    // LOGD("readDrawObjectArray %d elements", n);
    for (int i = 0; i < n; ++i) {
        CapellaType type = CapellaType(readByte());

        // LOGD("   readDrawObject %d of %d, type %d", i, n, type);
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
            IF_ASSERT_FAILED_X(false, QString::asprintf("readDrawObjectArray unsupported type %d", int(type))) {
                throw Capella::Error::BAD_FORMAT;
            }
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
    modeX       = cap->readByte();        // anchor mode
    modeY       = cap->readByte();
    distY       = cap->readByte();
    flags       = cap->readByte();
    nRefNote    = cap->readInt();
    short range = cap->readWord();
    nNotes      = range & 0x0fff;
    background  = range & 0x1000;
    pageRange   = (range >> 13) & 0x7;
    LOGD("BasicDrawObj::read modeX %d modeY %d distY %d flags %d nRefNote %d nNotes %d background %d pageRange %d",
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
    IF_ASSERT_FAILED(!(b & 0x80)) {
        throw Capella::Error::BAD_FORMAT;
    }
    nDots      = b & 0x03;
    noDuration = b & 0x04;
    postGrace  = b & 0x08;
    bSmall     = b & 0x10;
    invisible  = b & 0x20;
    notBlack   = b & 0x40;

    color = notBlack ? cap->readColor() : Qt::black;

    unsigned char c = cap->readByte();
    IF_ASSERT_FAILED(!(c & 0x80)) {
        throw Capella::Error::BAD_FORMAT;
    }
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
        if (tuplet & 0xc0) {
            LOGD("bad tuplet value 0x%02x", tuplet);
        }
    }
    if (c & 0x40) {
        objects = cap->readDrawObjectArray();
    }
    LOGD("DurationObj ndots %d nodur %d postgr %d bsm %d inv %d notbl %d t %d hsh %d cnt %d trp %d ispro %d",
         nDots, noDuration, postGrace, bSmall, invisible, notBlack, int(t), horizontalShift, count, tripartite, isProlonging
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
    if (b & 0xf8) {
        ASSERT_X(QString::asprintf("RestObj: res. bits 0x%02x", b));
    }
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
    beamMode = CapBeamMode::AUTO;
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
    beamMode      = (flags & 0x01) ? CapBeamMode(cap->readByte()) : CapBeamMode::AUTO;
    notationStave = (flags & 0x02) ? cap->readChar() : 0;
    IF_ASSERT_FAILED(notationStave >= -1 && notationStave <= 1) {
        throw Capella::Error::BAD_FORMAT;
    }

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
                if (b & 8) {
                    v.verseNumber = cap->readQString();
                }
                if (b & 16) {
                    v.text = cap->readQString();
                }
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
        n.pitch          = (signed char)c;
        if (bit7 != bit6) {
            n.explAlteration = 2;
            n.pitch ^= 0x80;
        }
        unsigned char b = cap->readByte();
        n.headType      = b & 7;
        if (n.headType == 6) {
            n.headType = 0;
            n.headGroup = int(NoteHeadGroup::HEAD_CROSS);
        } else {
            n.headGroup = int(NoteHeadGroup::HEAD_NORMAL);
        }
        n.alteration    = ((b >> 3) & 7) - 2;      // -2 -- +2
        if (b & 0x40) {
            n.explAlteration = 1;
        }
        n.silent = b & 0x80;
        LOGD("ChordObj::read() note pitch %d explAlt %d head group %d %d alt %d silent %d",
             n.pitch, n.explAlteration, n.headType, n.headGroup, n.alteration, n.silent);
        notes.append(n);
    }
}

//---------------------------------------------------------
//    read
//    return false on error
//---------------------------------------------------------

bool Capella::read(void* p, qint64 len)
{
    if (len == 0) {
        return true;
    }
    qint64 rv = f->read((char*)p, len);
    if (rv != len) {
        return false;
    }
    curPos += len;
    return true;
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
    } else if (c == 255) {
        unsigned s;
        read(&s, 4);
        return s;
    } else {
        return c;
    }
}

//---------------------------------------------------------
//   readInt
//---------------------------------------------------------

int Capella::readInt()
{
    signed char c;
    read(&c, 1);
    if (c == -128) {
        short s;
        read(&s, 2);
        return s;
    } else if (c == 127) {
        int s;
        read(&s, 4);
        return s;
    } else {
        return c;
    }
}

//---------------------------------------------------------
//   readString -- read Capella string into newly allocated char buffer
//   note that no carriage return / newline interpretation is done
//---------------------------------------------------------

char* Capella::readString()
{
    unsigned len = readUnsigned();
    char* buffer = new char[static_cast<size_t>(len) + 1];
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
    char* buffer = readString();                 // read Capella string
    QString res = QString::fromLatin1(buffer);   // and copy into QString
    res = res.remove(QChar('\r'));               // remove the \r
    delete [] buffer;                            // delete memory allocated by readString
    return res;
}

//---------------------------------------------------------
//   readColor
//---------------------------------------------------------

QColor Capella::readColor()
{
    static const int colors[] = {
        0x000000,     // schwarz
        0x000080,     // dunkelrot
        0x008000,     // dunkelgrün
        0x008080,     // ocker
        0x800000,     // dunkelblau
        0x800080,     // purpurrot
        0x808000,     // blaugün
        0x808080,     // grau
        0xC0C0C0,     // hellgrau
        0x0000FF,     // rot
        0x00FF00,     // grün
        0x00FFFF,     // gelb
        0xFF0000,     // blau
        0xFF00FF,     // lila
        0xFFFF00,     // aquamarin
        0xFFFFFF      // weiß
    };

    QColor c;
    unsigned char b = readByte();
    if (b >= 16) {
        IF_ASSERT_FAILED(b == 255) {
            throw Capella::Error::BAD_FORMAT;
        }
        int r = readByte();
        int g = readByte();
        int bi = readByte();
        c = QColor(r, g, bi);
    } else {
        c = QColor(colors[b]);
    }
    return c;
}

//---------------------------------------------------------
//   readFont
//---------------------------------------------------------

QFont Capella::readFont()
{
    unsigned index = readUnsigned();
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
        QString face             = readQString();

        LOGD("Font <%s> size %d, weight %d", qPrintable(face), lfHeight, lfWeight);
        QFont font(face);
        font.setPointSizeF(lfHeight / 1000.0);
        font.setItalic(lfItalic);
        font.setStrikeOut(lfStrikeOut);
        font.setUnderline(lfUnderline);

        switch (lfWeight) {
        case 700:  font.setWeight(QFont::Bold);
            break;
        case 400:  font.setWeight(QFont::Normal);
            break;
        case 0:    font.setWeight(QFont::Light);
            break;
        }
        fonts.append(font);
        return font;
    }
    index -= 1;
    if (index >= fonts.size()) {
        LOGD("illegal font index %u (max %lld)", index, fonts.size() - 1);
        return QFont();
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
    case 1:     break;              // one line
    case 2:     break;              // five lines
    default:
    {
        char lines[11];
        f->read(lines, 11);
        curPos += 11;
    }
    break;
    }
    LOGD("StaffLayout %d: barlineMode %d noteLines %d", idx, sl->barlineMode, sl->noteLines);

    sl->bSmall      = readByte();
    LOGD("staff size small %d", sl->bSmall);

    sl->topDist      = readInt();
    sl->btmDist      = readInt();
    sl->groupDist    = readInt();
    sl->barlineFrom = readByte();
    sl->barlineTo   = readByte();
    // LOGD("topDist %d btmDist %d groupDist %d barlineFrom %d barlineTo %d",
    //        sl->topDist, sl->btmDist, sl->groupDist, sl->barlineFrom, sl->barlineTo);

    unsigned char clef = readByte();
    sl->form = Form(clef & 7);
    sl->line = ClefLine((clef >> 3) & 7);
    sl->oct  = Oct((clef >> 6));
    LOGD("   clef %x  form %d, line %d, oct %d", clef, int(sl->form), int(sl->line), int(sl->oct));

    // Schlagzeuginformation
    unsigned char b   = readByte();
    sl->bPercussion  = b & 1;      // Schlagzeugkanal verwenden
    sl->bSoundMapIn  = b & 2;
    sl->bSoundMapOut = b & 4;
    if (sl->bSoundMapIn) {        // Umleitungstabelle für Eingabe vom Keyboard
        uchar iMin = readByte();
        uchar n    = readByte();
        IF_ASSERT_FAILED(n > 0 && iMin + n <= 128) {
            throw Capella::Error::BAD_FORMAT;
        }
        f->read(sl->soundMapIn, n);
        curPos += n;
    }
    if (sl->bSoundMapOut) {       // Umleitungstabelle für das Vorspielen
        unsigned char iMin = readByte();
        unsigned char n    = readByte();
        IF_ASSERT_FAILED(n > 0 && iMin + n <= 128) {
            throw Capella::Error::BAD_FORMAT;
        }
        f->read(sl->soundMapOut, n);
        curPos += n;
    }
    sl->sound  = readInt();
    sl->volume = readInt();
    sl->transp = readInt();
    LOGD("   sound %d vol %d transp %d", sl->sound, sl->volume, sl->transp);

    sl->descr              = readQString();
    sl->name               = readQString();
    sl->abbrev             = readQString();
    sl->intermediateName   = readQString();
    sl->intermediateAbbrev = readQString();
    LOGD("   descr <%s> name <%s>  abbrev <%s> iname <%s> iabbrev <%s>",
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
    LOGD("Capella::readLayout(): smallLineDist %g normalLineDist %g", smallLineDist, normalLineDist);

    topDist        = readInt();
    interDist      = readInt();
    LOGD("Capella::readLayout(): topDist %d", topDist);

    txtAlign   = readByte();      // Stimmenbezeichnungen 0=links, 1=zentriert, 2=rechts
    adjustVert = readByte();      // 0=nein, 1=außer letzte Seite, 3=alle Seiten

    unsigned char b  = readByte();
    IF_ASSERT_FAILED(!(b & 0xFC)) {   // bits 2...7 reserviert
        throw Capella::Error::BAD_FORMAT;
    }
    redundantKeys    = b & 1;
    modernDoubleNote = b & 2;

    bSystemSeparators = readByte();
    nUnnamed           = readInt();

    namesFont = readFont();

    // Musterzeilen
    unsigned nStaveLayouts = readUnsigned();

    // LOGD("%d staves", nStaveLayouts);

    for (unsigned iStave = 0; iStave < nStaveLayouts; iStave++) {
        CapStaffLayout* sl = new CapStaffLayout;
        readStaveLayout(sl, iStave);
        _staffLayouts.append(sl);
    }

    // system brackets:
    unsigned n = readUnsigned();    // number of brackets
    for (unsigned int i = 0; i < n; i++) {
        CapBracket cb;
        cb.from   = readInt();
        cb.to     = readInt();
        cb.curly = readByte();
        // LOGD("Bracket%d %d-%d curly %d", i, b.from, b.to, b.curly);
        brackets.append(cb);
    }
    // LOGD("Capella::readLayout(): done");
}

//---------------------------------------------------------
//   readExtra
//---------------------------------------------------------

void Capella::readExtra()
{
    uchar n = readByte();
    if (n) {
        LOGD("Capella::readExtra(%d)", n);
        for (int i = 0; i < n; ++i) {
            readByte();
        }
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
    LOGD("Clef::read form %d line %d oct %d", int(form), int(line), int(oct));
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
    case int(Form::G) + (int(ClefLine::L2) << 3) + (int(Oct::OCT_ALTA) << 5):  return ClefType::G8_VA;
    case int(Form::G) + (int(ClefLine::L2) << 3) + (int(Oct::OCT_BASSA) << 5): return ClefType::G8_VB;

    case int(Form::C) + (int(ClefLine::L1) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::C1;
    case int(Form::C) + (int(ClefLine::L2) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::C2;
    case int(Form::C) + (int(ClefLine::L3) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::C3;
    case int(Form::C) + (int(ClefLine::L4) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::C4;
    case int(Form::C) + (int(ClefLine::L4) << 3) + (int(Oct::OCT_BASSA) << 5): return ClefType::C4_8VB;
    case int(Form::C) + (int(ClefLine::L5) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::C5;

    case int(Form::F) + (int(ClefLine::L4) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::F;
    case int(Form::F) + (int(ClefLine::L4) << 3) + (int(Oct::OCT_BASSA) << 5): return ClefType::F8_VB;
    case int(Form::F) + (int(ClefLine::L3) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::F_B;
    case int(Form::F) + (int(ClefLine::L5) << 3) + (int(Oct::OCT_NULL) << 5):  return ClefType::F_C;

    default:
        if (form == Form::FORM_NULL) {
            return ClefType::INVALID;
        }
        LOGD("unknown clef %d %d %d", int(form), int(line), int(oct));
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
    // LOGD("         Key %d", signature);
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
    LOGD("   Meter %d/%d allaBreve %d", numerator, log2Denom, allaBreve);
    if (log2Denom > 7 || log2Denom < 0) {
        LOGD("   illegal fraction");
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
    int type = b & 0x0f;
    if (type == 0) {
        _type = BarLineType::NORMAL;
    } else if (type == 1) {
        _type = BarLineType::DOUBLE;
    } else if (type == 2) {
        _type = BarLineType::END;
    } else if (type == 3) {
        _type = BarLineType::END_REPEAT;
    } else if (type == 4) {
        _type = BarLineType::START_REPEAT;
    } else if (type == 5) {
        _type = BarLineType::END_START_REPEAT;
    } else if (type == 6) {
        _type = BarLineType::BROKEN;
    } else {
        _type = BarLineType::NORMAL;    // default
    }
    _barMode = b >> 4;           // 0 = auto, 1 = nur Zeilen, 2 = durchgezogen
    IF_ASSERT_FAILED(_barMode <= 2) {
        throw Capella::Error::BAD_FORMAT;
    }

    LOGD("         Expl.Barline type %d mode %d", int(_type), _barMode);
}

//---------------------------------------------------------
//   readVoice
//---------------------------------------------------------

void Capella::readVoice(CapStaff* cs, int idx)
{
    LOGD("      readVoice %d", idx);

    if (readChar() != 'C') {
        throw Capella::Error::BAD_VOICE_SIG;
    }

    CapVoice* v   = new CapVoice;
    v->voiceNo    = idx;
    v->y0Lyrics   = readByte();
    v->dyLyrics   = readByte();
    v->lyricsFont = readFont();
    v->stemDir    = readByte();
    readExtra();

    unsigned nNoteObjs = readUnsigned();            // Notenobjekte
    for (unsigned i = 0; i < nNoteObjs; i++) {
        QColor color       = Qt::black;
        uchar type = readByte();
        // LOGD("         Voice %d read object idx %d(%d) type %d", idx,  i, nNoteObjs, type);
        readExtra();
        if ((type != uchar(CapellaNoteObjectType::REST)) && (type != uchar(CapellaNoteObjectType::CHORD))
            && (type != uchar(CapellaNoteObjectType::PAGE_BKGR))) {
            color = readColor();
        }

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
            LOGD("append Expl Barline==========");
            v->objects.append(bl);
        }
        break;
        default:
            ASSERT_X(QString::asprintf("bad voice type %d", type));
        }
    }
    cs->voices.append(v);
}

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

void Capella::readStaff(CapSystem* system)
{
    if (readChar() != 'V') {
        throw Capella::Error::BAD_STAFF_SIG;
    }

    CapStaff* staff = new CapStaff;
    //Meter
    staff->numerator = readByte();
    uchar d          = readByte();
    staff->log2Denom = (d & 0x7f) - 1;
    staff->allaBreve = d & 0x80;
    LOGD("   CapStaff meter %d/%d allaBreve %d", staff->numerator, staff->log2Denom, staff->allaBreve);
    if (staff->log2Denom > 7 || staff->log2Denom < 0) {
        LOGD("   illegal fraction");
        staff->log2Denom = 2;
        staff->numerator = 4;
    }

    staff->iLayout   = readByte();
    staff->topDistX  = readInt();
    staff->btmDistX  = readInt();
    staff->color     = readColor();
    readExtra();

    LOGD("      Staff iLayout %d", staff->iLayout);
    // Stimmen
    unsigned nVoices = readUnsigned();
    for (unsigned i = 0; i < nVoices; i++) {
        readVoice(staff, i);
    }
    system->staves.append(staff);
}

//---------------------------------------------------------
//   readSystem
//---------------------------------------------------------

void Capella::readSystem()
{
    if (readChar() != 'S') {
        throw Capella::Error::BAD_SYSTEM_SIG;
    }

    CapSystem* s = new CapSystem;
    s->nAddBarCount   = readInt();
    s->bBarCountReset = readByte();
    s->explLeftIndent = readByte();

    s->beamMode = CapBeamMode(readByte());
    s->tempo    = readUnsigned();
    s->color    = readColor();
    readExtra();

    unsigned char b  = readByte();
    s->bJustified    = b & 2;
    s->bPageBreak    = (b & 4) != 0;
    s->instrNotation = (b >> 3) & 7;

    unsigned nStaves = readUnsigned();
    for (unsigned i = 0; i < nStaves; i++) {
        readStaff(s);
    }
    systems.append(s);
}

//---------------------------------------------------------
//   toTicks
//---------------------------------------------------------

Fraction BasicDurationalObj::ticks() const
{
    if (noDuration) {
        return Fraction(0, 1);
    }
    Fraction len = { 0, 1 };
    switch (t) {
    case TIMESTEP::D1:          len = Fraction(1, 1);
        break;
    case TIMESTEP::D2:          len = Fraction(1, 2);
        break;
    case TIMESTEP::D4:          len = Fraction(1, 4);
        break;
    case TIMESTEP::D8:          len = Fraction(1, 8);
        break;
    case TIMESTEP::D16:         len = Fraction(1, 16);
        break;
    case TIMESTEP::D32:         len = Fraction(1, 32);
        break;
    case TIMESTEP::D64:         len = Fraction(1, 64);
        break;
    case TIMESTEP::D128:        len = Fraction(1, 128);
        break;
    case TIMESTEP::D256:        len = Fraction(1, 256);
        break;
    case TIMESTEP::D_BREVE:     len = Fraction(2, 1);
        break;
    default:
        LOGD("BasicDurationalObj::ticks: illegal duration value %d", int(t));
        break;
    }
    Fraction slen = len;
    int dots = nDots;
    while (dots--) {
        slen /= Fraction(2, 1);
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
    if (memcmp(signature, "cap3-v:", 7) != 0) {
        throw Capella::Error::BAD_SIG;
    }

    // LOGD("read Capella file signature <%s>", signature);

    // TODO: test for signature[7] = a-z

    author   = readString();
    keywords = readString();
    comment  = readString();

    // LOGD("author <%s> keywords <%s> comment <%s>", author, keywords, comment);

    nRel   = readUnsigned();              // 75
    nAbs   = readUnsigned();              // 16
    unsigned char b   = readByte();
    bUseRealSize      = b & 1;
    bAllowCompression = b & 2;
    bPrintLandscape   = b & 16;

    // LOGD("  nRel %d  nAbs %d useRealSize %d compression %d", nRel, nAbs, bUseRealSize, bAllowCompression);

    readLayout();

    beamRelMin0 = readByte();          // basic setup for beam slope
    beamRelMin1 = readByte();
    beamRelMax0 = readByte();
    beamRelMax1 = readByte();

    readExtra();

    readDrawObjectArray();                  // Galerie (gesammelte Grafikobjekte)

    unsigned n = readUnsigned();
    if (n) {
        LOGD("Gallery objects");
    }
    for (unsigned int i = 0; i < n; ++i) {
        /*char* s =*/
        readString();                       // Namen der Galerie-Objekte
        // LOGD("Galerie: <%s>", s);
    }

    // LOGD("read backgroundChord");
    backgroundChord = new ChordObj(this);
    backgroundChord->read();                // contains graphic objects on the page background
    // LOGD("read backgroundChord done");
    bShowBarCount    = readByte();          // Taktnumerierung zeigen
    barNumberFrame   = readByte();          // 0=kein, 1=Rechteck, 2=Ellipse
    nBarDistX        = readByte();
    nBarDistY        = readByte();
    QFont barNumFont = readFont();
    nFirstPage       = readUnsigned();      // Versatz fuer Seitenzaehlung
    leftPageMargins  = readUnsigned();      // Seitenraender
    topPageMargins   = readUnsigned();
    rightPageMargins = readUnsigned();
    btmPageMargins   = readUnsigned();

    unsigned nSystems  = readUnsigned();
    for (unsigned i = 0; i < nSystems; i++) {
        readSystem();
    }
    char esig[4];
    read(esig, 4);
    if (memcmp(esig, "\0\0\0\0", 4) != 0) {
        throw Capella::Error::BAD_SIG;
    }
}

//---------------------------------------------------------
//   importCapella
//---------------------------------------------------------

Err importCapella(MasterScore* score, const QString& name)
{
    QFile fp(name);
    if (!fp.exists()) {
        return Err::FileNotFound;
    }
    if (!fp.open(QIODevice::ReadOnly)) {
        return Err::FileOpenError;
    }

    Capella cf;
    try {
        cf.read(&fp);
    }
    catch (Capella::Error errNo) {
        if (!MScore::noGui) {
            MessageBox(score->iocContext()).warning(muse::trc("iex_capella", "Import Capella"),
                                                    muse::qtrc("iex_capella", "Import failed: %1").arg(cf.error(errNo)).toStdString(),
                                                    { MessageBox::Ok });
        }
        fp.close();
        // avoid another error message box
        return Err::NoError;
    }
    fp.close();
    convertCapella(score, &cf, false);
    return Err::NoError;
}
}
