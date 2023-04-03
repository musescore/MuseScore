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
#ifndef MU_ENGRAVING_TREAD_H
#define MU_ENGRAVING_TREAD_H

namespace mu::engraving {
class XmlReader;
class ReadContext;
class EngravingItem;
class TextBase;
class TempoText;
class StaffText;
class StaffTextBase;
class Dynamic;
class Harmony;
class FretDiagram;
class Sticking;
class SystemText;
class PlayTechAnnotation;
class RehearsalMark;
class InstrumentChange;
class StaffState;
class FiguredBass;
class Fermata;
class Image;
class Tuplet;
class Ambitus;
class Accidental;
class Marker;
class Jump;
class MeasureNumber;
class MeasureNumberBase;
class MMRestRange;
class SystemDivider;
class ActionIcon;
class Arpeggio;
class Articulation;
class Audio;
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
class Symbol;
class BSymbol;
class FSymbol;
class Chord;
class ChordRest;
class ChordLine;
class Clef;
class Fingering;
class Glissando;
class GradualTempoChange;
class Groups;
class Hairpin;
class Hook;
class KeySig;
class Lyrics;
class MeasureBase;
class Note;
class NoteDot;
class SLine;
class Spanner;
class Stem;
class StemSlash;
class TextLineBase;
class Tremolo;
class TremoloBar;
}

namespace mu::engraving::rw400 {
class TRead
{
public:
    TRead() = default;

    // factory
    static void readItem(EngravingItem* el, XmlReader& xml, ReadContext& ctx);

    // types

    static void read(TextBase* t, XmlReader& xml, ReadContext& ctx);
    static void read(TempoText* t, XmlReader& xml, ReadContext& ctx);
    static void read(StaffText* t, XmlReader& xml, ReadContext& ctx);
    static void read(StaffTextBase* t, XmlReader& xml, ReadContext& ctx);
    static void read(Dynamic* d, XmlReader& xml, ReadContext& ctx);
    static void read(Harmony* h, XmlReader& xml, ReadContext& ctx);
    static void read(FretDiagram* d, XmlReader& xml, ReadContext& ctx);
    static void read(Sticking* s, XmlReader& xml, ReadContext& ctx);
    static void read(SystemText* t, XmlReader& xml, ReadContext& ctx);
    static void read(PlayTechAnnotation* a, XmlReader& xml, ReadContext& ctx);
    static void read(RehearsalMark* m, XmlReader& xml, ReadContext& ctx);
    static void read(StaffState* s, XmlReader& xml, ReadContext& ctx);
    static void read(Image* i, XmlReader& xml, ReadContext& ctx);
    static void read(Tuplet* t, XmlReader& xml, ReadContext& ctx);
    static void read(Ambitus* a, XmlReader& xml, ReadContext& ctx);
    static void read(Accidental* a, XmlReader& xml, ReadContext& ctx);
    static void read(Marker* m, XmlReader& xml, ReadContext& ctx);
    static void read(Jump* j, XmlReader& xml, ReadContext& ctx);
    static void read(MeasureNumber* n, XmlReader& xml, ReadContext& ctx);
    static void read(MeasureNumberBase* b, XmlReader& xml, ReadContext& ctx);
    static void read(MMRestRange* r, XmlReader& xml, ReadContext& ctx);
    static void read(SystemDivider* d, XmlReader& xml, ReadContext& ctx);
    static void read(ActionIcon* i, XmlReader& xml, ReadContext& ctx);
    static void read(Arpeggio* a, XmlReader& xml, ReadContext& ctx);
    static void read(Articulation* a, XmlReader& xml, ReadContext& ctx);
    static void read(Audio* a, XmlReader& xml, ReadContext& ctx);
    static void read(BagpipeEmbellishment* b, XmlReader& xml, ReadContext& ctx);
    static void read(BarLine* l, XmlReader& xml, ReadContext& ctx);
    static void read(Beam* b, XmlReader& xml, ReadContext& ctx);
    static void read(Bend* b, XmlReader& xml, ReadContext& ctx);
    static void read(Box* b, XmlReader& xml, ReadContext& ctx);
    static void read(HBox* b, XmlReader& xml, ReadContext& ctx);
    static void read(VBox* b, XmlReader& xml, ReadContext& ctx);
    static void read(FBox* b, XmlReader& xml, ReadContext& ctx);
    static void read(TBox* b, XmlReader& xml, ReadContext& ctx);
    static void read(Bracket* b, XmlReader& xml, ReadContext& ctx);
    static void read(Breath* b, XmlReader& xml, ReadContext& ctx);
    static void read(Symbol* sym, XmlReader& xml, ReadContext& ctx);
    static void read(FSymbol* sym, XmlReader& xml, ReadContext& ctx);
    static void read(Chord* ch, XmlReader& xml, ReadContext& ctx);
    static void read(ChordLine* l, XmlReader& xml, ReadContext& ctx);
    static void read(Clef* c, XmlReader& xml, ReadContext& ctx);
    static void read(Fermata* f, XmlReader& xml, ReadContext& ctx);
    static void read(FiguredBass* b, XmlReader& xml, ReadContext& ctx);
    static void read(Fingering* f, XmlReader& xml, ReadContext& ctx);
    static void read(Glissando* g, XmlReader& xml, ReadContext& ctx);
    static void read(GradualTempoChange* c, XmlReader& xml, ReadContext& ctx);
    static void read(Groups* g, XmlReader& xml, ReadContext& ctx);
    static void read(Hairpin* h, XmlReader& xml, ReadContext& ctx);
    static void read(Hook* h, XmlReader& xml, ReadContext& ctx);
    static void read(InstrumentChange* c, XmlReader& xml, ReadContext& ctx);
    static void read(KeySig* s, XmlReader& xml, ReadContext& ctx);
    static void read(Lyrics* l, XmlReader& xml, ReadContext& ctx);
    static void read(Note* n, XmlReader& xml, ReadContext& ctx);
    static void read(NoteDot* d, XmlReader& xml, ReadContext& ctx);
    static void read(Stem* s, XmlReader& xml, ReadContext& ctx);
    static void read(StemSlash* s, XmlReader& xml, ReadContext& ctx);
    static void read(Tremolo* t, XmlReader& xml, ReadContext& ctx);
    static void read(TremoloBar* b, XmlReader& xml, ReadContext& ctx);

    // temp compat
    static bool readProperties(StaffTextBase* t, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Articulation* a, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Box* b, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(MeasureBase* b, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(BSymbol* sym, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Chord* ch, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(ChordRest* ch, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Fermata* f, XmlReader& e, ReadContext& ctx);
    static bool readProperties(Lyrics* l, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Note* n, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(SLine* l, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Spanner* s, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Stem* s, XmlReader& e, ReadContext& ctx);
    static bool readProperties(TextLineBase* b, XmlReader& e, ReadContext& ctx);
};
}

#endif // MU_ENGRAVING_TREAD_H
