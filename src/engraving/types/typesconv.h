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

#pragma once

#include "types/string.h"
#include "types.h"

namespace mu::engraving {
class TConv
{
public:
    TConv() = default;

    static String toXml(const std::vector<int>& v);
    static std::vector<int> fromXml(const String& tag, const std::vector<int>& def);

    static String toXml(const std::vector<string_idx_t>& v);
    static std::vector<string_idx_t> fromXml(const String& tag, const std::vector<string_idx_t>& def);

    static const TranslatableString& userName(ElementType v, bool plural = false);

    static AsciiStringView toXml(ElementType v);
    static ElementType fromXml(const AsciiStringView& tag, ElementType def, bool silent = false);

    static String toXml(Align v);
    static Align fromXml(const String& str, Align def);
    static AlignH fromXml(const AsciiStringView& str, AlignH def);
    static AlignV fromXml(const AsciiStringView& str, AlignV def);

    static String toXml(OrnamentInterval interval);
    static OrnamentInterval fromXml(const String& str, OrnamentInterval def);
    static IntervalStep fromXml(const AsciiStringView& tag, IntervalStep def);
    static IntervalType fromXml(const AsciiStringView& tag, IntervalType def);

    static String translatedUserName(SymId v);
    static AsciiStringView toXml(SymId v);
    static SymId fromXml(const AsciiStringView& tag, SymId def);

    static String translatedUserName(Orientation v);
    static AsciiStringView toXml(Orientation v);
    static Orientation fromXml(const AsciiStringView& tag, Orientation def);

    static String translatedUserName(NoteHeadType v);
    static AsciiStringView toXml(NoteHeadType v);
    static NoteHeadType fromXml(const AsciiStringView& tag, NoteHeadType def);
    static String translatedUserName(NoteHeadScheme v);
    static AsciiStringView toXml(NoteHeadScheme v);
    static NoteHeadScheme fromXml(const AsciiStringView& tag, NoteHeadScheme def);
    static const TranslatableString& userName(NoteHeadGroup v);
    static String translatedUserName(NoteHeadGroup v);
    static AsciiStringView toXml(NoteHeadGroup v);
    static NoteHeadGroup fromXml(const AsciiStringView& tag, NoteHeadGroup def);

    static const TranslatableString& userName(ClefType v);
    static String translatedUserName(ClefType v);
    static AsciiStringView toXml(ClefType v);
    static ClefType fromXml(const AsciiStringView& tag, ClefType def);

    static SymId symId(DynamicType v);
    static DynamicType dynamicType(SymId v);
    static DynamicType dynamicType(const AsciiStringView& string);
    static bool dynamicValid(const AsciiStringView& tag);
    static const TranslatableString& userName(DynamicType v);
    static String translatedUserName(DynamicType v);
    static AsciiStringView toXml(DynamicType v);
    static DynamicType fromXml(const AsciiStringView& tag, DynamicType def);
    static DynamicRange fromXml(const AsciiStringView& tag, DynamicRange def);
    static String translatedUserName(DynamicSpeed v);
    static AsciiStringView toXml(DynamicSpeed v);
    static DynamicSpeed fromXml(const AsciiStringView& tag, DynamicSpeed def);

    static String translatedUserName(HookType v);
    static String toXml(HookType v);
    static HookType fromXml(const AsciiStringView& tag, HookType def);

    static AsciiStringView toXml(LineType v);
    static LineType fromXml(const AsciiStringView& tag, LineType def);

    static String translatedUserName(KeyMode v);
    static AsciiStringView toXml(KeyMode v);
    static KeyMode fromXml(const AsciiStringView& tag, KeyMode def);

    static const TranslatableString& userName(TextStyleType v);
    static String translatedUserName(TextStyleType v);
    static AsciiStringView toXml(TextStyleType v);
    static TextStyleType fromXml(const AsciiStringView& tag, TextStyleType def);

    static AsciiStringView toXml(ChangeMethod v);
    static ChangeMethod fromXml(const AsciiStringView& tag, ChangeMethod def);
    static std::map<int /*positionTick*/, int> easingValueCurve(const int ticksDuration, const int stepsCount, const int amplitude,
                                                                const ChangeMethod method);
    static std::map<int /*positionTick*/, double> easingValueCurve(const int ticksDuration, const int stepsCount, const double amplitude,
                                                                   const ChangeMethod method);

    static String toXml(const PitchValue& v);

    static const char* userName(AccidentalVal accidental, bool full);
    static String toXml(AccidentalRole v);
    static AccidentalRole fromXml(const AsciiStringView& tag, AccidentalRole def);

    static String toXml(BeatsPerSecond v);
    static BeatsPerSecond fromXml(const AsciiStringView& tag, BeatsPerSecond def);

    static String translatedUserName(DurationType v);
    static AsciiStringView toXml(DurationType v);
    static DurationType fromXml(const AsciiStringView& tag, DurationType def);

    static const TranslatableString& userName(PlayingTechniqueType v);
    static AsciiStringView toXml(PlayingTechniqueType v);
    static PlayingTechniqueType fromXml(const AsciiStringView& tag, PlayingTechniqueType def);

    static const TranslatableString& userName(GradualTempoChangeType v);
    static AsciiStringView toXml(GradualTempoChangeType v);
    static GradualTempoChangeType fromXml(const AsciiStringView& tag, GradualTempoChangeType def);

    static AsciiStringView toXml(OrnamentStyle v);
    static OrnamentStyle fromXml(const AsciiStringView& str, OrnamentStyle def);

    static AsciiStringView toXml(PlacementV v);
    static PlacementV fromXml(const AsciiStringView& str, PlacementV def);
    static AsciiStringView toXml(PlacementH v);
    static PlacementH fromXml(const AsciiStringView& str, PlacementH def);

