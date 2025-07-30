/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

namespace muse::draw {
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
class GuitarBend;
class GuitarBendSegment;

class Hairpin;
class HairpinSegment;
class HammerOnPullOffSegment;
class HammerOnPullOffText;
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
class Lyrics;

class Marker;
class MeasureNumber;
class MeasureRepeat;

class Note;
class NoteHead;
class NoteLineSegment;

class Ornament;
class Ottava;
class OttavaSegment;

class PalmMute;
class PalmMuteSegment;
class Pedal;
class PedalSegment;
class PickScrapeSegment;
class PlayCountText;
class PlayTechAnnotation;

class RasgueadoSegment;
class RehearsalMark;
class Rest;

class ShadowNote;
class Slur;
class SlurSegment;
class SoundFlag;
class Spacer;
class StaffLines;
class StaffState;
class StaffText;
class StaffTypeChange;
class Stem;
class StemSlash;
class Sticking;
class StringTunings;
class Symbol;
class SystemDivider;
class SystemText;

class TabDurationSymbol;
class Tapping;
class TempoText;
class Text;
class TextBase;
class TextLine;
class TextLineSegment;
class TextLineBaseSegment;
class TieSegment;
class TimeSig;
class TremoloSingleChord;
class TremoloTwoChord;
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

    static void draw(const HairpinSegment* item, muse::draw::Painter* painter);
    static void draw(const HammerOnPullOffSegment* item, muse::draw::Painter* painter);
    static void draw(const HammerOnPullOffText* item, muse::draw::Painter* painter);
    static void draw(const HarpPedalDiagram* item, muse::draw::Painter* painter);
    static void draw(const HarmonicMarkSegment* item, muse::draw::Painter* painter);
    static void draw(const Harmony* item, muse::draw::Painter* painter);
    static void draw(const Hook* item, muse::draw::Painter* painter);

    static void draw(const Image* item, muse::draw::Painter* painter);
    static void draw(const InstrumentChange* item, muse::draw::Painter* painter);
    static void draw(const InstrumentName* item, muse::draw::Painter* painter);

    static void draw(const Jump* item, muse::draw::Painter* painter);

    static void draw(const KeySig* item, muse::draw::Painter* painter);

    static void draw(const LayoutBreak* item, muse::draw::Painter* painter);
    static void draw(const LedgerLine* item, muse::draw::Painter* painter);
    static void draw(const LetRingSegment* item, muse::draw::Painter* painter);
    static void draw(const Lyrics* item, muse::draw::Painter* painter);

    static void draw(const Marker* item, muse::draw::Painter* painter);
    static void draw(const MeasureNumber* item, muse::draw::Painter* painter);
    static void draw(const MeasureRepeat* item, muse::draw::Painter* painter);

    static void draw(const Note* item, muse::draw::Painter* painter);
    static void draw(const NoteHead* item, muse::draw::Painter* painter);
    static void draw(const NoteLineSegment* item, muse::draw::Painter* painter);

    static void draw(const Ornament* item, muse::draw::Painter* painter);
    static void draw(const OttavaSegment* item, muse::draw::Painter* painter);

    static void draw(const PalmMuteSegment* item, muse::draw::Painter* painter);
    static void draw(const PedalSegment* item, muse::draw::Painter* painter);
    static void draw(const PickScrapeSegment* item, muse::draw::Painter* painter);
    static void draw(const PlayCountText* item, muse::draw::Painter* painter);
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
    static void draw(const SoundFlag* item, muse::draw::Painter* painter);

    static void draw(const Tapping* item, muse::draw::Painter* painter);
    static void draw(const TempoText* item, muse::draw::Painter* painter);
    static void draw(const Text* item, muse::draw::Painter* painter);
    static void draw(const TextLineSegment* item, muse::draw::Painter* painter);
    static void draw(const TieSegment* item, muse::draw::Painter* painter);
    static void draw(const TimeSig* item, muse::draw::Painter* painter);
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
};
}

#endif // MU_ENGRAVING_SINGLEDRAW_H
