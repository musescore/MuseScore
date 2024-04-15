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
#ifndef MU_ENGRAVING_GROUPNODE_H
#define MU_ENGRAVING_GROUPNODE_H

#include <vector>

namespace mu::engraving {
struct GroupNode {
    int pos = 0;            // tick position, division 32nd
    int action = 0;         // bits: cccc bbbb aaaa
    // cc - 1/64  bb - 1/32  aa - 1/16
    // bit pattern xxxx:
    // 1 - start new beam
    // 2 - start new 1/32 subbeam
    // 3 - start new 1/64 subbeam

    bool operator==(const GroupNode& g) const { return g.pos == pos && g.action == action; }
};
using GroupNodes = std::vector<GroupNode>;
}

#endif // MU_ENGRAVING_GROUPNODE_H
