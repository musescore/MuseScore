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

#include <QString>
#include "types/string.h"
#include "types.h"

namespace mu::engraving {
class TConv
{
public:
    TConv() = default;

    static QString toXml(const std::vector<int>& v);
    static std::vector<int> fromXml(const QString& tag, const std::vector<int>& def);

    static QString toXml(Align v);
    static Align fromXml(const QString& str, Align def);
    static AlignH fromXml(const AsciiString& str, AlignH def);
    static AlignV fromXml(const AsciiString& str, AlignV def);

    static QString toUserName(SymId v);
    static AsciiString toXml(SymId v);
    static SymId fromXml(const AsciiString& tag, SymId def);

    static QString toUserName(Orientation v);
    static AsciiString toXml(Orientation v);
    static Orientation fromXml(const AsciiString& tag, Orientation def);

    static QString toUserName(NoteHeadType v);
    static AsciiString toXml(NoteHeadType v);
    static NoteHeadType fromXml(const AsciiString& tag, NoteHeadType def);
    static QString toUserName(NoteHeadScheme v);
    static AsciiString toXml(NoteHeadScheme v);
    static NoteHeadScheme fromXml(const AsciiString& tag, NoteHeadScheme def);
    static QString toUserName(NoteHeadGroup v);
    static AsciiString toXml(NoteHeadGroup v);
    static NoteHeadGroup fromXml(const AsciiString& tag, NoteHeadGroup def);

    static QString toUserName(ClefType v);
    static AsciiString toXml(ClefType v);
    static ClefType fromXml(const AsciiString& tag, ClefType def);

    static QString toUserName(DynamicType v);
    static SymId symId(DynamicType v);
    static DynamicType dynamicType(SymId v);
    static DynamicType dynamicType(const AsciiString& string);
    static AsciiString toXml(DynamicType v);
    static DynamicType fromXml(const AsciiString& tag, DynamicType def);
    static QString toUserName(DynamicRange v);
    static QString toXml(DynamicRange v);
    static DynamicRange fromXml(const AsciiString& tag, DynamicRange def);
    static QString toUserName(DynamicSpeed v);
    static AsciiString toXml(DynamicSpeed v);
    static DynamicSpeed fromXml(const AsciiString& tag, DynamicSpeed def);

    static QString toUserName(HookType v);
    static QString toXml(HookType v);
    static HookType fromXml(const AsciiString& tag, HookType def);

    static QString toUserName(KeyMode v);
    static AsciiString toXml(KeyMode v);
    static KeyMode fromXml(const AsciiString& tag, KeyMode def);

    static QString toUserName(TextStyleType v);
    static AsciiString toXml(TextStyleType v);
    static TextStyleType fromXml(const AsciiString& tag, TextStyleType def);

    static QString toUserName(ChangeMethod v);
    static AsciiString toXml(ChangeMethod v);
    static ChangeMethod fromXml(const AsciiString& tag, ChangeMethod def);
    static std::map<int /*positionTick*/, int> easingValueCurve(const int ticksDuration, const int stepsCount, const int amplitude,
                                                                const ChangeMethod method);
    static std::map<int /*positionTick*/, double> easingValueCurve(const int ticksDuration, const int stepsCount, const double amplitude,
                                                                   const ChangeMethod method);

    static QString toXml(const PitchValue& v);

    static QString toXml(AccidentalRole v);
    static AccidentalRole fromXml(const AsciiString& tag, AccidentalRole def);

    static QString toXml(BeatsPerSecond v);
    static BeatsPerSecond fromXml(const AsciiString& tag, BeatsPerSecond def);

    static QString toUserName(DurationType v);
    static AsciiString toXml(DurationType v);
    static DurationType fromXml(const AsciiString& tag, DurationType def);

    static AsciiString toXml(PlayingTechniqueType v);
    static PlayingTechniqueType fromXml(const AsciiString& tag, PlayingTechniqueType def);

    static AsciiString toXml(GradualTempoChangeType v);
    static GradualTempoChangeType fromXml(const AsciiString& tag, GradualTempoChangeType def);

    static AsciiString toXml(OrnamentStyle v);
    static OrnamentStyle fromXml(const AsciiString& str, OrnamentStyle def);

    static AsciiString toXml(PlacementV v);
    static PlacementV fromXml(const AsciiString& str, PlacementV def);
    static AsciiString toXml(PlacementH v);
    static PlacementH fromXml(const AsciiString& str, PlacementH def);

    static AsciiString toXml(TextPlace v);
    static TextPlace fromXml(const AsciiString& str, TextPlace def);

    static AsciiString toXml(DirectionV v);
    static DirectionV fromXml(const AsciiString& str, DirectionV def);
    static AsciiString toXml(DirectionH v);
    static DirectionH fromXml(const AsciiString& str, DirectionH def);

    static AsciiString toXml(LayoutBreakType v);
    static LayoutBreakType fromXml(const AsciiString& str, LayoutBreakType def);

    static AsciiString toXml(VeloType v);
    static VeloType fromXml(const AsciiString& str, VeloType def);

    static AsciiString toXml(BeamMode v);
    static BeamMode fromXml(const AsciiString& str, BeamMode def);

    static AsciiString toXml(GlissandoStyle v);
    static GlissandoStyle fromXml(const AsciiString& str, GlissandoStyle def);

    static AsciiString toXml(BarLineType v);
    static BarLineType fromXml(const AsciiString& str, BarLineType def);
};
}

#endif // MU_ENGRAVING_TYPESCONV_H
