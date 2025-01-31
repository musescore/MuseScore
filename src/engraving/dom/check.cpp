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

#include "chordrest.h"
#include "durationtype.h"
#include "factory.h"
#include "keysig.h"
#include "masterscore.h"
#include "measure.h"
#include "rest.h"
#include "segment.h"
#include "sig.h"
#include "staff.h"
#include "tuplet.h"
#include "utils.h"

#include "engravingerrors.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   checkScore
//---------------------------------------------------------

void Score::checkScore()
{
    if (!firstMeasure()) {
        return;
    }

    for (Segment* s = firstMeasure()->first(); s;) {
        Segment* ns = s->next1();

        if (s->segmentType() & (SegmentType::ChordRest)) {
            bool empty = true;
            for (EngravingItem* e : s->elist()) {
                if (e) {
                    empty = false;
                    break;
                }
            }
            if (empty) {
                LOGD("checkScore: remove empty ChordRest segment");
            }
        }
        s = ns;
    }

    ChordRest* lcr = 0;
    for (size_t staffIdx = 0; staffIdx < m_staves.size(); ++staffIdx) {
        size_t track = staffIdx * VOICES;
        Fraction tick  = Fraction(0, 1);
        Staff* st = staff(staffIdx);
        for (Segment* s = firstMeasure()->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            ChordRest* cr = toChordRest(s->element(static_cast<int>(track)));
            if (!cr) {
                continue;
            }
            if (s->tick() != tick) {
                if (lcr) {
                    Fraction timeStretch = st->timeStretch(lcr->tick());
                    Fraction f = cr->globalTicks() * timeStretch;
                    LOGD() << "Chord/Rest gap at tick " << tick.ticks() << "(" << lcr->typeName() << "+" << f.ticks() << ")-"
                           << s->tick().ticks() << "(" << cr->typeName() << ") staffIdx " << staffIdx << " measure " << cr->measure()->no()
                           << " (len = " << (cr->tick() - tick).ticks() << ")";
                } else {
                    LOGD() << "Chord/Rest gap at tick " << tick.ticks() << "-" << s->tick().ticks() << "(" << cr->typeName() << ") "
                           << "staffIdx " << staffIdx << " measure " << cr->measure()->no()
                           << "  (len = " << (cr->tick() - tick).ticks() << ")";
                }
                tick = s->tick();
            }

            Fraction timeStretch = st->timeStretch(tick);
            Fraction f = cr->globalTicks() * timeStretch;
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

Ret MasterScore::sanityCheck()
{
    std::string accumulatedErrors;

    for (Score* score : scoreList()) {
        Ret ret = score->sanityCheckLocal();
        if (ret) {
            // everything is fine in this part, let's continue
            continue;
        }

        if (!accumulatedErrors.empty()) {
            accumulatedErrors += "\n";
        }

        accumulatedErrors += ret.text();
    }

    if (accumulatedErrors.empty()) {
        return muse::make_ok();
    }
    return Ret(static_cast<int>(Err::FileCorrupted), accumulatedErrors);
}

Ret Score::sanityCheckLocal()
{
    TRACEFUNC;

    StringList errors;
    int mNumber = 1;

    auto excerptInfo = [this]() {
        if (isMaster()) {
            return muse::mtrc("engraving", "Full score");
        }

        //: %1 is the name of a part score.
        return muse::mtrc("engraving", "Part score: %1").arg(name());
    };

    setHasCorruptedMeasures(false);
    for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
        Fraction mLen = m->ticks();
        size_t endStaff  = staves().size();

        for (size_t staffIdx = 0; staffIdx < endStaff; ++staffIdx) {
            Rest* fmrest0 = nullptr; // full measure rest in voice 0
            Fraction voices[VOICES];

            m->setCorrupted(staffIdx, false);

            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                for (voice_idx_t v = 0; v < VOICES; ++v) {
                    EngravingItem* element = s->element(staffIdx * VOICES + v);
                    if (!element) {
                        continue;
                    }

                    ChordRest* cr = toChordRest(element);
                    voices[v] += cr->actualTicks();
                    if (v == 0 && cr->isRest()) {
                        Rest* r = toRest(cr);
                        if (r->durationType().isMeasure()) {
                            fmrest0 = r;
                        }
                    }
                }
            }

            bool checkRepeats = m->isMeasureRepeatGroup(staffIdx);
            bool repeatsIsValid = true;

            if (checkRepeats) {
                repeatsIsValid = m->measureRepeatElement(staffIdx) != nullptr;
            }

            if (!repeatsIsValid) {
                errors << muse::mtrc("engraving", "<b>Corrupted measure</b>: %1, measure %2, staff %3.")
                    .arg(excerptInfo()).arg(mNumber).arg(staffIdx + 1);
                m->setCorrupted(staffIdx, true);
            }

            if (voices[0] != mLen) {
                //: %1 describes in which score the corruption is (either `Full score` or `"[part name]" part score`)
                errors << muse::mtrc("engraving", "<b>Incomplete measure</b>: %1, measure %2, staff %3. Found: %4. Expected: %5.")
                    .arg(excerptInfo()).arg(mNumber).arg(staffIdx + 1).arg(voices[0].toString(), mLen.toString());
                m->setCorrupted(staffIdx, true);
                // try to fix a bad full measure rest
                if (fmrest0) {
                    // fmrest0->setDuration(mLen * fmrest0->staff()->timeStretch(fmrest0->tick()));
                    fmrest0->setTicks(mLen);
                }
            }

            for (voice_idx_t v = 1; v < VOICES; ++v) {
                if (voices[v] > mLen) {
                    //: %1 describes in which score the corruption is (either `Full score` or `"[part name]" part score`)
                    errors << muse::mtrc("engraving", "<b>Voice too long</b>: %1, measure %2, staff %3, voice %4. Found: %5. Expected: %6.")
                        .arg(excerptInfo()).arg(mNumber).arg(staffIdx + 1).arg(v + 1).arg(voices[v].toString(), mLen.toString());
                    m->setCorrupted(staffIdx, true);
                }
            }
        }

        mNumber++;
    }

    if (errors.empty()) {
        return muse::make_ok();
    }

    return Ret(static_cast<int>(Err::FileCorrupted), errors.join(u"\n").toStdString());
}

//---------------------------------------------------------
//   checkKeys
///    check that key map is in sync with actual keys
//---------------------------------------------------------

bool Score::checkKeys()
{
    bool rc = true;
    for (size_t i = 0; i < nstaves(); ++i) {
        Key k = staff(i)->key(Fraction(0, 1));
        for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            Segment* s = m->findSegment(SegmentType::KeySig, m->tick());
            if (s) {
                EngravingItem* element = s->element(static_cast<int>(i) * VOICES);
                if (element) {
                    k = toKeySig(element)->key();
                }
            }
            if (staff(i)->key(m->tick()) != k) {
                LOGD("measure %d (tick %d) : key %d, map %d", m->no(), m->tick().ticks(), int(k),
                     int(staff(i)->key(m->tick())));
                rc = false;
            }
        }
    }
    return rc;
}

