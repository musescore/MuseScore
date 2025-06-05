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
#ifndef MU_ENGRAVING_SINGLELAYOUT_H
#define MU_ENGRAVING_SINGLELAYOUT_H

#include <memory>

#include "dom/textbase.h"

namespace mu::engraving {
class MStyle;
class IEngravingFont;
class EngravingItem;
class Score;

class Accidental;
class ActionIcon;
class Ambitus;
class Arpeggio;
class Articulation;

class BagpipeEmbellishment;
class BarLine;
class Bend;
class Bracket;
class Breath;

class Capo;
class Chord;
class ChordLine;
class Clef;

class Dynamic;

class Expression;
class Footnote;

class Fermata;
class Fingering;
class FretDiagram;
class FSymbol;

class Glissando;
class GlissandoSegment;
class GradualTempoChange;
class GradualTempoChangeSegment;

class Hairpin;
class HairpinSegment;
class HarpPedalDiagram;

class Image;
class InstrumentChange;

class Jump;

class KeySig;

class LayoutBreak;
class LetRing;
class LetRingSegment;
class SLine;
class LineSegment;
class Lyrics;

class Marker;
class MeasureNumber;
class MeasureRepeat;

class NoteHead;
class NoteLine;

class Ornament;
class Ottava;
class OttavaSegment;

class PalmMute;
class PalmMuteSegment;
class Pedal;
class PedalSegment;
class PlayTechAnnotation;

class RehearsalMark;

class Slur;
class SoundFlag;
class Spacer;
class StaffText;
class StaffTypeChange;
class Sticking;
class StringTunings;
class Symbol;
class SystemText;

class TempoText;
class Text;
class TextLine;
class TextLineSegment;
class TextLineBaseSegment;
class TimeSig;
class TremoloSingleChord;
class TremoloTwoChord;
class TremoloBar;
class Trill;
class TrillSegment;

class Vibrato;
class VibratoSegment;
class Volta;
class VoltaSegment;
}

namespace mu::engraving::compat {
class DummyElement;
}

namespace mu::engraving::rendering::single {
class SingleLayout
{
public:
    SingleLayout() = default;

    static void layoutItem(engraving::EngravingItem* item);

    struct Context {
        Context(engraving::Score* s)
            : m_score(s) {}

        const engraving::MStyle& style() const;
        std::shared_ptr<engraving::IEngravingFont> engravingFont() const;
        engraving::compat::DummyElement* dummyParent() const;

        //! NOTE Temporarily, do not use
        engraving::Score* dontUseScore() const { return m_score; }

    private:
        engraving::Score* m_score = nullptr;
    };

    static void layout(Accidental* item, const Context& ctx);
    static void layout(ActionIcon* item, const Context& ctx);
    static void layout(Ambitus* item, const Context& ctx);
    static void layout(Arpeggio* item, const Context& ctx);
    static void layout(Articulation* item, const Context& ctx);

    static void layout(BagpipeEmbellishment* item, const Context& ctx);
    static void layout(BarLine* item, const Context& ctx);
    static void layout(Bend* item, const Context& ctx);
    static void layout(Bracket* item, const Context& ctx);
    static void layout(Breath* item, const Context&);

    static void layout(Capo* item, const Context& ctx);
    static void layout(Chord* item, const Context& ctx);
    static void layout(ChordLine* item, const Context& ctx);
    static void layout(Clef* item, const Context& ctx);

    static void layout(Dynamic* item, const Context& ctx);

    static void layout(Expression* item, const Context& ctx);
    static void layout(Footnote* item, const Context& ctx);

    static void layout(Fermata* item, const Context& ctx);
    static void layout(Fingering* item, const Context& ctx);
    static void layout(FretDiagram* item, const Context& ctx);
    static void layout(FSymbol* item, const Context& ctx);

