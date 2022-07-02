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

#ifndef MU_ENGRAVING_TYPESCONV_H
#define MU_ENGRAVING_TYPESCONV_H

#include "types/string.h"
#include "types.h"

namespace mu::engraving {
class TConv
{
public:
    TConv() = default;

    static String toXml(const std::vector<int>& v);
    static std::vector<int> fromXml(const String& tag, const std::vector<int>& def);

    static String toXml(Align v);
    static Align fromXml(const String& str, Align def);
    static AlignH fromXml(const AsciiStringView& str, AlignH def);
    static AlignV fromXml(const AsciiStringView& str, AlignV def);

    static String toUserName(SymId v);
    static AsciiStringView toXml(SymId v);
    static SymId fromXml(const AsciiStringView& tag, SymId def);

    static String toUserName(Orientation v);
    static AsciiStringView toXml(Orientation v);
    static Orientation fromXml(const AsciiStringView& tag, Orientation def);

    static String toUserName(NoteHeadType v);
    static AsciiStringView toXml(NoteHeadType v);
    static NoteHeadType fromXml(const AsciiStringView& tag, NoteHeadType def);
    static String toUserName(NoteHeadScheme v);
    static AsciiStringView toXml(NoteHeadScheme v);
    static NoteHeadScheme fromXml(const AsciiStringView& tag, NoteHeadScheme def);
    static String toUserName(NoteHeadGroup v);
    static AsciiStringView toXml(NoteHeadGroup v);
    static NoteHeadGroup fromXml(const AsciiStringView& tag, NoteHeadGroup def);

    static String toUserName(ClefType v);
    static AsciiStringView toXml(ClefType v);
    static ClefType fromXml(const AsciiStringView& tag, ClefType def);

    static String toUserName(DynamicType v);
    static SymId symId(DynamicType v);
    static DynamicType dynamicType(SymId v);
    static DynamicType dynamicType(const AsciiStringView& string);
    static AsciiStringView toXml(DynamicType v);
    static DynamicType fromXml(const AsciiStringView& tag, DynamicType def);
    static String toUserName(DynamicRange v);
    static String toXml(DynamicRange v);
    static DynamicRange fromXml(const AsciiStringView& tag, DynamicRange def);
    static String toUserName(DynamicSpeed v);
    static AsciiStringView toXml(DynamicSpeed v);
    static DynamicSpeed fromXml(const AsciiStringView& tag, DynamicSpeed def);

    static String toUserName(HookType v);
    static String toXml(HookType v);
    static HookType fromXml(const AsciiStringView& tag, HookType def);

    static AsciiStringView toXml(LineType v);
    static LineType fromXml(const AsciiStringView& tag, LineType def);

    static String toUserName(KeyMode v);
    static AsciiStringView toXml(KeyMode v);
    static KeyMode fromXml(const AsciiStringView& tag, KeyMode def);

    static String toUserName(TextStyleType v);
    static AsciiStringView toXml(TextStyleType v);
    static TextStyleType fromXml(const AsciiStringView& tag, TextStyleType def);

    static String toUserName(ChangeMethod v);
    static AsciiStringView toXml(ChangeMethod v);
    static ChangeMethod fromXml(const AsciiStringView& tag, ChangeMethod def);
    static std::map<int /*positionTick*/, int> easingValueCurve(const int ticksDuration, const int stepsCount, const int amplitude,
                                                                const ChangeMethod method);
    static std::map<int /*positionTick*/, double> easingValueCurve(const int ticksDuration, const int stepsCount, const double amplitude,
                                                                   const ChangeMethod method);

    static String toXml(const PitchValue& v);

    static String toXml(AccidentalRole v);
    static AccidentalRole fromXml(const AsciiStringView& tag, AccidentalRole def);

    static String toXml(BeatsPerSecond v);
    static BeatsPerSecond fromXml(const AsciiStringView& tag, BeatsPerSecond def);

    static String toUserName(DurationType v);
    static AsciiStringView toXml(DurationType v);
    static DurationType fromXml(const AsciiStringView& tag, DurationType def);

    static AsciiStringView toXml(PlayingTechniqueType v);
    static PlayingTechniqueType fromXml(const AsciiStringView& tag, PlayingTechniqueType def);

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

    static AsciiStringView toXml(DirectionV v);
    static DirectionV fromXml(const AsciiStringView& str, DirectionV def);
    static AsciiStringView toXml(DirectionH v);
    static DirectionH fromXml(const AsciiStringView& str, DirectionH def);

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

    static AsciiStringView toXml(TremoloType v);
    static TremoloType fromXml(const AsciiStringView& str, TremoloType def);

    static AsciiStringView toXml(BracketType v);
    static BracketType fromXml(const AsciiStringView& str, BracketType def);
};
}

#endif // MU_ENGRAVING_TYPESCONV_H
