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

#include "groups.h"

#include "chordrest.h"
#include "durationtype.h"
#include "measure.h"
#include "staff.h"
#include "tuplet.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   NoteGroup
//---------------------------------------------------------

struct NoteGroup {
    Fraction timeSig;
    Groups endings;
};

//---------------------------------------------------------
//   noteGroups
//---------------------------------------------------------

static std::vector<NoteGroup> noteGroups {
    { Fraction(2, 2),
      Groups({ { 4, 0x200 }, { 8, 0x110 }, { 12, 0x200 }, { 16, 0x111 }, { 20, 0x200 }, { 24, 0x110 }, { 28, 0x200 } })
    },
    { Fraction(3, 2),
      Groups({ { 4, 0x200 }, { 8, 0x110 }, { 12, 0x200 }, { 16, 0x111 }, { 20, 0x200 }, { 24, 0x110 }, { 28, 0x200 },
                 { 32, 0x111 }, { 36, 0x200 }, { 40, 0x110 }, { 44, 0x200 } })
    },
    { Fraction(4, 2),
      Groups({ { 4, 0x200 }, { 8, 0x110 }, { 12, 0x200 }, { 16, 0x111 }, { 20, 0x200 }, { 24, 0x110 }, { 28, 0x200 },
                 { 32, 0x111 }, { 36, 0x200 }, { 40, 0x110 }, { 44, 0x200 }, { 48, 0x111 }, { 52, 0x200 }, { 56, 0x110 }, { 60, 0x200 } })
    },
    { Fraction(2, 4),
      Groups({ { 4, 0x200 }, { 8, 0x111 }, { 12, 0x200 } })
    },
    { Fraction(3, 4),
      Groups({ { 4, 0x200 }, { 8, 0x111 }, { 12, 0x200 }, { 16, 0x111 }, { 20, 0x200 } })
    },
    { Fraction(4, 4),
      Groups({ { 4, 0x200 }, { 8, 0x110 }, { 12, 0x200 }, { 16, 0x111 }, { 20, 0x200 }, { 24, 0x110 }, { 28, 0x200 } })
    },
    { Fraction(5, 4),
      Groups({ { 4, 0x200 }, { 8, 0x110 }, { 12, 0x200 }, { 16, 0x110 }, { 20, 0x200 }, { 24, 0x111 }, { 28, 0x200 },
                 { 32, 0x110 }, { 36, 0x200 } })
    },
    { Fraction(6, 4),
      Groups({ { 4, 0x200 }, { 8, 0x110 }, { 12, 0x200 }, { 16, 0x110 }, { 20, 0x200 }, { 24, 0x111 }, { 28, 0x200 },
                 { 32, 0x110 }, { 36, 0x200 }, { 40, 0x110 }, { 44, 0x200 } })
    },
    { Fraction(3, 8),
      Groups({ { 4, 0x200 }, { 8, 0x200 } })
    },
    { Fraction(5, 8),
      Groups({ { 4, 0x200 }, { 8, 0x200 }, { 12, 0x111 }, { 16, 0x200 } })
    },
    { Fraction(6, 8),
      Groups({ { 4, 0x200 }, { 8, 0x200 }, { 12, 0x111 }, { 16, 0x200 }, { 20, 0x200 } })
    },
    { Fraction(7, 8),
      Groups({ { 4, 0x200 }, { 8, 0x200 }, { 12, 0x111 }, { 16, 0x200 }, { 20, 0x111 }, { 24, 0x200 } })
    },
    { Fraction(9, 8),
      Groups({ { 4, 0x200 }, { 8, 0x200 }, { 12, 0x111 }, { 16, 0x200 }, { 20, 0x200 }, { 24, 0x111 }, { 28, 0x200 }, { 32, 0x200 } })
    },
    { Fraction(12, 8),
      Groups({ { 4, 0x200 }, { 8, 0x200 }, { 12, 0x111 }, { 16, 0x200 }, { 20, 0x200 }, { 24, 0x111 }, { 28, 0x200 },
                 { 32, 0x200 }, { 36, 0x111 }, { 40, 0x200 }, { 44, 0x200 } })
    },
};

//---------------------------------------------------------
//   endBeam
//---------------------------------------------------------

