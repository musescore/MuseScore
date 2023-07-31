/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_ENGRAVING_TDRAW_DEV_H
#define MU_ENGRAVING_TDRAW_DEV_H

#include "libmscore/engravingitem.h"

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
class Capo;

class DeadSlapped;
class Dynamic;

class Expression;

class Fermata;
class FiguredBassItem;
class FiguredBass;
class Fingering;
class FretDiagram;
class FretCircle;

class Glissando;
class GlissandoSegment;
class GraceNotesGroup;
class GradualTempoChangeSegment;
class GradualTempoChange;

class HairpinSegment;
class Hairpin;
class HarpPedalDiagram;
class HarmonicMarkSegment;
class Harmony;
class Hook;

class Image;
class InstrumentChange;
class InstrumentName;
class Jump;

class KeySig;

class LayoutBreak;
class LedgerLine;
class LetRing;
class LetRingSegment;
class LineSegment;
class Lyrics;
class LyricsLine;
class LyricsLineSegment;

class Marker;
class MeasureBase;
class MeasureNumber;
class MeasureNumberBase;
class MeasureRepeat;
class MMRest;
class MMRestRange;

class Note;
class NoteDot;

class Ornament;

class Ottava;
class OttavaSegment;

class PalmMute;
class PalmMuteSegment;
class Pedal;
class PedalSegment;
class PickScrapeSegment;
class PlayTechAnnotation;

class RasgueadoSegment;
class RehearsalMark;
class Rest;

class ShadowNote;
class SLine;
class Slur;
class Spacer;
class SpannerSegment;
class StaffLines;
class StaffState;
class StaffText;
class StaffTypeChange;
class Stem;
class StemSlash;
class Sticking;
class StretchedBend;

class BSymbol;
class Symbol;
class FSymbol;

class SystemDivider;
class SystemText;

class TabDurationSymbol;
class TempoText;
class TextBase;
class Text;
class TextLine;
class TextLineSegment;
class TextLineBase;
class TextLineBaseSegment;
class Tie;
class TimeSig;
class Tremolo;
class TremoloBar;
class TrillSegment;
class TripletFeel;
class Trill;
class Tuplet;

class VibratoSegment;
class Vibrato;
class Volta;
class VoltaSegment;

class WhammyBarSegment;
}

namespace mu::engraving::rendering::dev {
class TDraw
{
public:

    static void drawItem(const EngravingItem* item, draw::Painter* painter);      // factory

private:
    static void draw(const Accidental* item, draw::Painter* painter);
    static void draw(const ActionIcon* item, draw::Painter* painter);
    static void draw(const Ambitus* item, draw::Painter* painter);
    static void draw(const Arpeggio* item, draw::Painter* painter);
    static void draw(const Articulation* item, draw::Painter* painter);

    static void draw(const BagpipeEmbellishment* item, draw::Painter* painter);
    static void draw(const BarLine* item, draw::Painter* painter);
    static void draw(const Beam* item, draw::Painter* painter);
    static void draw(const Bend* item, draw::Painter* painter);
    static void draw(const Box* item, draw::Painter* painter);
    static void draw(const HBox* item, draw::Painter* painter);
    static void draw(const VBox* item, draw::Painter* painter);
    static void draw(const FBox* item, draw::Painter* painter);
    static void draw(const TBox* item, draw::Painter* painter);
    static void draw(const Bracket* item, draw::Painter* painter);
    static void draw(const Breath* item, draw::Painter* painter);

    static void draw(const ChordLine* item, draw::Painter* painter);
    static void draw(const Clef* item, draw::Painter* painter);

    static void draw(const Ornament* item, draw::Painter* painter);
};
}

#endif // MU_ENGRAVING_TDRAW_DEV_H
