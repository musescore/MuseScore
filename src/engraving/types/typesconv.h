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
#include "types.h"

namespace mu::engraving {
class TConv
{
public:
    TConv() = default;

    static QString toXml(const QList<int>& v);
    static QList<int> fromXml(const QString& tag, const QList<int>& def);

    static QString toXml(Align v);
    static Align fromXml(const QString& str, Align def);
    static AlignH fromXml(const QString& str, AlignH def);
    static AlignV fromXml(const QString& str, AlignV def);

    static QString toUserName(SymId v);
    static QString toXml(SymId v);
    static SymId fromXml(const QString& tag, SymId def);

    static QString toUserName(Orientation v);
    static QString toXml(Orientation v);
    static Orientation fromXml(const QString& tag, Orientation def);

    static QString toUserName(NoteHeadType v);
    static QString toXml(NoteHeadType v);
    static NoteHeadType fromXml(const QString& tag, NoteHeadType def);
    static QString toUserName(NoteHeadScheme v);
    static QString toXml(NoteHeadScheme v);
    static NoteHeadScheme fromXml(const QString& tag, NoteHeadScheme def);
    static QString toUserName(NoteHeadGroup v);
    static QString toXml(NoteHeadGroup v);
    static NoteHeadGroup fromXml(const QString& tag, NoteHeadGroup def);

    static QString toUserName(ClefType v);
    static QString toXml(ClefType v);
    static ClefType fromXml(const QString& tag, ClefType def);

    static QString toUserName(DynamicType v);
    static Ms::SymId symId(DynamicType v);
    static DynamicType dynamicType(SymId v);
    static DynamicType dynamicType(const std::string& string);
    static QString toXml(DynamicType v);
    static DynamicType fromXml(const QString& tag, DynamicType def);
    static QString toUserName(DynamicRange v);
    static QString toXml(DynamicRange v);
    static DynamicRange fromXml(const QString& tag, DynamicRange def);
    static QString toUserName(DynamicSpeed v);
    static QString toXml(DynamicSpeed v);
    static DynamicSpeed fromXml(const QString& tag, DynamicSpeed def);

    static QString toUserName(HookType v);
    static QString toXml(HookType v);
    static HookType fromXml(const QString& tag, HookType def);

    static QString toUserName(KeyMode v);
    static QString toXml(KeyMode v);
    static KeyMode fromXml(const QString& tag, KeyMode def);

    static QString toUserName(TextStyleType v);
    static QString toXml(TextStyleType v);
    static TextStyleType fromXml(const QString& tag, TextStyleType def);

    static QString toUserName(ChangeMethod v);
    static QString toXml(ChangeMethod v);
    static ChangeMethod fromXml(const QString& tag, ChangeMethod def);
    static std::map<int /*positionTick*/, int> easingValueCurve(const int ticksDuration, const int stepsCount, const int amplitude,
                                                                const ChangeMethod method);
    static std::map<int /*positionTick*/, double> easingValueCurve(const int ticksDuration, const int stepsCount, const double amplitude,
                                                                   const ChangeMethod method);

    static QString toXml(const PitchValue& v);

    static QString toXml(AccidentalRole v);
    static AccidentalRole fromXml(const QString& tag, AccidentalRole def);

    static QString toXml(BeatsPerSecond v);
    static BeatsPerSecond fromXml(const QString& tag, BeatsPerSecond def);

    static QString toUserName(DurationType v);
    static QString toXml(DurationType v);
    static DurationType fromXml(const QString& tag, DurationType def);

    static QString toXml(PlayingTechniqueType v);
    static PlayingTechniqueType fromXml(const QString& tag, PlayingTechniqueType def);

    static QString toXml(TempoTechniqueType v);
    static TempoTechniqueType fromXml(const QString& tag, TempoTechniqueType def);
};
}

#endif // MU_ENGRAVING_TYPESCONV_H