    static void layout(Glissando* item, const Context& ctx);
    static void layout(GradualTempoChange* item, const Context& ctx);
    static void layout(GuitarBend* item, const Context& ctx);

    static void layout(Hairpin* item, const Context& ctx);
    static void layout(HammerOnPullOff* item, const Context& ctx);
    static void layout(HammerOnPullOffSegment* item, const Context& ctx);
    static void layout(HarpPedalDiagram* item, const Context& ctx);

    static void layout(Image* item, const Context& ctx);
    static void layout(InstrumentChange* item, const Context& ctx);

    static void layout(Jump* item, const Context& ctx);

    static void layout(KeySig* item, const Context& ctx);

    static void layout(LayoutBreak* item, const Context& ctx);
    static void layout(LetRing* item, const Context& ctx);
    static void layout(Lyrics* item, const Context& ctx);

    static void layout(NoteHead* item, const Context& ctx);
    static void layout(NoteLine* item, const Context& ctx);

    static void layout(Marker* item, const Context& ctx);
    static void layout(MeasureNumber* item, const Context& ctx);
    static void layout(MeasureRepeat* item, const Context& ctx);

    static void layout(Ornament* item, const Context& ctx);
    static void layout(Ottava* item, const Context& ctx);

    static void layout(PalmMute* item, const Context& ctx);
    static void layout(Pedal* item, const Context& ctx);
    static void layout(PlayTechAnnotation* item, const Context& ctx);

    static void layout(RehearsalMark* item, const Context& ctx);

    static void layout(Slur* item, const Context& ctx);
    static void layout(SoundFlag* item, const Context& ctx);
    static void layout(Spacer* item, const Context&);
    static void layout(StaffText* item, const Context& ctx);
    static void layout(StaffTypeChange* item, const Context& ctx);
    static void layout(Stem* item, const Context& ctx);
    static void layout(Sticking* item, const Context& ctx);
    static void layout(StringTunings* item, const Context& ctx);
    static void layout(Symbol* item, const Context& ctx);
    static void layout(SystemText* item, const Context& ctx);

    static void layout(TempoText* item, const Context& ctx);
    static void layout(TextLine* item, const Context& ctx);
    static void layout(TimeSig* item, const Context& ctx);
    static void layout(TremoloSingleChord* item, const Context& ctx);
    static void layout(TremoloTwoChord* item, const Context& ctx);
    static void layout(TremoloBar* item, const Context& ctx);
    static void layout(Trill* item, const Context& ctx);

    static void layout(Vibrato* item, const Context& ctx);
    static void layout(Volta* item, const Context& ctx);

private:
    static void layout(GlissandoSegment* item, const Context& ctx);
    static void layout(GradualTempoChangeSegment* item, const Context& ctx);
    static void layout(GuitarBendSegment* item, const Context& ctx);
    static void layout(HairpinSegment* item, const Context& ctx);
    static void layout(LetRingSegment* item, const Context& ctx);
    static void layout(OttavaSegment* item, const Context& ctx);
    static void layout(PalmMuteSegment* item, const Context& ctx);
    static void layout(PedalSegment* item, const Context& ctx);
    static void layout(TextLineSegment* item, const Context& ctx);
    static void layout(TrillSegment* item, const Context& ctx);
    static void layout(VibratoSegment* item, const Context& ctx);
    static void layout(VoltaSegment* item, const Context& ctx);

    static void layout(Text* item, const Context& ctx);
    static void layoutTextBase(const TextBase* item, const Context& ctx, TextBase::LayoutData* ldata);
    static void layout1TextBase(const TextBase* item, const Context& ctx, TextBase::LayoutData* ldata);

    static void layoutLine(SLine* item, const Context& ctx);
    static void layoutLineSegment(LineSegment* item, const Context& ctx);
    static void layoutTextLineBaseSegment(TextLineBaseSegment* item, const Context& ctx);
};
}

#endif // MU_ENGRAVING_SINGLELAYOUT_H
