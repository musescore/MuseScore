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
    static void draw(const Capo* item, draw::Painter* painter);

    static void draw(const DeadSlapped* item, draw::Painter* painter);
    static void draw(const Dynamic* item, draw::Painter* painter);

    static void draw(const Expression* item, draw::Painter* painter);

    static void draw(const Fermata* item, draw::Painter* painter);
    static void draw(const FiguredBass* item, draw::Painter* painter);
    static void draw(const Fingering* item, draw::Painter* painter);
    static void draw(const FretDiagram* item, draw::Painter* painter);
    static void draw(const FretCircle* item, draw::Painter* painter);

    static void draw(const GlissandoSegment* item, draw::Painter* painter);
    static void draw(const GradualTempoChangeSegment* item, draw::Painter* painter);

    static void draw(const HairpinSegment* item, draw::Painter* painter);
    static void draw(const HarpPedalDiagram* item, draw::Painter* painter);
    static void draw(const HarmonicMarkSegment* item, draw::Painter* painter);
    static void draw(const Harmony* item, draw::Painter* painter);
    static void draw(const Hook* item, draw::Painter* painter);

    static void draw(const Image* item, draw::Painter* painter);
    static void draw(const InstrumentChange* item, draw::Painter* painter);
    static void draw(const InstrumentName* item, draw::Painter* painter);

    static void draw(const Jump* item, draw::Painter* painter);

    static void draw(const KeySig* item, draw::Painter* painter);

    static void draw(const LayoutBreak* item, draw::Painter* painter);
    static void draw(const LedgerLine* item, draw::Painter* painter);
    static void draw(const LetRingSegment* item, draw::Painter* painter);
    static void draw(const Lyrics* item, draw::Painter* painter);
    static void draw(const LyricsLineSegment* item, draw::Painter* painter);

    static void draw(const Marker* item, draw::Painter* painter);
    static void draw(const MeasureNumber* item, draw::Painter* painter);
    static void draw(const MeasureRepeat* item, draw::Painter* painter);
    static void draw(const MMRest* item, draw::Painter* painter);
    static void draw(const MMRestRange* item, draw::Painter* painter);

    static void draw(const Note* item, draw::Painter* painter);
    static void draw(const NoteDot* item, draw::Painter* painter);
    static void draw(const NoteHead* item, draw::Painter* painter);

    static void draw(const Ornament* item, draw::Painter* painter);
    static void draw(const OttavaSegment* item, draw::Painter* painter);

    static void draw(const PalmMuteSegment* item, draw::Painter* painter);
    static void draw(const PedalSegment* item, draw::Painter* painter);
    static void draw(const PickScrapeSegment* item, draw::Painter* painter);
    static void draw(const PlayTechAnnotation* item, draw::Painter* painter);

    static void draw(const RasgueadoSegment* item, draw::Painter* painter);
    static void draw(const RehearsalMark* item, draw::Painter* painter);
    static void draw(const Rest* item, draw::Painter* painter);

    static void draw(const ShadowNote* item, draw::Painter* painter);
    static void draw(const SlurSegment* item, draw::Painter* painter);
    static void draw(const Spacer* item, draw::Painter* painter);
    static void draw(const StaffState* item, draw::Painter* painter);
    static void draw(const StaffText* item, draw::Painter* painter);
    static void draw(const StaffTypeChange* item, draw::Painter* painter);
    static void draw(const Stem* item, draw::Painter* painter);
    static void draw(const StemSlash* item, draw::Painter* painter);
    static void draw(const Sticking* item, draw::Painter* painter);
    static void draw(const StretchedBend* item, draw::Painter* painter);
    static void draw(const Symbol* item, draw::Painter* painter);
    static void draw(const SystemDivider* item, draw::Painter* painter);
    static void draw(const SystemText* item, draw::Painter* painter);

    static void draw(const TabDurationSymbol* item, draw::Painter* painter);
    static void draw(const TempoText* item, draw::Painter* painter);
    static void draw(const Text* item, draw::Painter* painter);
    static void draw(const TextLineSegment* item, draw::Painter* painter);
    static void draw(const TieSegment* item, draw::Painter* painter);
    static void draw(const TimeSig* item, draw::Painter* painter);
    static void draw(const Tremolo* item, draw::Painter* painter);
    static void draw(const TremoloBar* item, draw::Painter* painter);
    static void draw(const TrillSegment* item, draw::Painter* painter);
    static void draw(const TripletFeel* item, draw::Painter* painter);
    static void draw(const Tuplet* item, draw::Painter* painter);
    static void draw(const VibratoSegment* item, draw::Painter* painter);

    static void drawTextBase(const TextBase* item, draw::Painter* painter);
    static void drawTextLineBaseSegment(const TextLineBaseSegment* item, draw::Painter* painter);
};
}

#endif // MU_ENGRAVING_TDRAW_DEV_H
