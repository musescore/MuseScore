/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#pragma once

#include "dom/engravingitem.h"

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

class LaissezVib;
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
class NoteLineSegment;

class Ornament;

class Ottava;
class OttavaSegment;

class PalmMute;
class PalmMuteSegment;
class Parenthesis;
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
class StringTunings;

class BSymbol;
class Symbol;
class FSymbol;

class SystemDivider;
class SystemText;
class SystemLockIndicator;
class SoundFlag;

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
class TimeTickAnchor;
class TremoloSingleChord;
class TremoloTwoChord;
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

// dev
class System;
class Measure;
class Segment;
class Chord;
}

namespace mu::engraving::rendering::score {
class TDraw
{
public:

    static void drawItem(const EngravingItem* item, muse::draw::Painter* painter);      // factory

private:
    static void draw(const Accidental* item, muse::draw::Painter* painter);
    static void draw(const ActionIcon* item, muse::draw::Painter* painter);
    static void draw(const Ambitus* item, muse::draw::Painter* painter);
    static void draw(const Arpeggio* item, muse::draw::Painter* painter);
    static void draw(const Articulation* item, muse::draw::Painter* painter);

    static void draw(const BagpipeEmbellishment* item, muse::draw::Painter* painter);
    static void draw(const BarLine* item, muse::draw::Painter* painter);
    static void draw(const Beam* item, muse::draw::Painter* painter);
    static void draw(const Bend* item, muse::draw::Painter* painter);
    static void draw(const Box* item, muse::draw::Painter* painter);
    static void draw(const HBox* item, muse::draw::Painter* painter);
    static void draw(const VBox* item, muse::draw::Painter* painter);
    static void draw(const FBox* item, muse::draw::Painter* painter);
    static void draw(const TBox* item, muse::draw::Painter* painter);
    static void draw(const Bracket* item, muse::draw::Painter* painter);
    static void draw(const Breath* item, muse::draw::Painter* painter);

    static void draw(const ChordLine* item, muse::draw::Painter* painter);
    static void draw(const Clef* item, muse::draw::Painter* painter);
    static void draw(const Capo* item, muse::draw::Painter* painter);

    static void draw(const DeadSlapped* item, muse::draw::Painter* painter);
    static void draw(const Dynamic* item, muse::draw::Painter* painter);

    static void draw(const Expression* item, muse::draw::Painter* painter);

    static void draw(const Fermata* item, muse::draw::Painter* painter);
    static void draw(const FiguredBass* item, muse::draw::Painter* painter);
    static void draw(const FiguredBassItem* item, muse::draw::Painter* painter);
    static void draw(const Fingering* item, muse::draw::Painter* painter);
    static void draw(const FretDiagram* item, muse::draw::Painter* painter);

    static void draw(const GlissandoSegment* item, muse::draw::Painter* painter);
    static void draw(const GradualTempoChangeSegment* item, muse::draw::Painter* painter);
    static void draw(const GuitarBendSegment* item, muse::draw::Painter* painter);
    static void draw(const GuitarBendHoldSegment* item, muse::draw::Painter* painter);

    static void draw(const HairpinSegment* item, muse::draw::Painter* painter);
    static void draw(const HarpPedalDiagram* item, muse::draw::Painter* painter);
    static void draw(const HarmonicMarkSegment* item, muse::draw::Painter* painter);
    static void draw(const Harmony* item, muse::draw::Painter* painter);
    static void draw(const Hook* item, muse::draw::Painter* painter);

    static void draw(const Image* item, muse::draw::Painter* painter);
    static void draw(const InstrumentChange* item, muse::draw::Painter* painter);
    static void draw(const InstrumentName* item, muse::draw::Painter* painter);

    static void draw(const Jump* item, muse::draw::Painter* painter);

    static void draw(const KeySig* item, muse::draw::Painter* painter);

    static void draw(const LaissezVibSegment* item, muse::draw::Painter* painter);
    static void draw(const Lasso* item, muse::draw::Painter* painter);
    static void draw(const LayoutBreak* item, muse::draw::Painter* painter);
    static void draw(const LedgerLine* item, muse::draw::Painter* painter);
    static void draw(const LetRingSegment* item, muse::draw::Painter* painter);
    static void draw(const Lyrics* item, muse::draw::Painter* painter);
    static void draw(const LyricsLineSegment* item, muse::draw::Painter* painter);