    static AsciiStringView toXml(TextPlace v);
    static TextPlace fromXml(const AsciiStringView& str, TextPlace def);

    static String translatedUserName(DirectionV v);
    static AsciiStringView toXml(DirectionV v);
    static DirectionV fromXml(const AsciiStringView& str, DirectionV def);
    static String translatedUserName(DirectionH v);
    static AsciiStringView toXml(DirectionH v);
    static DirectionH fromXml(const AsciiStringView& str, DirectionH def);

    static const TranslatableString& userName(LayoutBreakType v);
    static AsciiStringView toXml(LayoutBreakType v);
    static LayoutBreakType fromXml(const AsciiStringView& str, LayoutBreakType def);

    static AsciiStringView toXml(VeloType v);
    static VeloType fromXml(const AsciiStringView& str, VeloType def);

    static AsciiStringView toXml(BeamMode v);
    static BeamMode fromXml(const AsciiStringView& str, BeamMode def);

    static AsciiStringView toXml(GlissandoStyle v);
    static GlissandoStyle fromXml(const AsciiStringView& str, GlissandoStyle def);

    static AsciiStringView toXml(BarLineType v);
    static BarLineType fromXml(const AsciiStringView& str, BarLineType def);

    static const TranslatableString& userName(TremoloType v);
    static AsciiStringView toXml(TremoloType v);
    static TremoloType fromXml(const AsciiStringView& str, TremoloType def);

    static const TranslatableString& userName(BracketType v);
    static String translatedUserName(BracketType v);
    static AsciiStringView toXml(BracketType v);
    static BracketType fromXml(const AsciiStringView& str, BracketType def);

    static const TranslatableString& userName(ArpeggioType v);
    static AsciiStringView toXml(ArpeggioType v);
    static ArpeggioType fromXml(const AsciiStringView& tag, ArpeggioType def);

    static const TranslatableString& userName(EmbellishmentType v);
    static String translatedUserName(EmbellishmentType v);
    static String toXml(EmbellishmentType v);
    static EmbellishmentType fromXml(const AsciiStringView& tag, EmbellishmentType def);
    static StringList embellishmentNotes(EmbellishmentType v);
    static size_t embellishmentsCount();

    static const TranslatableString& userName(ChordLineType v, bool straight, bool wavy);
    static AsciiStringView toXml(ChordLineType v);
    static ChordLineType fromXml(const AsciiStringView& tag, ChordLineType def);

    static const String& userName(DrumNum v);

    static const TranslatableString& userName(GlissandoType v);
    static AsciiStringView toXml(GlissandoType v);
    static GlissandoType fromXml(const AsciiStringView& tag, GlissandoType def);

    static const TranslatableString& userName(JumpType v);
    static String translatedUserName(JumpType v);

    static const TranslatableString& userName(MarkerType v);
    static String translatedUserName(MarkerType v);
    static AsciiStringView toXml(MarkerType v);
    static MarkerType fromXml(const AsciiStringView& tag, MarkerType def);

    static String translatedUserName(StaffGroup v);
    static AsciiStringView toXml(StaffGroup v);
    static StaffGroup fromXml(const AsciiStringView& tag, StaffGroup def);

    static const TranslatableString& userName(TrillType v);
    static String translatedUserName(TrillType v);
    static AsciiStringView toXml(TrillType v);
    static TrillType fromXml(const AsciiStringView& tag, TrillType def);

    static const TranslatableString& userName(VibratoType v);
    static String translatedUserName(VibratoType v);
    static AsciiStringView toXml(VibratoType v);
    static VibratoType fromXml(const AsciiStringView& tag, VibratoType def);

    static const TranslatableString& userName(ArticulationTextType v);
    static String text(ArticulationTextType v);
    static AsciiStringView toXml(ArticulationTextType v);
    static ArticulationTextType fromXml(const AsciiStringView& tag, ArticulationTextType def);

    static AsciiStringView toXml(LyricsSyllabic v);
    static LyricsSyllabic fromXml(const AsciiStringView& tag, LyricsSyllabic def);

    static AsciiStringView toXml(LyricsDashSystemStart v);
    static LyricsDashSystemStart fromXml(const AsciiStringView& tag, LyricsDashSystemStart def);

    static const TranslatableString& userName(Key v, bool isAtonal = false, bool isCustom = false);
    static String translatedUserName(Key v, bool isAtonal = false, bool isCustom = false);

    static AsciiStringView toXml(TiePlacement interval);
    static TiePlacement fromXml(const AsciiStringView& str, TiePlacement def);

    static AsciiStringView toXml(TieDotsPlacement placement);
    static TieDotsPlacement fromXml(const AsciiStringView& str, TieDotsPlacement def);

    static AsciiStringView toXml(VoiceAssignment voiceAppl);
    static VoiceAssignment fromXml(const AsciiStringView& str, VoiceAssignment def);

    static AsciiStringView toXml(AutoOnOff autoOnOff);
    static AutoOnOff fromXml(const AsciiStringView& str, AutoOnOff def);

    static AsciiStringView toXml(PartialSpannerDirection v);
    static PartialSpannerDirection fromXml(const AsciiStringView& str, PartialSpannerDirection def);

    static AsciiStringView toXml(TimeSigPlacement timeSigPos);
    static TimeSigPlacement fromXml(const AsciiStringView& str, TimeSigPlacement def);

    static AsciiStringView toXml(TimeSigStyle timeSigStyle);
    static TimeSigStyle fromXml(const AsciiStringView& str, TimeSigStyle def);

    static AsciiStringView toXml(TimeSigVSMargin timeSigVSMargin);
    static TimeSigVSMargin fromXml(const AsciiStringView& str, TimeSigVSMargin def);
};
}
