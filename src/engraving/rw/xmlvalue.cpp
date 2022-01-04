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
#include "xmlvalue.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rw;

QString XmlValue::toXml(OrnamentStyle v)
{
    switch (v) {
    case OrnamentStyle::BAROQUE: return "baroque";
    case OrnamentStyle::DEFAULT: return "default";
    }
    return QString();
}

OrnamentStyle XmlValue::fromXml(const QString& str, OrnamentStyle def)
{
    if (str == "baroque") {
        return OrnamentStyle::BAROQUE;
    }

    LOGD() << "unknown value: " << str;
    return def;
}

QString XmlValue::toXml(PlacementV v)
{
    switch (v) {
    case PlacementV::ABOVE: return "above";
    case PlacementV::BELOW: return "below";
    }
    return QString();
}

PlacementV XmlValue::fromXml(const QString& str, PlacementV def)
{
    if ("above" == str) {
        return PlacementV::ABOVE;
    }

    if ("below" == str) {
        return PlacementV::BELOW;
    }
    LOGD() << "unknown value: " << str;
    return def;
}

QString XmlValue::toXml(PlacementH v)
{
    switch (v) {
    case PlacementH::LEFT: return "left";
    case PlacementH::CENTER: return "center";
    case PlacementH::RIGHT: return "right";
    }
    return QString();
}

PlacementH XmlValue::fromXml(const QString& str, PlacementH def)
{
    if ("left" == str) {
        return PlacementH::LEFT;
    }

    if ("center" == str) {
        return PlacementH::CENTER;
    }

    if ("right" == str) {
        return PlacementH::RIGHT;
    }
    LOGD() << "unknown value: " << str;
    return def;
}

QString XmlValue::toXml(TextPlace v)
{
    switch (v) {
    case TextPlace::AUTO: return "auto";
    case TextPlace::ABOVE: return "above";
    case TextPlace::BELOW: return "below";
    case TextPlace::LEFT: return "left";
    }
    return QString();
}

TextPlace XmlValue::fromXml(const QString& s, TextPlace def)
{
    if (s == "auto" || s == "0") {
        return TextPlace::AUTO;
    }
    if (s == "above" || s == "1") {
        return TextPlace::ABOVE;
    }
    if (s == "below" || s == "2") {
        return TextPlace::BELOW;
    }
    if (s == "left" || s == "3") {
        return TextPlace::LEFT;
    }
    LOGD() << "unknown value: " << s;
    return def;
}

QString XmlValue::toXml(DirectionV v)
{
    switch (v) {
    case DirectionV::AUTO: return "auto";
    case DirectionV::UP: return "up";
    case DirectionV::DOWN: return "down";
    }
    return QString();
}

DirectionV XmlValue::fromXml(const QString& str, DirectionV def)
{
    if ("auto" == str) {
        return DirectionV::AUTO;
    }

    if ("up" == str) {
        return DirectionV::UP;
    }

    if ("down" == str) {
        return DirectionV::DOWN;
    }
    LOGD() << "unknown value: " << str;
    return def;
}

QString XmlValue::toXml(DirectionH v)
{
    switch (v) {
    case DirectionH::LEFT:  return "left";
    case DirectionH::RIGHT: return "right";
    case DirectionH::AUTO:  return "auto";
    }
    return QString();
}

DirectionH XmlValue::fromXml(const QString& str, DirectionH def)
{
    if ("left" == str || "1" == str) {
        return DirectionH::LEFT;
    }

    if ("right" == str || "2" == str) {
        return DirectionH::RIGHT;
    }

    if ("auto" == str) {
        return DirectionH::AUTO;
    }
    LOGD() << "unknown value: " << str;
    return def;
}

QString XmlValue::toXml(LayoutBreakType v)
{
    switch (v) {
    case LayoutBreakType::LINE: return "line";
    case LayoutBreakType::PAGE: return "page";
    case LayoutBreakType::SECTION: return "section";
    case LayoutBreakType::NOBREAK: return "nobreak";
    }
    return QString();
}

LayoutBreakType XmlValue::fromXml(const QString& str, LayoutBreakType def)
{
    if (str == "line") {
        return LayoutBreakType::LINE;
    }
    if (str == "page") {
        return LayoutBreakType::PAGE;
    }
    if (str == "section") {
        return LayoutBreakType::SECTION;
    }
    if (str == "nobreak") {
        return LayoutBreakType::NOBREAK;
    }
    LOGD() << "unknown value: " << str;
    return def;
}

