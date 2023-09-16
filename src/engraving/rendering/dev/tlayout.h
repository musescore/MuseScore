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
#ifndef MU_ENGRAVING_TLAYOUT_DEV_H
#define MU_ENGRAVING_TLAYOUT_DEV_H

#include "layoutcontext.h"

#include "../../dom/accidental.h"
#include "../../dom/actionicon.h"
#include "../../dom/ambitus.h"
#include "../../dom/arpeggio.h"
#include "../../dom/articulation.h"

#include "../../dom/barline.h"
#include "../../dom/bend.h"
#include "../../dom/box.h"
#include "../../dom/bracket.h"
#include "../../dom/breath.h"

#include "../../dom/chordline.h"
#include "../../dom/clef.h"
#include "../../dom/capo.h"

#include "../../dom/deadslapped.h"
#include "../../dom/dynamic.h"

#include "../../dom/expression.h"

#include "../../dom/fermata.h"

#include "../../dom/measurebase.h"
#include "../../dom/measurenumberbase.h"

#include "../../dom/textbase.h"

#include "../../dom/ornament.h"

namespace mu::engraving {
class EngravingItem;

class BagpipeEmbellishment;
class Beam;

class Chord;

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
class MeasureNumber;
class MeasureRepeat;
class MMRest;
class MMRestRange;

class Note;
class NoteDot;

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
class TLayout
{
public:

    static void layoutItem(EngravingItem* item, LayoutContext& ctx);  // factory

