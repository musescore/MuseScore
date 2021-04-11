//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include "score.h"
#include "slur.h"
#include "measure.h"
#include "tuplet.h"
#include "chordrest.h"
#include "rest.h"
#include "segment.h"
#include "staff.h"
#include "keysig.h"
#include "clef.h"
#include "utils.h"

namespace Ms {
//---------------------------------------------------------
//   checkSlurs
//    helper routine to check for sanity slurs
//---------------------------------------------------------

void Score::checkSlurs()
{
#if 0 //TODO1
    foreach (Element* e, _gel) {
        if (e->type() != SLUR) {
            continue;
        }
        Slur* s = (Slur*)e;
        Element* n1 = s->startElement();
        Element* n2 = s->endElement();
        if (n1 == 0 || n2 == 0 || n1 == n2) {
            qDebug("unconnected slur: removing");
            if (n1) {
                ((ChordRest*)n1)->removeSlurFor(s);
                ((ChordRest*)n1)->removeSlurBack(s);
            }
            if (n1 == 0) {
                qDebug("  start at %d(%d) not found", s->tick(), s->track());
            }
            if (n2 == 0) {
                qDebug("  end at %d(%d) not found", s->tick2(), s->track2());
            }
            if ((n1 || n2) && (n1 == n2)) {
                qDebug("  start == end");
            }
            int idx = _gel.indexOf(s);
            _gel.removeAt(idx);
        }
    }
#endif
}

//---------------------------------------------------------
//   checkScore
//---------------------------------------------------------

void Score::checkScore()
{
    if (!firstMeasure()) {
        return;
    }
#if 0
    for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
        m->segments()->check();
    }
#endif
    for (Segment* s = firstMeasure()->first(); s;) {
        Segment* ns = s->next1();

        if (s->segmentType() & (SegmentType::ChordRest)) {
            bool empty = true;
            foreach (Element* e, s->elist()) {
                if (e) {
                    empty = false;
                    break;
                }
            }
            if (empty) {
                // Measure* m = s->measure();
                qDebug("checkScore: remove empty ChordRest segment");
//                        m->remove(s);
            }
        }
        s = ns;
    }

    checkSlurs();

    ChordRest* lcr = 0;
    for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
        int track = staffIdx * VOICES;
        Fraction tick  = Fraction(0,1);
        Staff* st = staff(staffIdx);
        for (Segment* s = firstMeasure()->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            ChordRest* cr = toChordRest(s->element(track));
            if (!cr) {
                continue;
            }
            if (s->tick() != tick) {
                if (lcr) {
                    Fraction timeStretch = st->timeStretch(lcr->tick());
                    Fraction f = cr->globalTicks() * timeStretch;
                    qDebug("Chord/Rest gap at tick %d(%s+%d)-%d(%s) staffIdx %d measure %d (len = %d)",
                           tick.ticks(), lcr->name(), f.ticks(),
                           s->tick().ticks(), cr->name(), staffIdx, cr->measure()->no(),
                           (cr->tick() - tick).ticks());
                } else {
                    qDebug("Chord/Rest gap at tick %d-%d(%s) staffIdx %d measure %d (len = %d)",
                           tick.ticks(),
                           s->tick().ticks(), cr->name(), staffIdx, cr->measure()->no(),
                           (cr->tick() - tick).ticks());
                }
#if 0
                if (cr->tick() > tick) {
                    int ttick = tick;
                    int ticks = cr->tick() - tick;

                    Fraction f = Fraction::fromTicks(ticks) / st->timeStretch(ttick);
                    qDebug("  insert %d/%d", f.numerator(), f.denominator());

                    while (ticks > 0) {
                        Measure* m = tick2measure(ttick);
                        int len    = ticks;
                        // split notes on measure boundary
                        if ((ttick + len) > m->tick() + m->ticks()) {
                            len = m->tick() + m->ticks() - ttick;
                        }
                        Fraction timeStretch = st->timeStretch(ttick);
                        Fraction ff          = Fraction::fromTicks(len);
                        qDebug("    - insert %d/%d", ff.numerator(), ff.denominator());
                        if (ff.numerator() == 0) {
                            break;
                        }
                        Fraction fff = ff / timeStretch;

                        for (const Duration& d, toDurationList(fff, true)) {
                            Rest* rest = new Rest(this);
                            rest->setDurationType(d);
                            rest->setDuration(d.fraction());
                            rest->setColor(Qt::red);
                            qDebug("    -   Rest %d/%d", d.fraction().numerator(), d.fraction().denominator());
                            rest->setTrack(track);
                            Segment* s = m->getSegment(rest, ttick);
                            s->add(rest);
                            ttick += (d.fraction() * timeStretch).ticks();
                        }
                        ticks -= len;
                    }
                }
#endif
                tick = s->tick();
            }
            Fraction timeStretch = st->timeStretch(tick);
            Fraction f = cr->globalTicks() * timeStretch;
//                  qDebug("%s %d + %d = %d", cr->name(), tick, f.ticks(), tick + f.ticks());
            tick      += f;
            lcr        = cr;
        }
    }
}

