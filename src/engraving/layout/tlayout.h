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
#ifndef MU_ENGRAVING_TLAYOUT_H
#define MU_ENGRAVING_TLAYOUT_H

#include "layoutcontext.h"

namespace mu::engraving {
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
class ChordLine;
class Clef;

class DeadSlapped;
class Dynamic;

class Expression;

class Fermata;
class FiguredBassItem;
class Fingering;
class FretDiagram;
class FretCircle;

class Glissando;
class GlissandoSegment;
class GraceNotesGroup;
class GradualTempoChangeSegment;

class HairpinSegment;
class Hairpin;
class HarmonicMarkSegment;
class Harmony;
class Hook;

class Image;
class InstrumentChange;
class Jump;

class KeySig;

class LedgerLine;
class LetRingSegment;
class Lyrics;
class LyricsLine;
class LyricsLineSegment;

class Marker;
class MeasureBase;
class MeasureNumberBase;
class MeasureRepeat;
class MMRest;

class Note;
class NoteDot;

class OttavaSegment;

class PalmMuteSegment;
class PedalSegment;

class SLine;
}

namespace mu::engraving::v0 {
class TLayout
{
public:

    static void layout(Accidental* item, LayoutContext& ctx);
    static void layout(ActionIcon* item, LayoutContext& ctx);
    static void layout(Ambitus* item, LayoutContext& ctx);
    static void layout(Arpeggio* item, LayoutContext& ctx);
    static void layout(Articulation* item, LayoutContext& ctx);

    static void layout(BagpipeEmbellishment* item, LayoutContext& ctx);
    static void layout(BarLine* item, LayoutContext& ctx);
    static void layout2(BarLine* item, LayoutContext& ctx);
    static void layout(Beam* item, LayoutContext& ctx);
    static void layout1(Beam* item, LayoutContext& ctx);
    static void layout(Bend* item, LayoutContext& ctx);

    static void layout(Box* item, LayoutContext& ctx);    // factory
    static void layoutBox(Box* item, LayoutContext& ctx); // base
    static void layout(HBox* item, LayoutContext& ctx);
    static void layout(VBox* item, LayoutContext& ctx);
    static void layout(FBox* item, LayoutContext& ctx);
    static void layout(TBox* item, LayoutContext& ctx);

    static void layout(Bracket* item, LayoutContext& ctx);
    static void layout(Breath* item, LayoutContext& ctx);

    static void layout(Chord* item, LayoutContext& ctx);
    static void layout(ChordLine* item, LayoutContext& ctx);
    static void layout(Clef* item, LayoutContext& ctx);

    static void layout(DeadSlapped* item, LayoutContext& ctx);
    static void layout(Dynamic* item, LayoutContext& ctx);

    static void layout(Expression* item, LayoutContext& ctx);

    static void layout(Fermata* item, LayoutContext& ctx);
    static void layout(FiguredBassItem* item, LayoutContext& ctx);
    static void layout(Fingering* item, LayoutContext& ctx);
    static void layout(FretDiagram* item, LayoutContext& ctx);
    static void layout(FretCircle* item, LayoutContext& ctx);

    static void layout(Glissando* item, LayoutContext& ctx);
    static void layout(GlissandoSegment* item, LayoutContext& ctx);
    static void layout(GraceNotesGroup* item, LayoutContext& ctx);
    static void layout(GradualTempoChangeSegment* item, LayoutContext& ctx);

    static void layout(HairpinSegment* item, LayoutContext& ctx);
    static void layout(Hairpin* item, LayoutContext& ctx);
    static void layout(HarmonicMarkSegment* item, LayoutContext& ctx);
    static void layout(Harmony* item, LayoutContext& ctx);
    static void layout1(Harmony* item, LayoutContext& ctx);
    static void layout(Hook* item, LayoutContext& ctx);

    static void layout(Image* item, LayoutContext& ctx);
    static void layout(InstrumentChange* item, LayoutContext& ctx);

    static void layout(Jump* item, LayoutContext& ctx);

    static void layout(KeySig* item, LayoutContext& ctx);

    static void layout(LedgerLine* item, LayoutContext& ctx);
    static void layout(LetRingSegment* item, LayoutContext& ctx);
    static void layout(Lyrics* item, LayoutContext& ctx);
    static void layout(LyricsLine* item, LayoutContext& ctx);
    static void layout(LyricsLineSegment* item, LayoutContext& ctx);

    static void layout(Marker* item, LayoutContext& ctx);
    static void layoutMeasureBase(MeasureBase* item, LayoutContext& ctx); // base
    static void layoutMeasureNumberBase(MeasureNumberBase* item, LayoutContext& ctx); // base
    static void layout(MeasureRepeat* item, LayoutContext& ctx);
    static void layout(MMRest* item, LayoutContext& ctx);

    static void layout(Note* item, LayoutContext& ctx);
    static void layout2(Note* item, LayoutContext& ctx);
    static void layout(NoteDot* item, LayoutContext& ctx);

    static void layout(OttavaSegment* item, LayoutContext& ctx);

    static void layout(PalmMuteSegment* item, LayoutContext& ctx);
    static void layout(PedalSegment* item, LayoutContext& ctx);

    static void layoutLine(SLine* item, LayoutContext& ctx); // base

private:
    static void layoutSingleGlyphAccidental(Accidental* item, LayoutContext& ctx);
    static void layoutMultiGlyphAccidental(Accidental* item, LayoutContext& ctx);

    static void adjustLayoutWithoutImages(VBox* item, LayoutContext& ctx);

    static PointF calculateBoundingRect(Harmony* item);

    static void keySigAddLayout(KeySig* item, SymId sym, int line);
};
}

#endif // MU_ENGRAVING_TLAYOUT_H
