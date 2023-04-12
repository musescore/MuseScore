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

#include "global/modularity/ioc.h"
#include "../../iengravingconfiguration.h"

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

class Fermata;
class FiguredBass;
class FiguredBassItem;
class Fingering;
class FretDiagram;

class Glissando;
class GradualTempoChange;
class Groups;

class Hairpin;
class Harmony;
class Hook;

class Image;
class InstrumentChange;

class Jump;

class KeySig;

class LayoutBreak;
class LedgerLine;
class LetRing;
class Location;
class Lyrics;

class Marker;
class MeasureNumber;
class MeasureRepeat;
class MMRest;
class MMRestRange;

class Note;
class NoteEvent;
class NoteDot;
class NoteLine;

class Ottava;

class Page;
class PalmMute;
class Part;
class Pedal;
class PlayTechAnnotation;

class RehearsalMark;
class Rest;

class Segment;
class Slur;
class SlurTie;
class SLine;
class Spanner;
class Spacer;
class StaffState;
class StaffText;
class StaffTextBase;
class StaffType;
class StaffTypeChange;
class Stem;
class StemSlash;
class Sticking;
class Symbol;
class BSymbol;
class System;
class SystemDivider;
class SystemText;

class TextLineBase;
class TextBase;
}

namespace mu::engraving::rw400 {
class TWrite
{
    INJECT_STATIC(engraving, IEngravingConfiguration, engravingConfiguration)

public:
    TWrite() = default;

    static void write(const Accidental* a, XmlWriter& xml, WriteContext& ctx);
    static void write(const ActionIcon* a, XmlWriter& xml, WriteContext& ctx);
    static void write(const Ambitus* a, XmlWriter& xml, WriteContext& ctx);
    static void write(const Arpeggio* a, XmlWriter& xml, WriteContext& ctx);
    static void write(const Articulation* a, XmlWriter& xml, WriteContext& ctx);

    static void write(const BagpipeEmbellishment* b, XmlWriter& xml, WriteContext& ctx);
    static void write(const BarLine* b, XmlWriter& xml, WriteContext& ctx);
    static void write(const Beam* b, XmlWriter& xml, WriteContext& ctx);
    static void write(const Bend* b, XmlWriter& xml, WriteContext& ctx);
    static void write(const Box* b, XmlWriter& xml, WriteContext& ctx);
    static void write(const HBox* b, XmlWriter& xml, WriteContext& ctx);
    static void write(const VBox* b, XmlWriter& xml, WriteContext& ctx);
    static void write(const FBox* b, XmlWriter& xml, WriteContext& ctx);
    static void write(const TBox* b, XmlWriter& xml, WriteContext& ctx);
    static void write(const Bracket* b, XmlWriter& xml, WriteContext& ctx);
    static void write(const Breath* b, XmlWriter& xml, WriteContext& ctx);

    static void write(const Chord* c, XmlWriter& xml, WriteContext& ctx);
    static void write(const ChordLine* c, XmlWriter& xml, WriteContext& ctx);
    static void write(const Clef* c, XmlWriter& xml, WriteContext& ctx);

    static void write(const Dynamic* d, XmlWriter& xml, WriteContext& ctx);

    static void write(const Fermata* f, XmlWriter& xml, WriteContext& ctx);
    static void write(const FiguredBass* f, XmlWriter& xml, WriteContext& ctx);
    static void write(const FiguredBassItem* f, XmlWriter& xml, WriteContext& ctx);
    static void write(const Fingering* f, XmlWriter& xml, WriteContext& ctx);
    static void write(const FretDiagram* f, XmlWriter& xml, WriteContext& ctx);

    static void write(const Glissando* g, XmlWriter& xml, WriteContext& ctx);
    static void write(const GradualTempoChange* g, XmlWriter& xml, WriteContext& ctx);
    static void write(const Groups* g, XmlWriter& xml, WriteContext& ctx);

    static void write(const Hairpin* h, XmlWriter& xml, WriteContext& ctx);
    static void write(const Harmony* h, XmlWriter& xml, WriteContext& ctx);
    static void write(const Hook* h, XmlWriter& xml, WriteContext& ctx);

    static void write(const Image* i, XmlWriter& xml, WriteContext& ctx);
    static void write(const InstrumentChange* i, XmlWriter& xml, WriteContext& ctx);

    static void write(const Jump* j, XmlWriter& xml, WriteContext& ctx);

    static void write(const KeySig* k, XmlWriter& xml, WriteContext& ctx);

    static void write(const LayoutBreak* l, XmlWriter& xml, WriteContext& ctx);
    static void write(const LedgerLine* l, XmlWriter& xml, WriteContext& ctx);
    static void write(const LetRing* l, XmlWriter& xml, WriteContext& ctx);
    static void write(const Location* l, XmlWriter& xml, WriteContext& ctx);
    static void write(const Lyrics* l, XmlWriter& xml, WriteContext& ctx);

    static void write(const Marker* m, XmlWriter& xml, WriteContext& ctx);
    static void write(const MeasureNumber* m, XmlWriter& xml, WriteContext& ctx);
    static void write(const MeasureRepeat* m, XmlWriter& xml, WriteContext& ctx);
    static void write(const MMRest* m, XmlWriter& xml, WriteContext& ctx);
    static void write(const MMRestRange* m, XmlWriter& xml, WriteContext& ctx);

    static void write(const Note* n, XmlWriter& xml, WriteContext& ctx);
    static void write(const NoteEvent* n, XmlWriter& xml, WriteContext& ctx);
    static void write(const NoteDot* n, XmlWriter& xml, WriteContext& ctx);
    static void write(const NoteLine* n, XmlWriter& xml, WriteContext& ctx);

    static void write(const Ottava* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Page* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const PalmMute* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Part* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Pedal* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const PlayTechAnnotation* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const RehearsalMark* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Rest* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Segment* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Slur* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Spacer* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const StaffState* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const StaffText* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const StaffType* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const StaffTypeChange* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Stem* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const StemSlash* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Sticking* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Symbol* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const System* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const SystemDivider* item, XmlWriter& xml, WriteContext& ctx);

private:
    static void writeProperty(const EngravingItem* item, XmlWriter& xml, Pid pid);
    static void writeStyledProperties(const EngravingItem* item, XmlWriter& xml);

    static void writeItemProperties(const EngravingItem* item, XmlWriter& xml, WriteContext& ctx);
    static void writeBoxProperties(const Box* b, XmlWriter& xml, WriteContext& ctx);

    static void writeProperties(const Box* b, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const HBox* b, XmlWriter& xml, WriteContext& ctx);

    static void writeProperties(const ChordRest* c, XmlWriter& xml, WriteContext& ctx);
    static void writeChordRestBeam(const ChordRest* c, XmlWriter& xml, WriteContext& ctx);

    static void writeProperties(const Rest* r, XmlWriter& xml, WriteContext& ctx);

    static void write(const StaffTextBase* s, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const SlurTie* s, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const SLine* l, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const Spanner* s, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const BSymbol* s, XmlWriter& xml, WriteContext& ctx);

    static void writeProperties(const TextLineBase* l, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const TextBase* t, XmlWriter& xml, WriteContext& ctx, bool writeText);
};
}

#endif // MU_ENGRAVING_TWRITE_H