//---------------------------------------------------------
//   sanityCheck - Simple check for score
///    Check that voice 1 is complete
///    Check that voices > 1 contains less than measure duration
//---------------------------------------------------------

bool Score::sanityCheck(const QString& name)
{
    bool result = true;
    int mNumber = 1;
    QString error;
    for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
        Fraction mLen = m->ticks();
        int endStaff  = staves().size();
        for (int staffIdx = 0; staffIdx < endStaff; ++staffIdx) {
            Rest* fmrest0 = 0;            // full measure rest in voice 0
            Fraction voices[VOICES];
#ifndef NDEBUG
            m->setCorrupted(staffIdx, false);
#endif
            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                for (int v = 0; v < VOICES; ++v) {
                    ChordRest* cr = toChordRest(s->element(staffIdx * VOICES + v));
                    if (cr == 0) {
                        continue;
                    }
                    voices[v] += cr->actualTicks();
                    if (v == 0 && cr->isRest()) {
                        Rest* r = toRest(cr);
                        if (r->durationType().isMeasure()) {
                            fmrest0 = r;
                        }
                    }
                }
            }
            if (voices[0] != mLen) {
                QString msg = QObject::tr("Measure %1, staff %2 incomplete. Expected: %3; Found: %4").arg(mNumber).arg(staffIdx + 1).arg(
                    mLen.print(), voices[0].print());
                qDebug() << msg;
                error += QString("%1\n").arg(msg);
#ifndef NDEBUG
                m->setCorrupted(staffIdx, true);
#endif
                result = false;
                // try to fix a bad full measure rest
                if (fmrest0) {
                    // fmrest0->setDuration(mLen * fmrest0->staff()->timeStretch(fmrest0->tick()));
                    fmrest0->setTicks(mLen);
                    if (fmrest0->actualTicks() != mLen) {
                        fprintf(stderr,"whoo???\n");
                    }
                }
            }
            for (int v = 1; v < VOICES; ++v) {
                if (voices[v] > mLen) {
                    QString msg = QObject::tr("Measure %1, staff %2, voice %3 too long. Expected: %4; Found: %5").arg(mNumber).arg(
                        staffIdx + 1).arg(v + 1).arg(mLen.print(), voices[v].print());
                    qDebug() << msg;
                    error += QString("%1\n").arg(msg);
#ifndef NDEBUG
                    m->setCorrupted(staffIdx, true);
#endif
                    result = false;
                }
            }
        }
        mNumber++;
    }
    if (!name.isEmpty()) {
        QJsonObject json;
        if (result) {
            json["result"] = 0;
        } else {
            json["result"] = 1;
            json["error"] = error.trimmed().replace("\n", "\\n");
        }
        QJsonDocument jsonDoc(json);
        QFile fp(name);
        if (!fp.open(QIODevice::WriteOnly)) {
            qDebug("Open <%s> failed", qPrintable(name));
            return false;
        }
        fp.write(jsonDoc.toJson(QJsonDocument::Compact));
        fp.close();
    } else {
        MScore::lastError = error;
    }
    return result;
}

//---------------------------------------------------------
//   checkKeys
///    check that key map is in sync with actual keys
//---------------------------------------------------------