    static void draw(const Marker* item, muse::draw::Painter* painter);
    static void draw(const MeasureNumber* item, muse::draw::Painter* painter);
    static void draw(const MeasureRepeat* item, muse::draw::Painter* painter);
    static void draw(const MMRest* item, muse::draw::Painter* painter);
    static void draw(const MMRestRange* item, muse::draw::Painter* painter);

    static void draw(const Note* item, muse::draw::Painter* painter);
    static void draw(const NoteDot* item, muse::draw::Painter* painter);
    static void draw(const NoteHead* item, muse::draw::Painter* painter);
    static void draw(const NoteLineSegment* item, muse::draw::Painter* painter);

    static void draw(const Ornament* item, muse::draw::Painter* painter);
    static void draw(const OttavaSegment* item, muse::draw::Painter* painter);

    static void draw(const Page* item, muse::draw::Painter* painter);
    static void draw(const Parenthesis* item, muse::draw::Painter* painter);
    static void draw(const PartialTieSegment* item, muse::draw::Painter* painter);
    static void draw(const PalmMuteSegment* item, muse::draw::Painter* painter);
    static void draw(const PedalSegment* item, muse::draw::Painter* painter);
    static void draw(const PickScrapeSegment* item, muse::draw::Painter* painter);
    static void draw(const PlayTechAnnotation* item, muse::draw::Painter* painter);

    static void draw(const RasgueadoSegment* item, muse::draw::Painter* painter);
    static void draw(const RehearsalMark* item, muse::draw::Painter* painter);
    static void draw(const Rest* item, muse::draw::Painter* painter);

    static void draw(const ShadowNote* item, muse::draw::Painter* painter);
    static void draw(const SlurSegment* item, muse::draw::Painter* painter);
    static void draw(const Spacer* item, muse::draw::Painter* painter);
    static void draw(const StaffLines* item, muse::draw::Painter* painter);
    static void draw(const StaffState* item, muse::draw::Painter* painter);
    static void draw(const StaffText* item, muse::draw::Painter* painter);
    static void draw(const StaffTypeChange* item, muse::draw::Painter* painter);
    static void draw(const Stem* item, muse::draw::Painter* painter);
    static void draw(const StemSlash* item, muse::draw::Painter* painter);
    static void draw(const Sticking* item, muse::draw::Painter* painter);
    static void draw(const StringTunings* item, muse::draw::Painter* painter);
    static void draw(const Symbol* item, muse::draw::Painter* painter);
    static void draw(const FSymbol* item, muse::draw::Painter* painter);
    static void draw(const SystemDivider* item, muse::draw::Painter* painter);
    static void draw(const SystemText* item, muse::draw::Painter* painter);
    static void draw(const SystemLockIndicator* item, muse::draw::Painter* painter);
    static void draw(const SoundFlag* item, muse::draw::Painter* painter);

    static void draw(const TabDurationSymbol* item, muse::draw::Painter* painter);
    static void draw(const TempoText* item, muse::draw::Painter* painter);
    static void draw(const Text* item, muse::draw::Painter* painter);
    static void draw(const TextLineSegment* item, muse::draw::Painter* painter);
    static void draw(const TieSegment* item, muse::draw::Painter* painter);
    static void draw(const TimeSig* item, muse::draw::Painter* painter);
    static void draw(const TimeTickAnchor* item, muse::draw::Painter* painter);
    static void draw(const TremoloSingleChord* item, muse::draw::Painter* painter);
    static void draw(const TremoloTwoChord* item, muse::draw::Painter* painter);
    static void draw(const TremoloBar* item, muse::draw::Painter* painter);
    static void draw(const TrillSegment* item, muse::draw::Painter* painter);
    static void draw(const TripletFeel* item, muse::draw::Painter* painter);
    static void draw(const Tuplet* item, muse::draw::Painter* painter);

    static void draw(const VibratoSegment* item, muse::draw::Painter* painter);
    static void draw(const VoltaSegment* item, muse::draw::Painter* painter);

    static void draw(const WhammyBarSegment* item, muse::draw::Painter* painter);

    static void drawTextBase(const TextBase* item, muse::draw::Painter* painter);
    static void drawTextLineBaseSegment(const TextLineBaseSegment* item, muse::draw::Painter* painter);

    // dev
    static void draw(const System* item, muse::draw::Painter* painter);
    static void draw(const Measure* item, muse::draw::Painter* painter);
    static void draw(const Segment* item, muse::draw::Painter* painter);
    static void draw(const Chord* item, muse::draw::Painter* painter);

    static void setMask(const EngravingItem* item, muse::draw::Painter* painter);
};
}