BeamMode Groups::endBeam(const ChordRest* cr, const ChordRest* prev)
{
    if (cr->isGrace() || cr->beamMode() != BeamMode::AUTO) {
        return cr->beamMode();
    }
    assert(cr->staff());

    // we need to figure out the longest note value beat upon which cr falls in the measure
    Fraction maxTickLen = std::max(cr->ticks(), prev ? prev->ticks() : Fraction());
    Fraction smallestTickLen = Fraction(1, 8); // start with 8th
    Fraction tickLenLimit = Fraction(1, 32); // only check up to 32nds because that's all thats available
                                             // in timesig properties
    while (smallestTickLen > maxTickLen || cr->tick().ticks() % smallestTickLen.ticks() != 0) {
        smallestTickLen /= 2; // proceed to 16th, 32nd, etc
        if (smallestTickLen < tickLenLimit) {
            smallestTickLen = cr->ticks();
            break;
        }
    }
    DurationType bigBeatDuration = TDuration(smallestTickLen).type();

    TDuration crDuration = cr->durationType();
    const Groups& g = cr->staff()->group(cr->tick());
    Fraction stretch = cr->staff()->timeStretch(cr->tick());
    Fraction tick = cr->rtick() * stretch + cr->measure()->anacrusisOffset();

    // We can choose to break beams based on its place in the measure, or by its duration. These
    // can be consolidated mostly, with bias towards its duration.
    BeamMode byType = g.beamMode(tick.ticks(), crDuration.type());
    BeamMode byPos = g.beamMode(tick.ticks(), bigBeatDuration);
    BeamMode val = byType == BeamMode::AUTO ? byPos : byType;

    // context-dependent checks
    if (val == BeamMode::AUTO && tick.isNotZero()) {
        // if current or previous cr is in tuplet (but not both in same tuplet):
        // consider it as if this were next shorter duration
        if (prev && (cr->tuplet() != prev->tuplet()) && (crDuration == prev->durationType())) {
            if (crDuration <= DurationType::V_EIGHTH) {
                val = g.beamMode(tick.ticks(), DurationType::V_16TH);
            } else if (crDuration == DurationType::V_16TH) {
                val = g.beamMode(tick.ticks(), DurationType::V_32ND);
            } else {
                val = g.beamMode(tick.ticks(), DurationType::V_64TH);
            }
        }
        // if there is a hole between previous and current cr, break beam
        // exclude tuplets from this check; tick calculations can be unreliable
        // and they seem to be handled well anyhow
        if (cr->voice() && prev && !prev->tuplet() && prev->tick() + prev->actualTicks() < cr->tick()) {
            val = BeamMode::BEGIN;
        }
    }

    return val;
}

//---------------------------------------------------------
//   beamMode
//    tick is relative to begin of measure
//---------------------------------------------------------

BeamMode Groups::beamMode(int tick, DurationType d) const
{
    int shift;
    switch (d) {
    case DurationType::V_EIGHTH: shift = 0;
        break;
    case DurationType::V_16TH:   shift = 4;
        break;
    case DurationType::V_32ND:   shift = 8;
        break;
    default:
        return BeamMode::AUTO;
    }
    const int dm = Constants::DIVISION / 8;
    for (const GroupNode& e : m_nodes) {
        if (e.pos * dm < tick) {
            continue;
        }
        if (e.pos * dm > tick) {
            break;
        }

        int action = (e.action >> shift) & 0xf;
        switch (action) {
        case 0: return BeamMode::AUTO;
        case 1: return BeamMode::BEGIN;
        case 2: return BeamMode::BEGIN16;
        case 3: return BeamMode::BEGIN32;
        default:
            LOGD("   Groups::beamMode: bad action %d", action);
            return BeamMode::AUTO;
        }
    }
    return BeamMode::AUTO;
}

//---------------------------------------------------------
//   endings
//---------------------------------------------------------

const Groups& Groups::endings(const Fraction& f)
{
    for (const NoteGroup& g : noteGroups) {
        if (g.timeSig.identical(f)) {
            return g.endings;
        }
    }
    NoteGroup g;
    g.timeSig = f;
    noteGroups.push_back(g);

    int pos = 0;
    switch (f.denominator()) {
    case 2:     pos = 16;
        break;
    case 4:     pos = 8;
        break;
    case 8:     pos = 4;
        break;
    case 16:    pos = 2;
        break;
    case 32:    pos = 1;
        break;
        break;
    }
    for (int i = 1; i < f.numerator(); ++i) {
        GroupNode n;
        n.pos    = pos * i;
        n.action = 0x111;
        g.endings.addNode(n);
    }
    return noteGroups.back().endings;
}

//---------------------------------------------------------
//   addStop
//---------------------------------------------------------

void Groups::addStop(int pos, DurationType d, BeamMode bm)
{
    int shift;
    switch (d) {
    case DurationType::V_EIGHTH: shift = 0;
        break;
    case DurationType::V_16TH:   shift = 4;
        break;
    case DurationType::V_32ND:   shift = 8;
        break;
    default:
        return;
    }
    int action;
    if (bm == BeamMode::BEGIN) {
        action = 1;
    } else if (bm == BeamMode::BEGIN16) {
        action = 2;
    } else if (bm == BeamMode::BEGIN32) {
        action = 3;
    } else {
        return;
    }

    pos    /= 60;
    action <<= shift;

    auto i = m_nodes.begin();
    for (; i != m_nodes.end(); ++i) {
        if (i->pos == pos) {
            i->action = (i->action & ~(0xf << shift)) | action;
            return;
        }
        if (i->pos > pos) {
            break;
        }
    }
    m_nodes.insert(i, GroupNode({ pos, action }));
}

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Groups::dump(const char* m) const
{
    LOGD("%s", m);
    for (const GroupNode& n : m_nodes) {
        LOGD("  group tick %d action 0x%02x", n.pos * 60, n.action);
    }
}
}
