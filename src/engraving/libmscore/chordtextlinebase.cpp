/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "chordtextlinebase.h"

#include "chordrest.h"
#include "measure.h"
#include "segment.h"
#include "score.h"

#include "types/types.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   ChordTextLineBase
//---------------------------------------------------------

ChordTextLineBase::ChordTextLineBase(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : TextLineBase(type, parent, f)
{
}

//---------------------------------------------------------
//   linePos
//    return System() coordinates
//---------------------------------------------------------

PointF ChordTextLineBase::linePos(Grip grip, System** sys) const
{
    double x = 0.0;
    double nhw = score()->noteHeadWidth();
    System* s = nullptr;
    if (grip == Grip::START) {
        ChordRest* c = toChordRest(startElement());
        if (!c) {
            *sys = s;
            return PointF();
        }
        s = c->segment()->system();
        x = c->pos().x() + c->segment()->pos().x() + c->segment()->measure()->pos().x();
        if (c->isRest() && c->durationType() == DurationType::V_MEASURE) {
            x -= c->x();
        }
    } else {
        EngravingItem* e = endElement();
        ChordRest* c = toChordRest(endElement());
        if (!e || e == startElement() || (endHookType() == HookType::HOOK_90)) {
            // pedal marking on single note or ends with non-angled hook:
            // extend to next note or end of measure
            Segment* seg = nullptr;
            if (!e) {
                seg = startSegment();
            } else {
                seg = c->segment();
            }
            if (seg) {
                seg = seg->next();
                for (; seg; seg = seg->next()) {
                    if (seg->segmentType() == SegmentType::ChordRest) {
                        // look for a chord/rest in any voice on this staff
                        bool crFound = false;
                        track_idx_t track = staffIdx() * VOICES;
                        for (voice_idx_t i = 0; i < VOICES; ++i) {
                            if (seg->element(track + i)) {
                                crFound = true;
                                break;
                            }
                        }
                        if (crFound) {
                            break;
                        }
                    } else if (seg->segmentType() == SegmentType::EndBarLine) {
                        break;
                    }
                }
            }
            if (seg) {
                s = seg->system();
                x = seg->pos().x() + seg->measure()->pos().x() - nhw * 2;
            }
        } else if (c) {
            s = c->segment()->system();
            x = c->pos().x() + c->segment()->pos().x() + c->segment()->measure()->pos().x();
            if (c->isRest() && c->durationType() == DurationType::V_MEASURE) {
                x -= c->x();
            }
        }
        if (!s) {
            Fraction t = tick2();
            Measure* m = score()->tick2measure(t);
            s = m->system();
            x = m->tick2pos(t);
        }
        x += nhw;
    }

    *sys = s;
    return PointF(x, 0);
}
}
