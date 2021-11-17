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

#ifndef __GROUPS__
#define __GROUPS__

#include "fraction.h"
//#include "mscore.h"
#include "durationtype.h"
#include "beam.h"

namespace Ms {
class ChordRest;
class XmlWriter;
class XmlReader;

//---------------------------------------------------------
//   GroupNode
//---------------------------------------------------------

struct GroupNode {
    int pos;            // tick position, division 32nd
    int action;         // bits: cccc bbbb aaaa
                        // cc - 1/64  bb - 1/32  aa - 1/16
                        // bit pattern xxxx:
                        // 1 - start new beam
                        // 2 - start new 1/32 subbeam
                        // 3 - start new 1/64 subbeam

    bool operator==(const GroupNode& g) const { return g.pos == pos && g.action == action; }
};

//---------------------------------------------------------
//   @@ Groups
///    GroupNodes must be sorted by tick
//---------------------------------------------------------

class Groups : public std::vector<GroupNode>
{
public:
    Groups() {}
    Groups(const std::vector<GroupNode>& l)
        : std::vector<GroupNode>(l) {}

    void write(XmlWriter&) const;
    void read(XmlReader&);

    Beam::Mode beamMode(int tick, TDuration::DurationType d) const;
    void addStop(int pos, TDuration::DurationType d, Beam::Mode bm);
    bool operator==(const Groups& g) const
    {
        if (g.size() != size()) {
            return false;
        }
        for (unsigned i = 0; i < size(); ++i) {
            if (!(g[i] == (*this)[i])) {
                return false;
            }
        }
        return true;
    }

    void dump(const char*) const;

    static const Groups& endings(const Fraction& f);
    static Beam::Mode endBeam(ChordRest* cr, ChordRest* prev = 0);
};

//---------------------------------------------------------
//   NoteGroup
//---------------------------------------------------------

struct NoteGroup {
    Fraction timeSig;
    Groups endings;
};
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Groups);

#endif
