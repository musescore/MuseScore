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
#ifndef MU_PALETTE_PALETTELAYOUT_H
#define MU_PALETTE_PALETTELAYOUT_H

#include <memory>

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
class ChordLine;
class Clef;

class Dynamic;

class Expression;

class Fermata;
class Fingering;
class FretDiagram;

class Glissando;
class GlissandoSegment;
class GradualTempoChange;
class GradualTempoChangeSegment;

class Hairpin;
class HairpinSegment;
class HarpPedalDiagram;

class InstrumentChange;

class Jump;

class KeySig;

class LayoutBreak;
class LetRing;
class LetRingSegment;
class SLine;
class LineSegment;

class Marker;
class MeasureNumber;
class MeasureRepeat;

class NoteHead;

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
class Spacer;
class StaffText;
class Symbol;
class SystemText;

class TempoText;
class Text;
class TextBase;
class TextLine;
class TextLineSegment;
class TextLineBaseSegment;
class TimeSig;
class Tremolo;
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

namespace mu::palette {
class PaletteLayout
{
public:
    PaletteLayout() = default;

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

    static void layout(engraving::Accidental* item, const Context& ctx);
    static void layout(engraving::ActionIcon* item, const Context& ctx);
    static void layout(engraving::Ambitus* item, const Context& ctx);
    static void layout(engraving::Arpeggio* item, const Context& ctx);
    static void layout(engraving::Articulation* item, const Context& ctx);

    static void layout(engraving::BagpipeEmbellishment* item, const Context& ctx);
    static void layout(engraving::BarLine* item, const Context& ctx);
    static void layout(engraving::Bend* item, const Context& ctx);
    static void layout(engraving::Bracket* item, const Context& ctx);
    static void layout(engraving::Breath* item, const Context&);

    static void layout(engraving::Capo* item, const Context& ctx);
    static void layout(engraving::ChordLine* item, const Context& ctx);
    static void layout(engraving::Clef* item, const Context& ctx);

    static void layout(engraving::Dynamic* item, const Context& ctx);

    static void layout(engraving::Expression* item, const Context& ctx);

    static void layout(engraving::Fermata* item, const Context& ctx);
    static void layout(engraving::Fingering* item, const Context& ctx);
    static void layout(engraving::FretDiagram* item, const Context& ctx);

    static void layout(engraving::Glissando* item, const Context& ctx);
    static void layout(engraving::GradualTempoChange* item, const Context& ctx);

    static void layout(engraving::Hairpin* item, const Context& ctx);
    static void layout(engraving::HarpPedalDiagram* item, const Context& ctx);

    static void layout(engraving::InstrumentChange* item, const Context& ctx);

    static void layout(engraving::Jump* item, const Context& ctx);

    static void layout(engraving::KeySig* item, const Context& ctx);

    static void layout(engraving::LayoutBreak* item, const Context& ctx);
    static void layout(engraving::LetRing* item, const Context& ctx);

    static void layout(engraving::NoteHead* item, const Context& ctx);

    static void layout(engraving::Marker* item, const Context& ctx);
    static void layout(engraving::MeasureNumber* item, const Context& ctx);
    static void layout(engraving::MeasureRepeat* item, const Context& ctx);

    static void layout(engraving::Ornament* item, const Context& ctx);
    static void layout(engraving::Ottava* item, const Context& ctx);

    static void layout(engraving::PalmMute* item, const Context& ctx);
    static void layout(engraving::Pedal* item, const Context& ctx);
    static void layout(engraving::PlayTechAnnotation* item, const Context& ctx);

    static void layout(engraving::RehearsalMark* item, const Context& ctx);

    static void layout(engraving::Slur* item, const Context& ctx);
    static void layout(engraving::Spacer* item, const Context&);
    static void layout(engraving::StaffText* item, const Context& ctx);
    static void layout(engraving::Symbol* item, const Context& ctx);
    static void layout(engraving::SystemText* item, const Context& ctx);

    static void layout(engraving::TempoText* item, const Context& ctx);
    static void layout(engraving::TextLine* item, const Context& ctx);
    static void layout(engraving::TimeSig* item, const Context& ctx);
    static void layout(engraving::Tremolo* item, const Context& ctx);
    static void layout(engraving::TremoloBar* item, const Context& ctx);
    static void layout(engraving::Trill* item, const Context& ctx);

    static void layout(engraving::Vibrato* item, const Context& ctx);
    static void layout(engraving::Volta* item, const Context& ctx);

private:
    static void layout(engraving::GlissandoSegment* item, const Context& ctx);
    static void layout(engraving::GradualTempoChangeSegment* item, const Context& ctx);
    static void layout(engraving::HairpinSegment* item, const Context& ctx);
    static void layout(engraving::LetRingSegment* item, const Context& ctx);
    static void layout(engraving::OttavaSegment* item, const Context& ctx);
    static void layout(engraving::PalmMuteSegment* item, const Context& ctx);
    static void layout(engraving::PedalSegment* item, const Context& ctx);
    static void layout(engraving::TextLineSegment* item, const Context& ctx);
    static void layout(engraving::TrillSegment* item, const Context& ctx);
    static void layout(engraving::VibratoSegment* item, const Context& ctx);
    static void layout(engraving::VoltaSegment* item, const Context& ctx);

    static void layout(engraving::Text* item, const Context& ctx);
    static void layoutTextBase(engraving::TextBase* item, const Context& ctx);
    static void layout1TextBase(engraving::TextBase* item, const Context& ctx);

    static void layoutLine(engraving::SLine* item, const Context& ctx);
    static void layoutLineSegment(engraving::LineSegment* item, const Context& ctx);
    static void layoutTextLineBaseSegment(engraving::TextLineBaseSegment* item, const Context& ctx);
};
}

#endif // MU_PALETTE_PALETTELAYOUT_H
