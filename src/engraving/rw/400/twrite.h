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
class Bend;
class Box;
class HBox;
class VBox;
class FBox;
class TBox;
class Bracket;
class Breath;

class Chord;
class ChordRest;
class ChordLine;
class Clef;

class DurationElement;
class Dynamic;

class TextBase;
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
    static void write(const Articulation* a, XmlWriter& xml, WriteContext& ctx);

    static void write(BagpipeEmbellishment* b, XmlWriter& xml, WriteContext& ctx);
    static void write(BarLine* b, XmlWriter& xml, WriteContext& ctx);
    static void write(Beam* b, XmlWriter& xml, WriteContext& ctx);
    static void write(Bend* b, XmlWriter& xml, WriteContext& ctx);
    static void write(Box* b, XmlWriter& xml, WriteContext& ctx);
    static void write(HBox* b, XmlWriter& xml, WriteContext& ctx);
    static void write(VBox* b, XmlWriter& xml, WriteContext& ctx);
    static void write(FBox* b, XmlWriter& xml, WriteContext& ctx);
    static void write(TBox* b, XmlWriter& xml, WriteContext& ctx);
    static void write(Bracket* b, XmlWriter& xml, WriteContext& ctx);
    static void write(Breath* b, XmlWriter& xml, WriteContext& ctx);

    static void write(const Chord* c, XmlWriter& xml, WriteContext& ctx);
    static void write(const ChordLine* c, XmlWriter& xml, WriteContext& ctx);
    static void write(const Clef* c, XmlWriter& xml, WriteContext& ctx);

    static void write(const Dynamic* d, XmlWriter& xml, WriteContext& ctx);

    static void writeProperty(const EngravingItem* item, XmlWriter& xml, Pid pid);
    static void writeStyledProperties(const EngravingItem* item, XmlWriter& xml);

    static void writeItemProperties(const EngravingItem* item, XmlWriter& xml, WriteContext& ctx);
    static void writeBoxProperties(Box* b, XmlWriter& xml, WriteContext& ctx);

private:
    static void writeProperties(Box* b, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(HBox* b, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const ChordRest* c, XmlWriter& xml, WriteContext& ctx);

    static void writeChordRestBeam(const ChordRest* c, XmlWriter& xml, WriteContext& ctx);

    static void writeProperties(const TextBase* t, XmlWriter& xml, WriteContext& ctx, bool writeText);
};
}

#endif // MU_ENGRAVING_TWRITE_H
