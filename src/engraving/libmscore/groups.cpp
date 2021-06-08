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

#include "groups.h"
#include "durationtype.h"
#include "chordrest.h"
#include "staff.h"
#include "tuplet.h"
#include "xml.h"

using namespace mu;

namespace Ms {
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

Beam::Mode Groups::endBeam(ChordRest* cr, ChordRest* prev)
{
    if (cr->isGrace() || cr->beamMode() != Beam::Mode::AUTO) {
        return cr->beamMode();
    }
    Q_ASSERT(cr->staff());

    TDuration d      = cr->durationType();
    const Groups& g  = cr->staff()->group(cr->tick());
    Fraction stretch = cr->staff()->timeStretch(cr->tick());
    Fraction tick    = cr->rtick() * stretch;

    Beam::Mode val = g.beamMode(tick.ticks(), d.type());

    // context-dependent checks
    if (val == Beam::Mode::AUTO && tick.isNotZero()) {
        // if current or previous cr is in tuplet (but not both in same tuplet):
        // consider it as if this were next shorter duration
        if (prev && (cr->tuplet() != prev->tuplet()) && (d == prev->durationType())) {
            if (d >= TDuration::DurationType::V_EIGHTH) {
                val = g.beamMode(tick.ticks(), TDuration::DurationType::V_16TH);
            } else if (d == TDuration::DurationType::V_16TH) {
                val = g.beamMode(tick.ticks(), TDuration::DurationType::V_32ND);
            } else {
                val = g.beamMode(tick.ticks(), TDuration::DurationType::V_64TH);
            }
        }
        // if there is a hole between previous and current cr, break beam
        // exclude tuplets from this check; tick calculations can be unreliable
        // and they seem to be handled well anyhow
        if (cr->voice() && prev && !prev->tuplet() && prev->tick() + prev->actualTicks() < cr->tick()) {
            val = Beam::Mode::BEGIN;
        }
    }

    return val;
}

//---------------------------------------------------------
//   beamMode
//    tick is relative to begin of measure
//---------------------------------------------------------

Beam::Mode Groups::beamMode(int tick, TDuration::DurationType d) const
{
    int shift;
    switch (d) {
    case TDuration::DurationType::V_EIGHTH: shift = 0;
        break;
    case TDuration::DurationType::V_16TH:   shift = 4;
        break;
    case TDuration::DurationType::V_32ND:   shift = 8;
        break;
    default:
        return Beam::Mode::AUTO;
    }
    const int dm = MScore::division / 8;
    for (const GroupNode& e : *this) {
        if (e.pos * dm < tick) {
            continue;
        }
        if (e.pos * dm > tick) {
            break;
        }

        int action = (e.action >> shift) & 0xf;
        switch (action) {
        case 0: return Beam::Mode::AUTO;
        case 1: return Beam::Mode::BEGIN;
        case 2: return Beam::Mode::BEGIN32;
        case 3: return Beam::Mode::BEGIN64;
        default:
            qDebug("   Groups::beamMode: bad action %d", action);
            return Beam::Mode::AUTO;
        }
    }
    return Beam::Mode::AUTO;
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
        g.endings.push_back(n);
    }
    return noteGroups.back().endings;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Groups::write(XmlWriter& xml) const
{
    xml.stag("Groups");
    for (const GroupNode& n : *this) {
        xml.tagE(QString("Node pos=\"%1\" action=\"%2\"")
                 .arg(n.pos).arg(n.action));
    }
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Groups::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "Node") {
            GroupNode n;
            n.pos    = e.intAttribute("pos");
            n.action = e.intAttribute("action");
            push_back(n);
            e.skipCurrentElement();
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   addStop
//---------------------------------------------------------

void Groups::addStop(int pos, TDuration::DurationType d, Beam::Mode bm)
{
    int shift;
    switch (d) {
    case TDuration::DurationType::V_EIGHTH: shift = 0;
        break;
    case TDuration::DurationType::V_16TH:   shift = 4;
        break;
    case TDuration::DurationType::V_32ND:   shift = 8;
        break;
    default:
        return;
    }
    int action;
    if (bm == Beam::Mode::BEGIN) {
        action = 1;
    } else if (bm == Beam::Mode::BEGIN32) {
        action = 2;
    } else if (bm == Beam::Mode::BEGIN64) {
        action = 3;
    } else {
        return;
    }

    pos    /= 60;
    action <<= shift;

    auto i = begin();
    for (; i != end(); ++i) {
        if (i->pos == pos) {
            i->action = (i->action & ~(0xf << shift)) | action;
            return;
        }
        if (i->pos > pos) {
            break;
        }
    }
    insert(i, GroupNode({ pos, action }));
}

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Groups::dump(const char* m) const
{
    qDebug("%s", m);
    for (const GroupNode& n : *this) {
        qDebug("  group tick %d action 0x%02x", n.pos * 60, n.action);
    }
}
}
