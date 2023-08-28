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
#ifndef MU_ENGRAVING_SINGLEDRAW_H
#define MU_ENGRAVING_SINGLEDRAW_H

namespace mu::draw {
class Painter;
}

namespace mu::engraving {
class MStyle;
class IEngravingFont;
class Score;
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
class Bracket;
class Breath;

class Capo;
class Chord;
class ChordLine;
class Clef;

class DeadSlapped;
class Dynamic;

class Expression;

class Fermata;
class Fingering;
class FiguredBass;
class FiguredBassItem;
class FretDiagram;
class FSymbol;

class Glissando;
class GlissandoSegment;
class GradualTempoChange;
class GradualTempoChangeSegment;

class Hairpin;
class HairpinSegment;
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
class SLine;
class LineSegment;

class Marker;
class MeasureNumber;
class MeasureRepeat;

class Note;
class NoteHead;

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
class Slur;
class SlurSegment;
class Spacer;
class StaffLines;
class StaffState;
class StaffText;
class StaffTypeChange;
class Stem;
class StemSlash;
class StretchedBend;
class Sticking;
class Symbol;
class SystemText;
class SystemDivider;

class TabDurationSymbol;
class TempoText;
class Text;
class TextBase;
class TextLine;
class TextLineSegment;
class TextLineBaseSegment;
class TieSegment;
class TimeSig;
class Tremolo;
class TremoloBar;
class Trill;
class TrillSegment;
class TripletFeel;
class Tuplet;

class Vibrato;
class VibratoSegment;
class Volta;
class VoltaSegment;

class WhammyBarSegment;
}

namespace mu::engraving::rendering::single {
class SingleDraw
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
    static void draw(const FiguredBassItem* item, draw::Painter* painter);
    static void draw(const Fingering* item, draw::Painter* painter);
    static void draw(const FretDiagram* item, draw::Painter* painter);

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

    static void draw(const Marker* item, draw::Painter* painter);
    static void draw(const MeasureNumber* item, draw::Painter* painter);
    static void draw(const MeasureRepeat* item, draw::Painter* painter);

    static void draw(const Note* item, draw::Painter* painter);
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
    static void draw(const StaffLines* item, draw::Painter* painter);
    static void draw(const StaffState* item, draw::Painter* painter);
    static void draw(const StaffText* item, draw::Painter* painter);
    static void draw(const StaffTypeChange* item, draw::Painter* painter);
    static void draw(const Stem* item, draw::Painter* painter);
    static void draw(const StemSlash* item, draw::Painter* painter);
    static void draw(const Sticking* item, draw::Painter* painter);
    static void draw(const StretchedBend* item, draw::Painter* painter);
    static void draw(const Symbol* item, draw::Painter* painter);
    static void draw(const FSymbol* item, draw::Painter* painter);
    static void draw(const SystemDivider* item, draw::Painter* painter);
    static void draw(const SystemText* item, draw::Painter* painter);

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
    static void draw(const VoltaSegment* item, draw::Painter* painter);

    static void draw(const WhammyBarSegment* item, draw::Painter* painter);

    static void drawTextBase(const TextBase* item, draw::Painter* painter);
    static void drawTextLineBaseSegment(const TextLineBaseSegment* item, draw::Painter* painter);
};
}

#endif // MU_ENGRAVING_SINGLEDRAW_H
