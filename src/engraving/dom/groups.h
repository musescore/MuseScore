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

#ifndef MU_ENGRAVING_GROUPS_H
#define MU_ENGRAVING_GROUPS_H

#include "../types/groupnode.h"
#include "../types/types.h"

namespace mu::engraving {
class ChordRest;

//---------------------------------------------------------
//   @@ Groups
///    GroupNodes must be sorted by tick
//---------------------------------------------------------

class Groups
{
public:
    Groups() {}
    Groups(const GroupNodes& nodes)
        : m_nodes(nodes) {}

    const GroupNodes& nodes() const { return m_nodes; }
    bool empty() const { return m_nodes.empty(); }
    void addNode(const GroupNode& n) { m_nodes.push_back(n); }

    BeamMode beamMode(int tick, DurationType d) const;
    void addStop(int pos, DurationType d, BeamMode bm);
    bool operator==(const Groups& g) const
    {
        if (g.m_nodes.size() != m_nodes.size()) {
            return false;
        }
        for (size_t i = 0; i < m_nodes.size(); ++i) {
            if (!(g.m_nodes.at(i) == m_nodes.at(i))) {
                return false;
            }
        }
        return true;
    }

    void dump(const char*) const;

    static const Groups& endings(const Fraction& f);
    static BeamMode endBeam(const ChordRest* cr, const ChordRest* prev = 0);

private:
    GroupNodes m_nodes;
};
} // namespace mu::engraving

#endif
