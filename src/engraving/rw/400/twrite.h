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
#ifndef MU_ENGRAVING_TWRITE_H
#define MU_ENGRAVING_TWRITE_H

#include "../../libmscore/property.h"

namespace mu::engraving {
class XmlWriter;
class WriteContext;
class EngravingItem;
class Accidental;
class ActionIcon;
class Ambitus;
class Arpeggio;
class Articulation;

class BagpipeEmbellishment;
class BarLine;
class Beam;
}

namespace mu::engraving::rw400 {
class TWrite
{
public:
    TWrite() = default;

    static void write(Accidental* a, XmlWriter& xml, WriteContext& ctx);
    static void write(ActionIcon* a, XmlWriter& xml, WriteContext& ctx);
    static void write(Ambitus* a, XmlWriter& xml, WriteContext& ctx);
    static void write(Arpeggio* a, XmlWriter& xml, WriteContext& ctx);
    static void write(Articulation* a, XmlWriter& xml, WriteContext& ctx);

    static void write(BagpipeEmbellishment* b, XmlWriter& xml, WriteContext& ctx);
    static void write(BarLine* b, XmlWriter& xml, WriteContext& ctx);
    static void write(Beam* b, XmlWriter& xml, WriteContext& ctx);

    static void writeProperty(EngravingItem* item, XmlWriter& xml, Pid pid);

    static void writeItemProperties(EngravingItem* item, XmlWriter& xml, WriteContext& ctx);
};
}

#endif // MU_ENGRAVING_TWRITE_H
