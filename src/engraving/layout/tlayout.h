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

private:
    static void layoutSingleGlyphAccidental(Accidental* item, LayoutContext& ctx);
    static void layoutMultiGlyphAccidental(Accidental* item, LayoutContext& ctx);

    static void adjustLayoutWithoutImages(VBox* item, LayoutContext& ctx);
};
}

#endif // MU_ENGRAVING_TLAYOUT_H