//---------------------------------------------------------
//   fillGap
//---------------------------------------------------------

void Measure::fillGap(const Fraction& pos, const Fraction& len, track_idx_t track, const Fraction& stretch, bool useGapRests)
{
    LOGN("measure %6d pos %d, len %d/%d, stretch %d/%d track %zu",
         tick().ticks(),
         pos.ticks(),
         len.numerator(), len.denominator(),
         stretch.numerator(), stretch.denominator(),
         track);

    // break the gap into shorter durations if necessary
    std::vector<TDuration> durationList = toRhythmicDurationList(len, true, pos, timesig(), this, 0);

    Fraction curTick = pos;
    for (TDuration d : durationList) {
        Rest* rest = Factory::createRest(score()->dummy()->segment());
        rest->setTicks(d.isMeasure() ? ticks() : d.fraction());
        rest->setDurationType(d);
        rest->setTrack(track);
        rest->setGap(useGapRests);
        score()->undoAddCR(rest, this, curTick + tick());
        curTick += d.fraction();
    }
}

//---------------------------------------------------------
//   checkMeasure
//    after opening / paste and every read operation
//    this method checks for gaps and fills them
//    with invisible rests
//---------------------------------------------------------

void Measure::checkMeasure(staff_idx_t staffIdx, bool useGapRests)
{
    if (isMMRest()) {
        return;
    }

    track_idx_t strack = staffIdx * VOICES;
    track_idx_t dtrack = strack + (hasVoices(staffIdx) ? VOICES : 1);
    Fraction stretch = score()->staff(staffIdx)->timeStretch(tick());
    Fraction f       = ticks() * stretch;

    for (track_idx_t track = strack; track < dtrack; track++) {
        Fraction expectedPos = Fraction(0, 1);
        Fraction currentPos  = Fraction(0, 1);

        for (Segment* seg = first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
            EngravingItem* e = seg->element(track);
            if (!e) {
                continue;
            }

            ChordRest* cr = toChordRest(e);
            currentPos    = seg->rtick() * stretch;

            if (currentPos < expectedPos) {
                LOGN("in measure overrun %6d at %d-%d track %zu", tick().ticks(),
                     (currentPos / stretch).ticks(), (expectedPos / stretch).ticks(), track);
                break;
            } else if (currentPos > expectedPos) {
                LOGN("in measure underrun %6d at %d-%d track %zu", tick().ticks(),
                     (currentPos / stretch).ticks(), (expectedPos / stretch).ticks(), track);
                fillGap(expectedPos, currentPos - expectedPos, track, stretch, useGapRests);
                if (currentPos >= f) {
                    break;
                }
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
            LOGD("measure overrun %6d, %d > %d, track %zu", tick().ticks(), expectedPos.ticks(), f.ticks(), track);
        }
    }
}
}