QString XmlValue::toXml(VeloType v)
{
    switch (v) {
    case VeloType::OFFSET_VAL: return "offset";
    case VeloType::USER_VAL: return "user";
    }
    return QString();
}

VeloType XmlValue::fromXml(const QString& str, VeloType def)
{
    if (str == "offset") {
        return VeloType::OFFSET_VAL;
    }
    if (str == "user") {
        return VeloType::USER_VAL;
    }
    LOGD() << "unknown value: " << str;
    return def;
}

QString XmlValue::toXml(BeamMode v)
{
    switch (v) {
    case BeamMode::AUTO: return "auto";
    case BeamMode::BEGIN: return "begin";
    case BeamMode::MID: return "mid";
    case BeamMode::END: return "end";
    case BeamMode::NONE: return "no";
    case BeamMode::BEGIN32: return "begin32";
    case BeamMode::BEGIN64: return "begin64";
    case BeamMode::INVALID: return "invalid";
    }
    return QString();
}

BeamMode XmlValue::fromXml(const QString& str, BeamMode def)
{
    if (str == "auto") {
        return BeamMode::AUTO;
    } else if (str == "begin") {
        return BeamMode::BEGIN;
    } else if (str == "mid") {
        return BeamMode::MID;
    } else if (str == "end") {
        return BeamMode::END;
    } else if (str == "no") {
        return BeamMode::NONE;
    } else if (str == "begin32") {
        return BeamMode::BEGIN32;
    } else if (str == "begin64") {
        return BeamMode::BEGIN64;
    } else {
        bool ok = false;
        int v = str.toInt(&ok);
        if (ok) {
            return static_cast<BeamMode>(v);
        }
    }
    return def;
}

QString XmlValue::toXml(GlissandoStyle v)
{
    switch (v) {
    case GlissandoStyle::BLACK_KEYS: return "blackkeys";
    case GlissandoStyle::WHITE_KEYS: return "whitekeys";
    case GlissandoStyle::DIATONIC: return "diatonic";
    case GlissandoStyle::PORTAMENTO: return "portamento";
    case GlissandoStyle::CHROMATIC: return "chromatic";
    }
    return QString();
}

GlissandoStyle XmlValue::fromXml(const QString& str, GlissandoStyle def)
{
    if (str == "whitekeys") {
        return GlissandoStyle::WHITE_KEYS;
    } else if (str == "blackkeys") {
        return GlissandoStyle::BLACK_KEYS;
    } else if (str == "diatonic") {
        return GlissandoStyle::DIATONIC;
    } else if (str == "portamento") {
        return GlissandoStyle::PORTAMENTO;
    } else if (str == "chromatic" || str == "Chromatic") { //! NOTE Chromatic - for compat reason
        return GlissandoStyle::CHROMATIC;
    }
    return def;
}

QString XmlValue::toXml(BarLineType v)
{
    switch (v) {
    case BarLineType::NORMAL: return "normal";
    case BarLineType::DOUBLE: return "double";
    case BarLineType::START_REPEAT: return "start-repeat";
    case BarLineType::END_REPEAT: return "end-repeat";
    case BarLineType::BROKEN: return "dashed";
    case BarLineType::END: return "end";
    case BarLineType::END_START_REPEAT: return "end-start-repeat";
    case BarLineType::DOTTED: return "dotted";
    case BarLineType::REVERSE_END: return "reverse-end";
    case BarLineType::HEAVY: return "heavy";
    case BarLineType::DOUBLE_HEAVY: return "double-heavy";
    }
    return QString();
}

BarLineType XmlValue::fromXml(const QString& str, BarLineType def)
{
    if (str == "normal") {
        return BarLineType::NORMAL;
    } else if (str == "double") {
        return BarLineType::DOUBLE;
    } else if (str == "start-repeat") {
        return BarLineType::START_REPEAT;
    } else if (str == "end-repeat") {
        return BarLineType::END_REPEAT;
    } else if (str == "dashed") {
        return BarLineType::BROKEN;
    } else if (str == "end") {
        return BarLineType::END;
    } else if (str == "end-start-repeat") {
        return BarLineType::END_START_REPEAT;
    } else if (str == "dotted") {
        return BarLineType::DOTTED;
    } else if (str == "reverse-end") {
        return BarLineType::REVERSE_END;
    } else if (str == "heavy") {
        return BarLineType::HEAVY;
    } else if (str == "double-heavy") {
        return BarLineType::DOUBLE_HEAVY;
    } else {
        bool ok = false;
        int v = str.toInt(&ok);
        if (ok) {
            return static_cast<BarLineType>(v);
        }
    }
    return def;
}