    static void layout(const Accidental* item, Accidental::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layout(const ActionIcon* item, ActionIcon::LayoutData* ldata);
    static void layout(const Ambitus* item, Ambitus::LayoutData* ldata, const LayoutContext& ctx);
    static void layout(const Arpeggio* item, Arpeggio::LayoutData* ldata, const LayoutConfiguration& conf,
                       bool includeCrossStaffHeight = false);
    static void layout(const Articulation* item, Articulation::LayoutData* ldata);

    static void layout(const BarLine* item, BarLine::LayoutData* ldata, const LayoutContext& ctx);
    static void layout2(BarLine* item, LayoutContext& ctx);
    static void layout(Beam* item, LayoutContext& ctx);
    static void layout1(Beam* item, LayoutContext& ctx);
    static void layout(const Bend* item, Bend::LayoutData* ldata);

    static void layout(const Box* item, Box::LayoutData* ldata, const LayoutContext& ctx); // factory
    static void layoutBox(const Box* item, Box::LayoutData* ldata, const LayoutContext& ctx); // base class
    static void layout(const HBox* item, HBox::LayoutData* ldata, const LayoutContext& ctx);
    static void layout2(HBox* item, LayoutContext& ctx);
    static void layout(const VBox* item, VBox::LayoutData* ldata, const LayoutContext& ctx);
    static void layout(const FBox* item, FBox::LayoutData* ldata, const LayoutContext& ctx);
    static void layout(const TBox* item, FBox::LayoutData* ldata, const LayoutContext& ctx);

    static void layout(const Bracket* item, Bracket::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layout(const Breath* item, Breath::LayoutData* ldata, const LayoutConfiguration& conf);

    static void layout(Chord* item, LayoutContext& ctx);
    static void layout(const ChordLine* item, ChordLine::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layout(const Clef* item, Clef::LayoutData* ldata);
    static void layout(const Capo* item, Capo::LayoutData* ldata, const LayoutContext& ctx);

    static void layout(const DeadSlapped* item, DeadSlapped::LayoutData* ldata);
    static void layout(const Dynamic* item, Dynamic::LayoutData* ldata, const LayoutConfiguration& conf);

    static void layout(const Expression* item, Expression::LayoutData* ldata);

    static void layout(const Fermata* item, Fermata::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layout(FiguredBassItem* item, LayoutContext& ctx);
    static void layout(FiguredBass* item, LayoutContext& ctx);
    static void layout(Fingering* item, LayoutContext& ctx);
    static void layout(FretDiagram* item, LayoutContext& ctx);
    static void layout(FretCircle* item, LayoutContext& ctx);

    static void layout(Glissando* item, LayoutContext& ctx);
    static void layout(GlissandoSegment* item, LayoutContext& ctx);
    static void layout(GraceNotesGroup* item, LayoutContext& ctx);
    static void layout(GradualTempoChangeSegment* item, LayoutContext& ctx);
    static void layout(GradualTempoChange* item, LayoutContext& ctx);

    static void layout(HairpinSegment* item, LayoutContext& ctx);
    static void layout(Hairpin* item, LayoutContext& ctx);
    static void layout(HarpPedalDiagram* item, LayoutContext& ctx);
    static void layout(HarmonicMarkSegment* item, LayoutContext& ctx);
    static void layout(Harmony* item, LayoutContext& ctx);
    static void layout1(Harmony* item, const LayoutContext& ctx);
    static void layout(Hook* item, LayoutContext& ctx);

    static void layout(Image* item, LayoutContext& ctx);
    static void layout(InstrumentChange* item, LayoutContext& ctx);
    static void layout(InstrumentName* item, LayoutContext& ctx);

    static void layout(Jump* item, LayoutContext& ctx);

    static void layout(KeySig* item, LayoutContext& ctx);

    static void layout(LayoutBreak* item, LayoutContext& ctx);
    static void layout(LedgerLine* item, LayoutContext& ctx);
    static void layout(LetRing* item, LayoutContext& ctx);
    static void layout(LetRingSegment* item, LayoutContext& ctx);
    static void layout(LineSegment* item, LayoutContext& ctx);  // factory
    static void layout(Lyrics* item, LayoutContext& ctx);
    static void layout(LyricsLine* item, LayoutContext& ctx);
    static void layout(LyricsLineSegment* item, LayoutContext& ctx);

    static void layout(Marker* item, LayoutContext& ctx);
    static void layout(MeasureBase* item, LayoutContext& ctx); // factory
    static void layoutMeasureBase(const MeasureBase* item, MeasureBase::LayoutData* ldata, const LayoutContext& ctx); // base class
    static void layout(MeasureNumber* item, LayoutContext& ctx);
    static void layoutMeasureNumberBase(const MeasureNumberBase* item, const LayoutContext& ctx, TextBase::LayoutData* ldata); // base class
    static void layout(MeasureRepeat* item, LayoutContext& ctx);
    static void layout(MMRest* item, LayoutContext& ctx);
    static void layout(MMRestRange* item, LayoutContext& ctx);

    static void layout(Note* item, LayoutContext& ctx);
    static void layout(NoteDot* item, LayoutContext& ctx);

    static void layout(const Ornament* item, Ornament::LayoutData* ldata, const LayoutConfiguration& conf);
    static void layoutOrnamentCueNote(Ornament* item, LayoutContext& ctx);

    static void layout(Ottava* item, LayoutContext& ctx);
    static void layout(OttavaSegment* item, LayoutContext& ctx);

    static void layout(PalmMute* item, LayoutContext& ctx);
    static void layout(PalmMuteSegment* item, LayoutContext& ctx);
    static void layout(Pedal* item, LayoutContext& ctx);
    static void layout(PedalSegment* item, LayoutContext& ctx);
    static void layout(PickScrapeSegment* item, LayoutContext& ctx);
    static void layout(PlayTechAnnotation* item, LayoutContext& ctx);

    static void layout(RasgueadoSegment* item, LayoutContext& ctx);
    static void layout(RehearsalMark* item, LayoutContext& ctx);
    static void layout(Rest* item, LayoutContext& ctx);

    static void layout(ShadowNote* item, LayoutContext& ctx);
    static void layoutLine(SLine* item, LayoutContext& ctx); // base class
    static void layout(Slur* item, LayoutContext& ctx);
    static void layout(Spacer* item, LayoutContext& ctx);
    static void layout(Spanner* item, LayoutContext& ctx);
    static void layout(StaffLines* item, LayoutContext& ctx);
    static void layoutForWidth(StaffLines* item, double w, LayoutContext& ctx);
    static void layout(StaffState* item, LayoutContext& ctx);
    static void layout(StaffText* item, LayoutContext& ctx);
    static void layout(StaffTypeChange* item, LayoutContext& ctx);
    static void layout(Stem* item, LayoutContext& ctx);
    static void layout(StemSlash* item, LayoutContext& ctx);
    static void layout(Sticking* item, LayoutContext& ctx);
    static void layout(StretchedBend* item, LayoutContext& ctx);
    static void layoutStretched(StretchedBend* item, LayoutContext& ctx);

    static void layout(Symbol* item, LayoutContext& ctx);
    static void layout(FSymbol* item, LayoutContext& ctx);

    static void layout(SystemDivider* item, LayoutContext& ctx);
    static void layout(SystemText* item, LayoutContext& ctx);

    static void layout(TabDurationSymbol* item, LayoutContext& ctx);
    static void layout(TempoText* item, LayoutContext& ctx);

    static void layout(TextBase* item, LayoutContext& ctx);                 // factory
    static void layoutTextBase(const TextBase* item, TextBase::LayoutData* data); // base class
    static void layoutTextBase(TextBase* item, LayoutContext& ctx);  // base class
    static void layout1TextBase(TextBase* item, const LayoutContext& ctx);  // base class
    static void layout1TextBase(const TextBase* item, TextBase::LayoutData* data);

    static void layout(Text* item, LayoutContext& ctx);

    static void layout(TextLine* item, LayoutContext& ctx);
    static void layout(TextLineSegment* item, LayoutContext& ctx);
    static void layoutTextLineBase(TextLineBase* item, LayoutContext& ctx);
    static void layoutTextLineBaseSegment(TextLineBaseSegment* item, LayoutContext& ctx); // base class
    static void layout(Tie* item, LayoutContext& ctx);
    static void layout(TimeSig* item, LayoutContext& ctx);
    static void layout(Tremolo* item, LayoutContext& ctx);
    static void layout(TremoloBar* item, LayoutContext& ctx);
    static void layout(Trill* item, LayoutContext& ctx);
    static void layout(TrillSegment* item, LayoutContext& ctx);
    static void layout(TripletFeel* item, LayoutContext& ctx);
    static void layout(Tuplet* item, LayoutContext& ctx);

    static void layout(Vibrato* item, LayoutContext& ctx);
    static void layout(VibratoSegment* item, LayoutContext& ctx);
    static void layout(Volta* item, LayoutContext& ctx);
    static void layout(VoltaSegment* item, LayoutContext& ctx);

    static void layout(WhammyBarSegment* item, LayoutContext& ctx);

    static RectF layoutRect(const BarLine* item, LayoutContext& ctx);

    // layoutSystem;
    static SpannerSegment* layoutSystem(Spanner* item, System* system, LayoutContext& ctx); // factory
    static SpannerSegment* layoutSystem(LyricsLine* line, System* system, LayoutContext& ctx);
    static SpannerSegment* layoutSystem(Volta* line, System* system, LayoutContext& ctx);
    static SpannerSegment* layoutSystem(Slur* line, System* system, LayoutContext& ctx);
    static void layoutSystemsDone(Spanner* item);

private:

    friend class SlurTieLayout;

    static PointF calculateBoundingRect(Harmony* item, const LayoutContext& ctx);

    static void keySigAddLayout(KeySig* item, LayoutContext& ctx, SymId sym, int line);

    static SpannerSegment* layoutSystemSLine(SLine* line, System* system, LayoutContext& ctx);
    static SpannerSegment* getNextLayoutSystemSegment(Spanner* spanner, System* system,
                                                      std::function<SpannerSegment* (System* parent)> createSegment);
};
}

#endif // MU_ENGRAVING_TLAYOUT_DEV_H
