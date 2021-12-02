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

    return def;
}

QString XmlValue::toXml(Align a)
{
    const char* h;
    if (a & Align::HCENTER) {
        h = "center";
    } else if (a & Align::RIGHT) {
        h = "right";
    } else {
        h = "left";
    }
    const char* v;
    if (a & Align::BOTTOM) {
        v = "bottom";
    } else if (a & Align::VCENTER) {
        v = "center";
    } else if (a & Align::BASELINE) {
        v = "baseline";
    } else {
        v = "top";
    }
    return QString("%1,%2").arg(h, v);
}

Align XmlValue::fromXml(const QString& str, Align def)
{
    QStringList sl = str.split(',');
    if (sl.size() != 2) {
        LOGD() << "bad align value: " << str;
        return def;
    }

    Align align = Align::LEFT;
    if (sl[0] == "center") {
        align = align | Align::HCENTER;
    } else if (sl[0] == "right") {
        align = align | Align::RIGHT;
    } else if (sl[0] == "left") {
    } else {
        LOGD() << "bad align value: " << str;
        return def;
    }

    if (sl[1] == "center") {
        align = align | Align::VCENTER;
    } else if (sl[1] == "bottom") {
        align = align | Align::BOTTOM;
    } else if (sl[1] == "baseline") {
        align = align | Align::BASELINE;
    } else if (sl[1] == "top") {
    } else {
        LOGD() << "bad align value: " << str;
        return def;
    }

    return align;
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
    return def;
}