bool Score::checkKeys()
{
    bool rc = true;
    for (int i = 0; i < nstaves(); ++i) {
        Key k = staff(i)->key(Fraction(0,1));
        for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            Segment* s = m->findSegment(SegmentType::KeySig, m->tick());
            if (s) {
                Element* element = s->element(i * VOICES);
                if (element) {
                    k = toKeySig(element)->key();
                }
            }
            if (staff(i)->key(m->tick()) != k) {
                qDebug("measure %d (tick %d) : key %d, map %d", m->no(), m->tick().ticks(), int(k),
                       int(staff(i)->key(m->tick())));
                rc = false;
            }
        }
    }
    return rc;
}

//---------------------------------------------------------
//   checkClefs
///    check that clef map is in sync with actual clefs
//---------------------------------------------------------

bool Score::checkClefs()
{
    bool rc = true;
//TODO:ws   what about clefs not at measure start?

#if 0
    int track = 0;
    for (Staff* staff : _staves) {
        ClefType clefType = staff->clef(0);
        Measure* prevMeasure  = 0;

        for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            if (prevMeasure) {
                Segment* segment = prevMeasure->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef, 0);
                if (segment) {
                    Element* e = segment->element(track);
                    if (e) {
                        clefType = toClef(e)->clefType();
                    }
                }
            }
            ClefType mapClefType = staff->clef(m->tick());
            if (mapClefType != clefType) {
                qDebug("measure %d (tick %d) : clef %d, map %d", m->no(), m->tick(), int(clefType), int(mapClefType));
                rc = false;
            }
            prevMeasure = m;
        }
        track += VOICES;
    }
#endif
    return rc;
}

//---------------------------------------------------------
//   fillGap
//---------------------------------------------------------

void Measure::fillGap(const Fraction& pos, const Fraction& len, int track, const Fraction& stretch, bool useGapRests)
{
    qDebug("measure %6d pos %d, len %d/%d, stretch %d/%d track %d",
           tick().ticks(),
           pos.ticks(),
           len.numerator(), len.denominator(),
           stretch.numerator(), stretch.denominator(),
           track);
    TDuration d;
    d.setVal(len.ticks());
    if (d.isValid()) {
        Rest* rest = new Rest(score());
        rest->setTicks(len);
        rest->setDurationType(d);
        rest->setTrack(track);
        rest->setGap(useGapRests);
        score()->undoAddCR(rest, this, (pos / stretch) + tick());
    }
}

//---------------------------------------------------------
//   checkMeasure
//    after opening / paste and every read operation
//    this method checks for gaps and fills them
//    with invisible rests
//---------------------------------------------------------

void Measure::checkMeasure(int staffIdx, bool useGapRests)
{
    if (isMMRest()) {
        return;
    }

    int strack       = staffIdx * VOICES;
    int dtrack       = strack + (hasVoices(staffIdx) ? VOICES : 1);
    Fraction stretch = score()->staff(staffIdx)->timeStretch(tick());
    Fraction f       = ticks() * stretch;

    for (int track = strack; track < dtrack; track++) {
        Fraction expectedPos = Fraction(0,1);
        Fraction currentPos  = Fraction(0,1);

        for (Segment* seg = first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
            Element* e = seg->element(track);
            if (!e) {
                continue;
            }

            ChordRest* cr = toChordRest(e);
            currentPos    = seg->rtick() * stretch;

            if (currentPos < expectedPos) {
                qDebug("in measure overrun %6d at %d-%d track %d", tick().ticks(),
                       (currentPos / stretch).ticks(), (expectedPos / stretch).ticks(), track);
                break;
            } else if (currentPos > expectedPos) {
                qDebug("in measure underrun %6d at %d-%d track %d", tick().ticks(),
                       (currentPos / stretch).ticks(), (expectedPos / stretch).ticks(), track);
                fillGap(expectedPos, currentPos - expectedPos, track, stretch, useGapRests);
            }

            DurationElement* de = cr;
            Tuplet* tuplet = cr->topTuplet();
            if (tuplet) {
                seg = skipTuplet(tuplet);
                de  = tuplet;
            }
            expectedPos = currentPos + de->ticks();
        }
        if (f > expectedPos) {
            // don't fill empty voices
            if (expectedPos.isNotZero()) {
                fillGap(expectedPos, f - expectedPos, track, stretch);
            }
        } else if (f < expectedPos) {
            qDebug("measure overrun %6d, %d > %d, track %d", tick().ticks(), expectedPos.ticks(), f.ticks(), track);
        }
    }
}
}
